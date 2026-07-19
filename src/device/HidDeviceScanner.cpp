#include "HidDeviceScanner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QRegularExpression>

#include <algorithm>
#include <unistd.h>

namespace strikepro {
namespace {

QMap<QString, QString> readProperties(const QString &path)
{
    QMap<QString, QString> result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }

    // sysfs attributes commonly report a synthetic size (often 4096 bytes).
    // QFile::atEnd() then remains false after the real EOF and causes a busy
    // loop, so read the pseudo-file once and iterate over that snapshot.
    const QList<QByteArray> lines = file.readAll().split('\n');
    for (const QByteArray &rawLine : lines) {
        const QByteArray line = rawLine.trimmed();
        const qsizetype separator = line.indexOf('=');
        if (separator <= 0) {
            continue;
        }
        result.insert(
            QString::fromUtf8(line.first(separator)),
            QString::fromUtf8(line.sliced(separator + 1)));
    }
    return result;
}

int interfaceNumberFromPath(const QString &canonicalPath)
{
    static const QRegularExpression pattern(
        QStringLiteral(R"(:1\.([0-9]+)(?:/|$))"));
    const QRegularExpressionMatch match = pattern.match(canonicalPath);
    return match.hasMatch() ? match.captured(1).toInt() : -1;
}

} // namespace

QList<HidInterface> HidDeviceScanner::scan()
{
    QList<HidInterface> result;
    const QDir hidraw(QStringLiteral("/sys/class/hidraw"));

    for (const QString &entry : hidraw.entryList(
             {QStringLiteral("hidraw*")},
             QDir::Dirs | QDir::NoDotAndDotDot,
             QDir::Name)) {
        const QString classPath = hidraw.absoluteFilePath(entry);
        const QString devicePath = classPath + QStringLiteral("/device");
        const QMap<QString, QString> properties =
            readProperties(devicePath + QStringLiteral("/uevent"));
        const QStringList hidId =
            properties.value(QStringLiteral("HID_ID")).split(':');
        if (hidId.size() != 3) {
            continue;
        }

        bool vendorOk = false;
        bool productOk = false;
        const quint16 vendorId = hidId.at(1).toUInt(&vendorOk, 16);
        const quint16 productId = hidId.at(2).toUInt(&productOk, 16);
        if (!vendorOk || !productOk || vendorId != kMsiVendorId
            || !isStrikeProProduct(productId)) {
            continue;
        }

        HidInterface interface;
        interface.devNode = QStringLiteral("/dev/") + entry;
        interface.sysfsPath = QFileInfo(devicePath).canonicalFilePath();
        interface.name = properties.value(QStringLiteral("HID_NAME"));
        interface.vendorId = vendorId;
        interface.productId = productId;
        interface.interfaceNumber =
            interfaceNumberFromPath(interface.sysfsPath);
        interface.readable =
            ::access(interface.devNode.toLocal8Bit().constData(), R_OK) == 0;
        interface.writable =
            ::access(interface.devNode.toLocal8Bit().constData(), W_OK) == 0;

        QFile descriptor(devicePath + QStringLiteral("/report_descriptor"));
        if (descriptor.open(QIODevice::ReadOnly)) {
            interface.reportDescriptor = descriptor.readAll();
        }

        result.push_back(interface);
    }

    std::ranges::sort(
        result,
        [](const HidInterface &left, const HidInterface &right) {
            if (left.productId != right.productId) {
                return left.productId < right.productId;
            }
            return left.interfaceNumber < right.interfaceNumber;
        });
    return result;
}

} // namespace strikepro
