#include "BatteryDecoder.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace strikepro {

ProtocolProfile BatteryDecoder::confirmedStrikeProProfile()
{
    return ProtocolProfile{
        .version = 1,
        .source = ReportSource::Input,
        .interfaceNumber = 1,
        .reportId = 0x0d,
        .percentageOffset = 8,
        .chargingOffset = -1,
        .chargingMask = 0,
        .transportOffset = 9,
        .wirelessTransportValue = 2,
        .wiredTransportValue = 1,
        .matchPrefix = QByteArray::fromHex("0db0010000000502"),
        .path = QStringLiteral("built-in:msi-center-2026-07-19"),
    };
}

std::optional<ProtocolProfile>
BatteryDecoder::loadProfile(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.exists()) {
        return confirmedStrikeProProfile();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (error != nullptr) {
            *error = QCoreApplication::translate(
                         "BatteryDecoder",
                         "Protocol profile not found: %1")
                         .arg(path);
        }
        return std::nullopt;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error != nullptr) {
            *error = QCoreApplication::translate(
                         "BatteryDecoder",
                         "Invalid JSON: %1")
                         .arg(parseError.errorString());
        }
        return std::nullopt;
    }

    const QJsonObject root = document.object();
    const QJsonObject battery =
        root.value(QStringLiteral("battery")).toObject();
    ProtocolProfile profile;
    profile.version = root.value(QStringLiteral("version")).toInt(1);
    profile.path = path;

    const QString source = battery.value(QStringLiteral("source")).toString();
    if (source == QStringLiteral("input")) {
        profile.source = ReportSource::Input;
    } else if (source == QStringLiteral("feature")) {
        profile.source = ReportSource::Feature;
    } else {
        if (error != nullptr) {
            *error = QCoreApplication::translate(
                "BatteryDecoder",
                "battery.source must be input or feature");
        }
        return std::nullopt;
    }

    profile.interfaceNumber =
        battery.value(QStringLiteral("interface")).toInt(-1);
    profile.reportId = battery.value(QStringLiteral("report_id")).toInt(-1);
    profile.percentageOffset =
        battery.value(QStringLiteral("percentage_offset")).toInt(-1);
    profile.chargingOffset =
        battery.value(QStringLiteral("charging_offset")).toInt(-1);
    profile.chargingMask =
        battery.value(QStringLiteral("charging_mask")).toInt(0);
    profile.transportOffset =
        battery.value(QStringLiteral("transport_offset")).toInt(-1);
    profile.wirelessTransportValue =
        battery.value(QStringLiteral("wireless_transport_value")).toInt(-1);
    profile.wiredTransportValue =
        battery.value(QStringLiteral("wired_transport_value")).toInt(-1);
    const QByteArray matchPrefixHex =
        battery.value(QStringLiteral("match_prefix_hex")).toString().toLatin1();
    if (!matchPrefixHex.isEmpty()) {
        profile.matchPrefix = QByteArray::fromHex(matchPrefixHex);
        if (profile.matchPrefix.size() * 2 != matchPrefixHex.size()) {
            if (error != nullptr) {
                *error = QCoreApplication::translate(
                    "BatteryDecoder",
                    "battery.match_prefix_hex must contain pairs of "
                    "hexadecimal "
                    "digits");
            }
            return std::nullopt;
        }
    }

    if (profile.interfaceNumber < 0 || profile.reportId < 0) {
        if (error != nullptr) {
            *error = QCoreApplication::translate(
                "BatteryDecoder",
                "The profile does not define interface and report_id");
        }
        return std::nullopt;
    }
    return profile;
}

std::optional<BatteryReading>
BatteryDecoder::decode(const HidReport &report, const ProtocolProfile &profile)
{
    const int reportId =
        report.requestedReportId >= 0 ? report.requestedReportId
        : report.data.isEmpty()       ? -1
                                      : static_cast<quint8>(report.data.at(0));
    if (!profile.canDecodePercentage() || report.source != profile.source
        || report.interfaceNumber != profile.interfaceNumber
        || report.data.isEmpty() || reportId != profile.reportId
        || (!profile.matchPrefix.isEmpty()
            && !report.data.startsWith(profile.matchPrefix))
        || profile.percentageOffset >= report.data.size()) {
        return std::nullopt;
    }

    BatteryReading reading;
    reading.percent =
        static_cast<quint8>(report.data.at(profile.percentageOffset));
    if (!reading.isValid()) {
        return std::nullopt;
    }

    if (profile.transportOffset >= 0) {
        if (profile.transportOffset >= report.data.size()) {
            return std::nullopt;
        }
        const int transport =
            static_cast<quint8>(report.data.at(profile.transportOffset));
        if (report.productId == kStrikeProWirelessProductId) {
            // The receiver returns a zero level with the wireless transport
            // byte when the keyboard link is unavailable. A working keyboard
            // shuts down before reporting a usable zero-percent reading.
            if (transport != profile.wirelessTransportValue
                || reading.percent == 0) {
                return std::nullopt;
            }
            reading.charging = false;
        } else if (report.productId == kStrikeProWiredProductId) {
            if (transport != profile.wiredTransportValue) {
                return std::nullopt;
            }
            reading.charging = true;
        } else {
            return std::nullopt;
        }
    }

    if (profile.chargingOffset >= 0
        && profile.chargingOffset < report.data.size()
        && profile.chargingMask != 0) {
        reading.charging =
            (static_cast<quint8>(report.data.at(profile.chargingOffset))
             & profile.chargingMask)
            != 0;
    }
    return reading;
}

} // namespace strikepro
