#pragma once

#include "HidTypes.h"

#include <QString>
#include <optional>

namespace strikepro {

struct ProtocolProfile {
    int version = 1;
    ReportSource source = ReportSource::Input;
    int interfaceNumber = -1;
    int reportId = -1;
    int percentageOffset = -1;
    int chargingOffset = -1;
    int chargingMask = 0;
    int transportOffset = -1;
    int wirelessTransportValue = -1;
    int wiredTransportValue = -1;
    QByteArray matchPrefix;
    QString path;

    [[nodiscard]] bool canDecodePercentage() const { return percentageOffset >= 0; }
};

struct BatteryReading {
    int percent = -1;
    std::optional<bool> charging;

    [[nodiscard]] bool isValid() const { return percent >= 0 && percent <= 100; }
};

class BatteryDecoder {
public:
    [[nodiscard]] static ProtocolProfile confirmedStrikeProProfile();
    [[nodiscard]] static std::optional<ProtocolProfile> loadProfile(
        const QString &path,
        QString *error = nullptr);
    [[nodiscard]] static std::optional<BatteryReading> decode(
        const HidReport &report,
        const ProtocolProfile &profile);
};

} // namespace strikepro
