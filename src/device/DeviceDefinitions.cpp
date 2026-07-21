#include "DeviceDefinitions.h"

#include <algorithm>

namespace strikepro {
namespace {

QString fromUtf8(const std::string_view value)
{
    return QString::fromUtf8(
        value.data(),
        static_cast<qsizetype>(value.size()));
}

} // namespace

QString DeviceDefinition::idString() const
{
    return fromUtf8(id);
}

QString DeviceDefinition::displayNameString() const
{
    return fromUtf8(displayName);
}

QString DeviceDefinition::artworkResourceString() const
{
    return fromUtf8(artworkResource);
}

QString DeviceDefinition::batteryProtocolString() const
{
    return fromUtf8(batteryProtocol);
}

std::span<const DeviceDefinition> supportedDeviceDefinitions()
{
    return generated::kDeviceDefinitions;
}

const DeviceDefinition *
findDeviceDefinition(const quint16 vendorId, const quint16 productId)
{
    const std::span definitions = supportedDeviceDefinitions();
    const auto found = std::ranges::find_if(
        definitions,
        [vendorId, productId](const DeviceDefinition &definition) {
            return definition.matches(vendorId, productId);
        });
    return found == definitions.end() ? nullptr : &*found;
}

const DeviceDefinition *findDeviceDefinitionByProductId(const quint16 productId)
{
    const std::span definitions = supportedDeviceDefinitions();
    const DeviceDefinition *match = nullptr;
    for (const DeviceDefinition &definition : definitions) {
        if (!definition.matchesProduct(productId)) {
            continue;
        }
        if (match != nullptr) {
            return nullptr;
        }
        match = &definition;
    }
    return match;
}

} // namespace strikepro
