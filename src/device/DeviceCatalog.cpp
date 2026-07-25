#include "DeviceCatalog.h"

#include <QCryptographicHash>
#include <QMap>
#include <QRegularExpression>

#include <algorithm>
#include <optional>

namespace strikepro {
namespace {

struct PhysicalEndpoint {
    QString id;
    QString name;
    QString uniqueId;
    quint16 vendorId = 0;
    quint16 productId = 0;
    QList<HidInterface> interfaces;
};

struct EndpointPair {
    std::optional<PhysicalEndpoint> usb;
    std::optional<PhysicalEndpoint> dongle;
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

QString stableToken(const QString &value)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Sha256)
            .toHex()
            .first(12));
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

QString logicalDeviceId(
    const DeviceDefinition &definition,
    const EndpointPair &pair,
    const bool onlyLogicalCandidate)
{
    const QString definitionId = definition.idString();
    const QString usbUnique =
        pair.usb.has_value() ? pair.usb->uniqueId : QString();
    const QString dongleUnique =
        pair.dongle.has_value() ? pair.dongle->uniqueId : QString();
    const QString sharedUnique =
        !usbUnique.isEmpty() && usbUnique == dongleUnique ? usbUnique
        : pair.usb.has_value() && !usbUnique.isEmpty()
                && !pair.dongle.has_value()
            ? usbUnique
        : pair.dongle.has_value() && !dongleUnique.isEmpty()
                && !pair.usb.has_value()
            ? dongleUnique
            : QString();
    if (!sharedUnique.isEmpty()) {
        return QStringLiteral("%1:id:%2")
            .arg(definitionId, stableToken(sharedUnique));
    }
    if (onlyLogicalCandidate) {
        return QStringLiteral("%1:0001").arg(definitionId);
    }
    if (pair.usb.has_value() && pair.dongle.has_value()) {
        return QStringLiteral("%1:pair:%2")
            .arg(
                definitionId,
                stableToken(pair.usb->id + QLatin1Char('|') + pair.dongle->id));
    }
    const PhysicalEndpoint &endpoint =
        pair.usb.has_value() ? *pair.usb : *pair.dongle;
    const QString transport =
        pair.usb.has_value() ? QStringLiteral("usb") : QStringLiteral("dongle");
    return QStringLiteral("%1:%2:%3")
        .arg(definitionId, transport, stableToken(endpoint.id));
}

QList<EndpointPair> pairEndpoints(
    const QList<PhysicalEndpoint> &usbEndpoints,
    const QList<PhysicalEndpoint> &dongleEndpoints)
{
    QList<EndpointPair> pairs;
    QList<bool> usbUsed(usbEndpoints.size(), false);
    QList<bool> dongleUsed(dongleEndpoints.size(), false);

    for (qsizetype usbIndex = 0; usbIndex < usbEndpoints.size(); ++usbIndex) {
        const QString uniqueId = usbEndpoints.at(usbIndex).uniqueId;
        if (uniqueId.isEmpty()) {
            continue;
        }
        qsizetype matchingDongle = -1;
        int matchCount = 0;
        for (qsizetype dongleIndex = 0; dongleIndex < dongleEndpoints.size();
             ++dongleIndex) {
            if (!dongleUsed.at(dongleIndex)
                && dongleEndpoints.at(dongleIndex).uniqueId == uniqueId) {
                matchingDongle = dongleIndex;
                ++matchCount;
            }
        }
        if (matchCount != 1) {
            continue;
        }
        usbUsed[usbIndex] = true;
        dongleUsed[matchingDongle] = true;
        pairs.push_back(EndpointPair{
            usbEndpoints.at(usbIndex),
            dongleEndpoints.at(matchingDongle)});
    }

    QList<PhysicalEndpoint> remainingUsb;
    QList<PhysicalEndpoint> remainingDongles;
    for (qsizetype index = 0; index < usbEndpoints.size(); ++index) {
        if (!usbUsed.at(index)) {
            remainingUsb.push_back(usbEndpoints.at(index));
        }
    }
    for (qsizetype index = 0; index < dongleEndpoints.size(); ++index) {
        if (!dongleUsed.at(index)) {
            remainingDongles.push_back(dongleEndpoints.at(index));
        }
    }

    // With one endpoint per transport there is only one possible pairing. With
    // multiple endpoints and no shared HID_UNIQ, guessing would merge different
    // physical keyboards, so ambiguous endpoints deliberately stay separate.
    if (remainingUsb.size() == 1 && remainingDongles.size() == 1) {
        pairs.push_back(EndpointPair{
            remainingUsb.takeFirst(),
            remainingDongles.takeFirst()});
    }
    for (const PhysicalEndpoint &endpoint : remainingUsb) {
        pairs.push_back(EndpointPair{endpoint, std::nullopt});
    }
    for (const PhysicalEndpoint &endpoint : remainingDongles) {
        pairs.push_back(EndpointPair{std::nullopt, endpoint});
    }
    return pairs;
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
        if (endpoint.uniqueId.isEmpty() && !interface.uniqueId.isEmpty()) {
            endpoint.uniqueId = interface.uniqueId;
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

        const QList<EndpointPair> pairs =
            pairEndpoints(usbEndpoints, dongleEndpoints);
        const bool onlyLogicalCandidate = pairs.size() == 1;
        devices.reserve(devices.size() + pairs.size());
        for (const EndpointPair &pair : pairs) {
            SupportedDevice device;
            device.definition = definition;
            device.id = logicalDeviceId(definition, pair, onlyLogicalCandidate);
            device.name = definition.displayNameString();
            if (pair.usb.has_value()) {
                appendEndpoint(device, *pair.usb);
                device.productId = definition.usbProductId;
            }
            if (pair.dongle.has_value()) {
                appendEndpoint(device, *pair.dongle);
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
