#include "HidMonitor.h"

#include "HidDeviceScanner.h"

#include <QSet>
#include <QSocketNotifier>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <linux/netlink.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace strikepro {
namespace {

QString interfaceSignature(const QList<HidInterface> &interfaces)
{
    QStringList parts;
    for (const HidInterface &interface : interfaces) {
        parts.push_back(QStringLiteral("%1:%2:%3:%4:%5")
                            .arg(interface.devNode)
                            .arg(interface.productId)
                            .arg(interface.interfaceNumber)
                            .arg(interface.readable)
                            .arg(interface.writable));
    }
    return parts.join('|');
}

const HidInterface *
findInterface(const QList<HidInterface> &interfaces, const QString &devNode)
{
    const auto found =
        std::ranges::find(interfaces, devNode, &HidInterface::devNode);
    return found == interfaces.end() ? nullptr : &*found;
}

const HidInterface *findPreferredInterface(
    const QList<HidInterface> &interfaces, int interfaceNumber)
{
    for (const quint16 productId :
         {kStrikeProWiredProductId, kStrikeProWirelessProductId}) {
        const auto found = std::ranges::find_if(
            interfaces,
            [productId, interfaceNumber](const HidInterface &interface) {
                return interface.productId == productId
                       && interface.interfaceNumber == interfaceNumber;
            });
        if (found != interfaces.end()) {
            return &*found;
        }
    }
    return nullptr;
}

bool isTargetDeviceEvent(const QByteArray &event)
{
    const QByteArray lower = event.toLower();
    const bool relevantSubsystem = lower.contains("subsystem=hidraw")
                                   || lower.contains("subsystem=hid")
                                   || lower.contains("subsystem=usb");
    const bool relevantAction =
        lower.startsWith("add@") || lower.startsWith("remove@")
        || lower.startsWith("change@") || lower.startsWith("bind@")
        || lower.startsWith("unbind@") || lower.contains("action=add")
        || lower.contains("action=remove") || lower.contains("action=change")
        || lower.contains("action=bind") || lower.contains("action=unbind");
    const bool strikePro =
        lower.contains("0db0")
        && (lower.contains("1620") || lower.contains("b231"));
    return relevantSubsystem && relevantAction && strikePro;
}

} // namespace

HidMonitor::HidMonitor(QObject *parent)
    : QObject(parent)
{
    m_refreshTimer.setInterval(1000);
    connect(&m_refreshTimer, &QTimer::timeout, this, &HidMonitor::refresh);
    m_refreshTimer.start();

    m_eventRefreshTimer.setSingleShot(true);
    m_eventRefreshTimer.setInterval(75);
    connect(&m_eventRefreshTimer, &QTimer::timeout, this, &HidMonitor::refresh);
    setupUeventMonitor();

    QTimer::singleShot(0, this, &HidMonitor::refresh);
}

HidMonitor::~HidMonitor()
{
    closeUeventMonitor();
    closeReaders();
}

bool HidMonitor::requestBattery(QString *error)
{
    return requestBattery(QString(), error);
}

