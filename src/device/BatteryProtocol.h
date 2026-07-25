#pragma once

#include "BatteryDecoder.h"
#include "DeviceDefinitions.h"

#include <QByteArray>
#include <optional>

namespace strikepro {

[[nodiscard]] bool hasKnownBatteryProtocol(const DeviceDefinition &definition);
[[nodiscard]] QByteArray batteryQueryFor(const DeviceDefinition &definition);
[[nodiscard]] std::optional<ProtocolProfile>
builtInBatteryProfileFor(const DeviceDefinition &definition);
[[nodiscard]] std::optional<ProtocolProfile> batteryProfileFor(
    const DeviceDefinition &definition,
    const QString &overridePath = {},
    QString *error = nullptr);

} // namespace strikepro
