#include "BatteryProtocol.h"

#include <string_view>

namespace strikepro {
namespace {

constexpr std::string_view kStrikeProProtocol{"strike-pro-v1"};

} // namespace

bool hasKnownBatteryProtocol(const DeviceDefinition &definition)
{
    return definition.batteryProtocol == kStrikeProProtocol;
}

QByteArray batteryQueryFor(const DeviceDefinition &definition)
{
    if (!hasKnownBatteryProtocol(definition)) {
        return {};
    }

    // Interface 1 has an unnumbered 64-byte output report. hidraw requires
    // a leading zero report ID, followed by the confirmed MSI Center query.
    QByteArray report(65, '\0');
    report[1] = static_cast<char>(0x0d);
    report[2] = static_cast<char>(0xb0);
    report[3] = static_cast<char>(0x01);
    report[7] = static_cast<char>(0x05);
    return report;
}

std::optional<ProtocolProfile>
builtInBatteryProfileFor(const DeviceDefinition &definition)
{
    if (!hasKnownBatteryProtocol(definition)) {
        return std::nullopt;
    }
    return BatteryDecoder::confirmedStrikeProProfile();
}

std::optional<ProtocolProfile> batteryProfileFor(
    const DeviceDefinition &definition,
    const QString &overridePath,
    QString *error)
{
    if (!hasKnownBatteryProtocol(definition)) {
        if (error != nullptr) {
            *error = QStringLiteral("Unknown battery protocol: %1")
                         .arg(definition.batteryProtocolString());
        }
        return std::nullopt;
    }
    return BatteryDecoder::loadProfile(overridePath, error);
}

} // namespace strikepro
