#include "CliRunner.h"

#include "device/DeviceCatalog.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <QTimer>

#include <algorithm>

namespace strikepro {
namespace {

QString sourceName(ReportSource source)
{
    switch (source) {
    case ReportSource::Input:
        return QStringLiteral("input");
    case ReportSource::Feature:
        return QStringLiteral("feature");
    case ReportSource::Output:
        return QStringLiteral("output");
    }
    return QStringLiteral("unknown");
}

QString hexId(const QByteArray &data)
{
    if (data.isEmpty()) {
        return QStringLiteral("none");
    }
    return QStringLiteral("0x%1")
        .arg(static_cast<quint8>(data.front()), 2, 16, QLatin1Char('0'));
}

} // namespace

CliRunner::CliRunner(CliOptions options, QObject *parent)
    : QObject(parent)
    , m_options(std::move(options))
    , m_monitor(this)
{
    connect(
        &m_monitor,
        &HidMonitor::interfacesChanged,
        this,
        &CliRunner::handleInterfaces);
    connect(
        &m_monitor,
        &HidMonitor::reportReceived,
        this,
        &CliRunner::handleReport);
    connect(
        &m_monitor,
        &HidMonitor::diagnosticMessage,
        this,
        &CliRunner::handleDiagnostic);

    QString profileError;
    m_profile =
        BatteryDecoder::loadProfile(m_options.profilePath, &profileError);
    if (m_profile.has_value() && m_profile->canDecodePercentage()) {
        log(QStringLiteral("info"),
            QStringLiteral("profile_loaded"),
            tr("Battery profile loaded"),
            {{QStringLiteral("path"), m_profile->path}});
    } else {
        m_profile.reset();
        log(QStringLiteral("info"),
            QStringLiteral("profile_unavailable"),
            tr("The battery decoder is not configured"),
            {{QStringLiteral("path"), m_options.profilePath},
             {QStringLiteral("reason"), profileError}});
    }
}

void CliRunner::handleInterfaces(const QList<HidInterface> &interfaces)
{
    if (interfaces.isEmpty()) {
        log(QStringLiteral("error"),
            QStringLiteral("device_not_found"),
            tr("No supported keyboard was found"));
        if (!m_options.logs) {
            finishOnce(2);
        }
        return;
    }

    QJsonArray interfaceList;
    QStringList numbers;
    bool diagnosticAccess = true;
    for (const HidInterface &interface : interfaces) {
        QJsonObject item;
        item.insert(QStringLiteral("number"), interface.interfaceNumber);
        item.insert(
            QStringLiteral("vendor_id"),
            QStringLiteral("%1")
                .arg(interface.vendorId, 4, 16, QLatin1Char('0')));
        item.insert(
            QStringLiteral("product_id"),
            QStringLiteral("%1")
                .arg(interface.productId, 4, 16, QLatin1Char('0')));
        item.insert(QStringLiteral("readable"), interface.readable);
        item.insert(QStringLiteral("writable"), interface.writable);
        item.insert(
            QStringLiteral("descriptor_bytes"),
            interface.reportDescriptor.size());
        interfaceList.append(item);
        numbers.push_back(QStringLiteral("%1:if%2")
                              .arg(interface.productId, 4, 16, QLatin1Char('0'))
                              .arg(interface.interfaceNumber));

        if (interface.interfaceNumber == 1 || interface.interfaceNumber == 2
            || interface.interfaceNumber == 4) {
            diagnosticAccess = diagnosticAccess && interface.readable;
        }
    }

    QStringList deviceNames;
    for (const SupportedDevice &device : groupSupportedDevices(interfaces)) {
        deviceNames.push_back(device.definition.displayNameString());
    }
    log(QStringLiteral("info"),
        QStringLiteral("device_connected"),
        tr("%1 found, interfaces %2")
            .arg(deviceNames.join(QStringLiteral(", ")), numbers.join(',')),
        {{QStringLiteral("devices"), QJsonArray::fromStringList(deviceNames)},
         {QStringLiteral("interfaces"), interfaceList}});

    if (!diagnosticAccess) {
        log(QStringLiteral("warning"),
            QStringLiteral("access_denied"),
            tr("No access to vendor hidraw. Install the rule with "
               "scripts/linux/install-udev-rule.sh"),
            {{QStringLiteral("helper"),
              QStringLiteral("scripts/linux/install-udev-rule.sh")}});
    }

    if (m_options.battery && !m_batteryQueryStarted) {
        m_batteryQueryStarted = true;
        QString error;
        if (!m_monitor.requestBattery(&error)) {
            log(QStringLiteral("error"),
                QStringLiteral("battery_query_failed"),
                error);
            finishOnce(4);
            return;
        }
        log(QStringLiteral("info"),
            QStringLiteral("battery_query_sent"),
            tr("Confirmed MSI Center battery query sent"));
        QTimer::singleShot(2500, this, [this] {
            if (m_options.battery && !m_finishScheduled) {
                log(QStringLiteral("error"),
                    QStringLiteral("battery_timeout"),
                    tr("The keyboard did not answer the battery query"));
                finishOnce(4);
            }
        });
        return;
    }

    if (m_options.once) {
        m_monitor.takeReadOnlySnapshot();
        finishOnce(diagnosticAccess ? 0 : 3);
        return;
    }

    if (!m_options.logs) {
        finishOnce(diagnosticAccess ? 0 : 3);
    }
}

void CliRunner::handleReport(const HidReport &report)
{
    if (m_options.logs || m_options.once) {
        const QString source = sourceName(report.source);
        const QString responseId = hexId(report.data);
        const QString requestedId = report.requestedReportId >= 0
                                        ? QStringLiteral("0x%1").arg(
                                              report.requestedReportId,
                                              2,
                                              16,
                                              QLatin1Char('0'))
                                        : QStringLiteral("none");
        const QString idSummary =
            report.requestedReportId >= 0
                ? tr("request=%1 response=%2").arg(requestedId, responseId)
                : tr("id=%1").arg(responseId);
        log(QStringLiteral("info"),
            QStringLiteral("hid_report"),
            tr("%1 if%2 %3 size=%4 data=%5")
                .arg(
                    source,
                    QString::number(report.interfaceNumber),
                    idSummary,
                    QString::number(report.data.size()),
                    QString::fromLatin1(report.data.toHex(' '))),
            {{QStringLiteral("source"), source},
             {QStringLiteral("interface"), report.interfaceNumber},
             {QStringLiteral("product_id"),
              QStringLiteral("%1")
                  .arg(report.productId, 4, 16, QLatin1Char('0'))},
             {QStringLiteral("report_id"), responseId},
             {QStringLiteral("requested_report_id"), requestedId},
             {QStringLiteral("size"), report.data.size()},
             {QStringLiteral("data_hex"),
              QString::fromLatin1(report.data.toHex())}});
    }

    if (!m_profile.has_value()) {
        return;
    }
    const auto reading = BatteryDecoder::decode(report, *m_profile);
    if (!reading.has_value()) {
        return;
    }

    QJsonObject fields{{QStringLiteral("percent"), reading->percent}};
    QString message = tr("Battery: %1%").arg(reading->percent);
    if (reading->charging.has_value()) {
        fields.insert(QStringLiteral("charging"), *reading->charging);
        message += *reading->charging ? tr(", charging") : tr(", on battery");
    }
    log(QStringLiteral("info"),
        QStringLiteral("battery"),
        message,
        std::move(fields));
    if (m_options.battery) {
        finishOnce(0);
    }
}

void CliRunner::handleDiagnostic(const QString &message)
{
    if (m_options.logs || m_options.once || m_options.battery) {
        log(QStringLiteral("warning"), QStringLiteral("diagnostic"), message);
    }
}

void CliRunner::log(
    const QString &level,
    const QString &event,
    const QString &message,
    QJsonObject fields)
{
    const QString timestamp =
        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    QTextStream output(stdout);

    if (m_options.json) {
        fields.insert(QStringLiteral("timestamp"), timestamp);
        fields.insert(QStringLiteral("level"), level);
        fields.insert(QStringLiteral("event"), event);
        fields.insert(QStringLiteral("message"), message);
        output << QJsonDocument(fields).toJson(QJsonDocument::Compact) << '\n';
    } else {
        output << timestamp << " [" << level.toUpper() << "] " << event << ": "
               << message << '\n';
    }
    output.flush();
}

void CliRunner::finishOnce(int exitCode)
{
    if (m_finishScheduled) {
        return;
    }
    m_finishScheduled = true;
    QTimer::singleShot(0, qApp, [exitCode] {
        QCoreApplication::exit(exitCode);
    });
}

} // namespace strikepro
