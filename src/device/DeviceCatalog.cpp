#include "DeviceCatalog.h"

#include <QMap>
#include <QRegularExpression>

#include <algorithm>

namespace strikepro {
namespace {

struct PhysicalEndpoint {
    QString id;
    QString name;
    quint16 vendorId = 0;
    quint16 productId = 0;
    QList<HidInterface> interfaces;
};

QString physicalEndpointId(const HidInterface &interface)
{
    QString endpointId = interface.sysfsPath;
    if (!endpointId.isEmpty()) {
        const qsizetype hidMarker =
            endpointId.indexOf(QStringLiteral("/0003:"));
        if (hidMarker >= 0) {
            endpointId.truncate(hidMarker);
        }
        static const QRegularExpression interfaceSuffix(
            QStringLiteral(R"(:[0-9]+\.[0-9]+$)"));
        endpointId.remove(interfaceSuffix);
        if (!endpointId.isEmpty()) {
            return endpointId;
        }
    }
    if (!interface.devNode.isEmpty()) {
        return interface.devNode;
    }
    return QStringLiteral("%1:%2:%3")
        .arg(interface.vendorId, 4, 16, QLatin1Char('0'))
        .arg(interface.productId, 4, 16, QLatin1Char('0'))
        .arg(interface.name);
}

void sortInterfaces(QList<HidInterface> &interfaces)
{
    std::ranges::sort(
        interfaces,
        [](const HidInterface &left, const HidInterface &right) {
            if (left.interfaceNumber != right.interfaceNumber) {
                return left.interfaceNumber < right.interfaceNumber;
            }
            return left.devNode < right.devNode;
        });
}

void appendEndpoint(SupportedDevice &device, const PhysicalEndpoint &endpoint)
{
    if (device.name.isEmpty() && !endpoint.name.isEmpty()) {
        device.name = endpoint.name;
    }
    device.interfaces.append(endpoint.interfaces);
}

const DeviceDefinition *findDefinition(
    const std::span<const DeviceDefinition> definitions,
    const HidInterface &interface)
{
    const auto found = std::ranges::find_if(
        definitions,
        [&interface](const DeviceDefinition &definition) {
            return definition.matches(interface.vendorId, interface.productId);
        });
    return found == definitions.end() ? nullptr : &*found;
}

} // namespace

bool SupportedDevice::supportsBattery() const
{
    return definition.supportsBattery();
}

const HidInterface *SupportedDevice::batteryInterface() const
{
    if (!supportsBattery()) {
        return nullptr;
    }

    QList<quint16> preferredProducts{definition.usbProductId};
    if (definition.dongleProductId != 0) {
        preferredProducts.push_back(definition.dongleProductId);
    }
    for (const bool requireAccess : {true, false}) {
        for (const quint16 preferredProduct : preferredProducts) {
            const auto found = std::ranges::find_if(
                interfaces,
                [this, preferredProduct, requireAccess](
                    const HidInterface &interface) {
                    const bool accessible =
                        interface.readable && interface.writable;
                    return interface.vendorId == definition.vendorId
                           && interface.productId == preferredProduct
                           && interface.interfaceNumber
                                  == definition.batteryInterfaceNumber
                           && (!requireAccess || accessible);
                });
            if (found != interfaces.end()) {
                return &*found;
            }
        }
    }
    return nullptr;
}

bool SupportedDevice::canQueryBattery() const
{
    const HidInterface *interface = batteryInterface();
    return supportsBattery() && interface != nullptr && interface->readable
           && interface->writable;
}

QList<SupportedDevice>
groupSupportedDevices(const QList<HidInterface> &interfaces)
{
    return groupSupportedDevices(interfaces, supportedDeviceDefinitions());
}

QList<SupportedDevice> groupSupportedDevices(
    const QList<HidInterface> &interfaces,
    const std::span<const DeviceDefinition> definitions)
{
    QMap<QString, PhysicalEndpoint> endpoints;
    for (const HidInterface &interface : interfaces) {
        if (findDefinition(definitions, interface) == nullptr) {
            continue;
        }

        const QString endpointId = physicalEndpointId(interface);
        PhysicalEndpoint &endpoint = endpoints[endpointId];
        endpoint.id = endpointId;
        endpoint.vendorId = interface.vendorId;
        endpoint.productId = interface.productId;
        if (endpoint.name.isEmpty() && !interface.name.isEmpty()) {
            endpoint.name = interface.name;
        }
        endpoint.interfaces.push_back(interface);
    }

    QList<SupportedDevice> devices;
    const auto byId = [](const PhysicalEndpoint &left,
                         const PhysicalEndpoint &right) {
        return left.id < right.id;
    };
    for (const DeviceDefinition &definition : definitions) {
        QList<PhysicalEndpoint> usbEndpoints;
        QList<PhysicalEndpoint> dongleEndpoints;
        for (PhysicalEndpoint endpoint : endpoints) {
            if (endpoint.vendorId != definition.vendorId) {
                continue;
            }
            sortInterfaces(endpoint.interfaces);
            if (endpoint.productId == definition.usbProductId) {
                usbEndpoints.push_back(std::move(endpoint));
            } else if (
                definition.dongleProductId != 0
                && endpoint.productId == definition.dongleProductId) {
                dongleEndpoints.push_back(std::move(endpoint));
            }
        }
        std::ranges::sort(usbEndpoints, byId);
        std::ranges::sort(dongleEndpoints, byId);

        const qsizetype deviceCount =
            std::max(usbEndpoints.size(), dongleEndpoints.size());
        devices.reserve(devices.size() + deviceCount);
        for (qsizetype index = 0; index < deviceCount; ++index) {
            SupportedDevice device;
            device.definition = definition;
            device.id = QStringLiteral("%1:%2")
                            .arg(definition.idString())
                            .arg(index + 1, 4, 10, QLatin1Char('0'));
            device.name = definition.displayNameString();
            if (index < usbEndpoints.size()) {
                appendEndpoint(device, usbEndpoints.at(index));
                device.productId = definition.usbProductId;
            }
            if (index < dongleEndpoints.size()) {
                appendEndpoint(device, dongleEndpoints.at(index));
                if (device.productId == 0) {
                    device.productId = definition.dongleProductId;
                }
            }
            sortInterfaces(device.interfaces);
            devices.push_back(std::move(device));
        }
    }
    return devices;
}

QString retainedDeviceSelection(
    const QList<SupportedDevice> &devices, const QString &selectedDeviceId)
{
    const auto selected =
        std::ranges::find(devices, selectedDeviceId, &SupportedDevice::id);
    if (selected != devices.end()) {
        return selectedDeviceId;
    }
    return devices.isEmpty() ? QString() : devices.first().id;
}

} // namespace strikepro