bool HidMonitor::requestBattery(const QString &devNode, QString *error)
{
    // Interface 1 has an unnumbered 64-byte output report. hidraw requires
    // a leading zero report ID, followed by the exact query captured from
    // MSI Center ten times across wireless and wired traces.
    QByteArray report(65, '\0');
    report[1] = static_cast<char>(0x0d);
    report[2] = static_cast<char>(0xb0);
    report[3] = static_cast<char>(0x01);
    report[7] = static_cast<char>(0x05);

    bool interfaceFound = false;
    bool accessibleInterfaceFound = false;
    bool querySent = false;
    QString lastError;
    for (const quint16 productId :
         {kStrikeProWiredProductId, kStrikeProWirelessProductId}) {
        const auto found = std::ranges::find_if(
            m_interfaces,
            [devNode, productId](const HidInterface &interface) {
                return (devNode.isEmpty() || interface.devNode == devNode)
                       && interface.productId == productId
                       && interface.interfaceNumber == 1;
            });
        if (found == m_interfaces.end()) {
            continue;
        }

        interfaceFound = true;
        if (!found->readable || !found->writable) {
            lastError = tr("No access to interface 1 of device 0db0:%1. "
                           "Reinstall the udev rule.")
                            .arg(found->productId, 4, 16, QLatin1Char('0'));
            continue;
        }
        accessibleInterfaceFound = true;

        const QByteArray nativePath = found->devNode.toLocal8Bit();
        const int fd =
            ::open(nativePath.constData(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) {
            lastError = tr("No write access to %1: %2")
                            .arg(
                                found->devNode,
                                QString::fromLocal8Bit(std::strerror(errno)));
            continue;
        }

        const ssize_t written = ::write(fd, report.constData(), report.size());
        const int savedErrno = errno;
        ::close(fd);
        if (written == report.size()) {
            querySent = true;
            continue;
        }
        lastError =
            written < 0
                ? tr("Could not send the battery query: %1")
                      .arg(QString::fromLocal8Bit(std::strerror(savedErrno)))
                : tr("Battery query was incomplete: %1 of %2 bytes")
                      .arg(written)
                      .arg(report.size());
    }

    if (querySent) {
        return true;
    }
    if (error != nullptr) {
        if (!interfaceFound) {
            *error = tr("HID interface 1 was not found");
        } else if (!accessibleInterfaceFound && lastError.isEmpty()) {
            *error = tr("No accessible HID interface 1 was found");
        } else {
            *error = lastError;
        }
    }
    return false;
}

void HidMonitor::refresh()
{
    const QList<HidInterface> current = HidDeviceScanner::scan();
    const bool changed =
        !m_hasRefreshed
        || interfaceSignature(current) != interfaceSignature(m_interfaces);
    m_hasRefreshed = true;
    m_interfaces = current;
    if (!changed) {
        return;
    }

    rebuildReaders();
    emit interfacesChanged(m_interfaces);
}

void HidMonitor::setupUeventMonitor()
{
    m_ueventFd = ::socket(
        AF_NETLINK,
        SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
        NETLINK_KOBJECT_UEVENT);
    if (m_ueventFd < 0) {
        return;
    }

    int receiveBufferSize = 256 * 1024;
    ::setsockopt(
        m_ueventFd,
        SOL_SOCKET,
        SO_RCVBUF,
        &receiveBufferSize,
        sizeof(receiveBufferSize));

    sockaddr_nl address{};
    address.nl_family = AF_NETLINK;
    address.nl_groups = 1;
    if (::bind(
            m_ueventFd,
            reinterpret_cast<const sockaddr *>(&address),
            sizeof(address))
        < 0) {
        ::close(m_ueventFd);
        m_ueventFd = -1;
        return;
    }

    m_ueventNotifier =
        new QSocketNotifier(m_ueventFd, QSocketNotifier::Read, this);
    connect(
        m_ueventNotifier,
        &QSocketNotifier::activated,
        this,
        &HidMonitor::readUevents);
}

void HidMonitor::closeUeventMonitor()
{
    if (m_ueventNotifier != nullptr) {
        m_ueventNotifier->setEnabled(false);
        delete m_ueventNotifier;
        m_ueventNotifier = nullptr;
    }
    if (m_ueventFd >= 0) {
        ::close(m_ueventFd);
        m_ueventFd = -1;
    }
}

void HidMonitor::readUevents()
{
    bool targetEventReceived = false;
    for (;;) {
        char buffer[8192] = {};
        const ssize_t count =
            ::recv(m_ueventFd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (count > 0) {
            targetEventReceived |=
                isTargetDeviceEvent(QByteArray(buffer, count));
            continue;
        }
        if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            emit diagnosticMessage(
                tr("USB event monitor failed: %1")
                    .arg(QString::fromLocal8Bit(std::strerror(errno))));
        }
        break;
    }

    if (targetEventReceived) {
        emit deviceEvent();
        m_eventRefreshTimer.start();
    }
}

void HidMonitor::rebuildReaders()
{
    closeReaders();

    for (const HidInterface &interface : m_interfaces) {
        // Interface 1 is the bidirectional vendor channel. Interface 4 publishes
        // vendor input report 0x0d. We deliberately do not consume keyboard input.
        if (!interface.readable
            || (interface.interfaceNumber != 1
                && interface.interfaceNumber != 4)) {
            continue;
        }

        const QByteArray nativePath = interface.devNode.toLocal8Bit();
        const int fd =
            ::open(nativePath.constData(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) {
            emit diagnosticMessage(
                tr("Could not open %1: %2")
                    .arg(
                        interface.devNode,
                        QString::fromLocal8Bit(std::strerror(errno))));
            continue;
        }

        auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        connect(
            notifier,
            &QSocketNotifier::activated,
            this,
            [this, path = interface.devNode] { readAvailable(path); });
        m_fds.insert(interface.devNode, fd);
        m_notifiers.insert(interface.devNode, notifier);
    }
}

void HidMonitor::closeReaders()
{
    for (QSocketNotifier *notifier : std::as_const(m_notifiers)) {
        notifier->setEnabled(false);
        delete notifier;
    }
    m_notifiers.clear();

    for (const int fd : std::as_const(m_fds)) {
        ::close(fd);
    }
    m_fds.clear();
}

void HidMonitor::readAvailable(const QString &devNode)
{
    const auto fdIt = m_fds.constFind(devNode);
    const HidInterface *interface = findInterface(m_interfaces, devNode);
    if (fdIt == m_fds.cend() || interface == nullptr) {
        return;
    }

    for (;;) {
        char buffer[256] = {};
        const ssize_t count = ::read(fdIt.value(), buffer, sizeof(buffer));
        if (count > 0) {
            emit reportReceived(HidReport{
                .devNode = devNode,
                .interfaceNumber = interface->interfaceNumber,
                .productId = interface->productId,
                .source = ReportSource::Input,
                .requestedReportId = -1,
                .data = QByteArray(buffer, count),
            });
            continue;
        }
        if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            emit diagnosticMessage(
                tr("Read error on %1: %2")
                    .arg(
                        devNode,
                        QString::fromLocal8Bit(std::strerror(errno))));
        }
        break;
    }
}

void HidMonitor::takeReadOnlySnapshot()
{
    struct Query {
        int interfaceNumber;
        ReportSource source;
        quint8 reportId;
        int size;
    };
    // These are device-to-host GET_REPORT requests. HIDIOCGOUTPUT reads the
    // current Output report and does not send or change it.
    const Query queries[] = {
        {1, ReportSource::Input, 0x00, 65},
        {1, ReportSource::Output, 0x00, 65},
        {2, ReportSource::Feature, 0x07, 64},
        {2, ReportSource::Feature, 0x0c, 11},
        {2, ReportSource::Input, 0x06, 4},
        {4, ReportSource::Input, 0x0d, 64},
    };

    for (const Query query : queries) {
        const HidInterface *interface =
            findPreferredInterface(m_interfaces, query.interfaceNumber);
        if (interface == nullptr) {
            emit diagnosticMessage(tr("HID interface %1 was not found")
                                       .arg(query.interfaceNumber));
            continue;
        }

        const QByteArray nativePath = interface->devNode.toLocal8Bit();
        int fd =
            ::open(nativePath.constData(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) {
            fd = ::open(
                nativePath.constData(),
                O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        }
        if (fd < 0) {
            emit diagnosticMessage(
                tr("No access to %1. Install the udev rule from packaging.")
                    .arg(interface->devNode));
            continue;
        }

        QByteArray data(query.size, '\0');
        data[0] = static_cast<char>(query.reportId);
        const unsigned long request =
            query.source == ReportSource::Feature  ? HIDIOCGFEATURE(query.size)
            : query.source == ReportSource::Output ? HIDIOCGOUTPUT(query.size)
                                                   : HIDIOCGINPUT(query.size);
        const QString operation = query.source == ReportSource::Feature
                                      ? QStringLiteral("GET_FEATURE")
                                  : query.source == ReportSource::Output
                                      ? QStringLiteral("GET_OUTPUT")
                                      : QStringLiteral("GET_INPUT");
        const int result = ::ioctl(fd, request, data.data());
        if (result < 0) {
            emit diagnosticMessage(
                tr("%1 if%2 0x%3: %4")
                    .arg(operation)
                    .arg(query.interfaceNumber)
                    .arg(query.reportId, 2, 16, QLatin1Char('0'))
                    .arg(QString::fromLocal8Bit(std::strerror(errno))));
            ::close(fd);
            continue;
        }
        data.resize(result);
        emit reportReceived(HidReport{
            .devNode = interface->devNode,
            .interfaceNumber = interface->interfaceNumber,
            .productId = interface->productId,
            .source = query.source,
            .requestedReportId = query.reportId,
            .data = data,
        });
        ::close(fd);
    }
}

} // namespace strikepro
