#include "DeviceCatalog.h"

#include <QMap>
#include <QRegularExpression>

#include <algorithm>

namespace strikepro {
namespace {

struct PhysicalEndpoint {
    QString id;
    QString name;
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
    return QStringLiteral("%1:%2")
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

} // namespace

bool SupportedDevice::supportsBattery() const
{
    return std::ranges::any_of(interfaces, [](const HidInterface &interface) {
        return isStrikeProProduct(interface.productId);
    });
}

const HidInterface *SupportedDevice::batteryInterface() const
{
    for (const bool requireAccess : {true, false}) {
        for (const quint16 preferredProduct :
             {kStrikeProWiredProductId, kStrikeProWirelessProductId}) {
            const auto found = std::ranges::find_if(
                interfaces,
                [preferredProduct,
                 requireAccess](const HidInterface &interface) {
                    const bool accessible =
                        interface.readable && interface.writable;
                    return interface.productId == preferredProduct
                           && interface.interfaceNumber == 1
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
    QMap<QString, PhysicalEndpoint> endpoints;
    for (const HidInterface &interface : interfaces) {
        if (!interface.isTarget()) {
            continue;
        }

        const QString endpointId = physicalEndpointId(interface);
        PhysicalEndpoint &endpoint = endpoints[endpointId];
        endpoint.id = endpointId;
        endpoint.productId = interface.productId;
        if (endpoint.name.isEmpty() && !interface.name.isEmpty()) {
            endpoint.name = interface.name;
        }
        endpoint.interfaces.push_back(interface);
    }

    QList<PhysicalEndpoint> wired;
    QList<PhysicalEndpoint> wireless;
    for (PhysicalEndpoint endpoint : endpoints) {
        sortInterfaces(endpoint.interfaces);
        if (endpoint.productId == kStrikeProWiredProductId) {
            wired.push_back(std::move(endpoint));
        } else if (endpoint.productId == kStrikeProWirelessProductId) {
            wireless.push_back(std::move(endpoint));
        }
    }
    const auto byId = [](const PhysicalEndpoint &left,
                         const PhysicalEndpoint &right) {
        return left.id < right.id;
    };
    std::ranges::sort(wired, byId);
    std::ranges::sort(wireless, byId);

    const qsizetype deviceCount = std::max(wired.size(), wireless.size());
    QList<SupportedDevice> devices;
    devices.reserve(deviceCount);
    for (qsizetype index = 0; index < deviceCount; ++index) {
        SupportedDevice device;
        device.id = QStringLiteral("strike-pro:%1")
                        .arg(index + 1, 4, 10, QLatin1Char('0'));
        if (index < wired.size()) {
            appendEndpoint(device, wired.at(index));
            device.productId = kStrikeProWiredProductId;
        }
        if (index < wireless.size()) {
            appendEndpoint(device, wireless.at(index));
            if (device.productId == 0) {
                device.productId = kStrikeProWirelessProductId;
            }
        }
        sortInterfaces(device.interfaces);
        devices.push_back(std::move(device));
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
