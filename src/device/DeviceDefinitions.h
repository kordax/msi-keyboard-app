#pragma once

#include <QString>
#include <QtGlobal>
#include <span>
#include <string_view>

namespace strikepro {

enum class DeviceTransport {
    Usb,
    Dongle,
};

// Add supported keyboard models in config/devices.json. CMake generates the
// constexpr records below and the matching udev rules from that single file.
struct DeviceDefinition {
    std::string_view id;
    std::string_view displayName;
    quint16 vendorId = 0;
    quint16 usbProductId = 0;
    quint16 dongleProductId = 0;
    std::string_view artworkResource;
    std::string_view batteryProtocol;
    int batteryInterfaceNumber = -1;

    [[nodiscard]] constexpr bool matches(
        const quint16 candidateVendorId, const quint16 candidateProductId) const
    {
        return candidateVendorId == vendorId
               && (candidateProductId == usbProductId
                   || (dongleProductId != 0
                       && candidateProductId == dongleProductId));
    }

    [[nodiscard]] constexpr bool
    matchesProduct(const quint16 candidateProductId) const
    {
        return candidateProductId == usbProductId
               || (dongleProductId != 0
                   && candidateProductId == dongleProductId);
    }

    [[nodiscard]] constexpr DeviceTransport
    transportForProduct(const quint16 candidateProductId) const
    {
        return candidateProductId == dongleProductId && dongleProductId != 0
                   ? DeviceTransport::Dongle
                   : DeviceTransport::Usb;
    }

    [[nodiscard]] constexpr bool supportsBattery() const
    {
        return !batteryProtocol.empty() && batteryInterfaceNumber >= 0;
    }

    [[nodiscard]] QString idString() const;
    [[nodiscard]] QString displayNameString() const;
    [[nodiscard]] QString artworkResourceString() const;
    [[nodiscard]] QString batteryProtocolString() const;
};

} // namespace strikepro

#include "DeviceDefinitionsData.h"

namespace strikepro {

[[nodiscard]] std::span<const DeviceDefinition> supportedDeviceDefinitions();
[[nodiscard]] const DeviceDefinition *
findDeviceDefinition(quint16 vendorId, quint16 productId);
[[nodiscard]] const DeviceDefinition *
findDeviceDefinitionByProductId(quint16 productId);

} // namespace strikepro
