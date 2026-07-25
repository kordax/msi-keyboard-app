#pragma once

#include "DeviceDefinitions.h"

#include <QByteArray>
#include <span>

namespace strikepro {

[[nodiscard]] bool isSupportedDeviceUevent(const QByteArray &event);
[[nodiscard]] bool isSupportedDeviceUevent(
    const QByteArray &event, std::span<const DeviceDefinition> definitions);

} // namespace strikepro
