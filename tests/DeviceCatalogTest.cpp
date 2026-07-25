#include "device/DeviceCatalog.h"

#include <QtTest>

#include <QSet>
#include <algorithm>
#include <array>

using strikepro::groupSupportedDevices;
using strikepro::HidInterface;
using strikepro::retainedDeviceSelection;
using strikepro::SupportedDevice;

namespace {

HidInterface makeInterface(
    const QString &deviceId,
    const quint16 productId,
    const int interfaceNumber,
    const bool readable = true,
    const bool writable = true,
    const quint16 vendorId = strikepro::kMsiVendorId,
    const QString &uniqueId = {})
{
    return HidInterface{
        .devNode =
            QStringLiteral("/dev/%1-if%2").arg(deviceId).arg(interfaceNumber),
        .sysfsPath = deviceId,
        .name = QStringLiteral("Test keyboard"),
        .uniqueId = uniqueId,
        .vendorId = vendorId,
        .productId = productId,
        .interfaceNumber = interfaceNumber,
        .readable = readable,
        .writable = writable,
        .reportDescriptor = QByteArray(),
    };
}

} // namespace

class DeviceCatalogTest final : public QObject {
    Q_OBJECT

  private slots:
    void keepsSeparateWiredDevices()
    {
        const QList<HidInterface> interfaces{
            makeInterface(
                QStringLiteral("usb-a"),
                strikepro::kStrikeProWiredProductId,
                4),
            makeInterface(
                QStringLiteral("usb-a"),
                strikepro::kStrikeProWiredProductId,
                1),
            makeInterface(
                QStringLiteral("usb-b"),
                strikepro::kStrikeProWiredProductId,
                1),
        };

        const QList<SupportedDevice> devices =
            groupSupportedDevices(interfaces);

        QCOMPARE(devices.size(), 2);
        QCOMPARE(devices.at(0).interfaces.size(), 2);
        QCOMPARE(devices.at(0).interfaces.first().interfaceNumber, 1);
        QCOMPARE(devices.at(1).interfaces.size(), 1);
        QVERIFY(devices.at(0).id != devices.at(1).id);
    }

    void mergesWiredAndDongleTransports()
    {
        const QList<SupportedDevice> devices = groupSupportedDevices({
            makeInterface(
                QStringLiteral("wired"),
                strikepro::kStrikeProWiredProductId,
                1),
            makeInterface(
                QStringLiteral("receiver"),
                strikepro::kStrikeProWirelessProductId,
                1),
        });

        QCOMPARE(devices.size(), 1);
        QCOMPARE(devices.first().interfaces.size(), 2);
        QVERIFY(devices.first().canQueryBattery());
        QCOMPARE(
            devices.first().batteryInterface()->productId,
            strikepro::kStrikeProWirelessProductId);
    }

    void neverSelectsUnsafeUsbForBatteryQueries()
    {
        const QList<SupportedDevice> devices = groupSupportedDevices({
            makeInterface(
                QStringLiteral("wired"),
                strikepro::kStrikeProWiredProductId,
                1),
            makeInterface(
                QStringLiteral("receiver"),
                strikepro::kStrikeProWirelessProductId,
                1),
        });

        QCOMPARE(devices.size(), 1);
        const SupportedDevice &device = devices.first();
        QCOMPARE(
            device.batteryInterface(
                      strikepro::kStrikeProWiredProductId,
                      0)
                ->productId,
            strikepro::kStrikeProWirelessProductId);
        QCOMPARE(
            device.batteryInterface(
                      0,
                      strikepro::kStrikeProWirelessProductId)
                ->productId,
            strikepro::kStrikeProWirelessProductId);

        const QList<SupportedDevice> wiredOnly = groupSupportedDevices({
            makeInterface(
                QStringLiteral("wired"),
                strikepro::kStrikeProWiredProductId,
                1),
        });
        QCOMPARE(wiredOnly.size(), 1);
        QVERIFY(wiredOnly.first().batteryInterface() == nullptr);
        QVERIFY(!wiredOnly.first().canQueryBattery());
    }

    void fallsBackToAccessibleDongleTransport()
    {
        SupportedDevice device;
        device.definition = strikepro::kStrikeProDeviceDefinition;
        device.productId = strikepro::kStrikeProWiredProductId;
        device.interfaces = {
            makeInterface(
                QStringLiteral("wired"),
                strikepro::kStrikeProWiredProductId,
                1,
                true,
                false),
            makeInterface(
                QStringLiteral("receiver"),
                strikepro::kStrikeProWirelessProductId,
                1),
        };

        QVERIFY(device.canQueryBattery());
        QCOMPARE(
            device.batteryInterface()->productId,
            strikepro::kStrikeProWirelessProductId);

        device.interfaces.first().writable = true;
        QCOMPARE(
            device.batteryInterface()->productId,
            strikepro::kStrikeProWirelessProductId);
    }

    void exposesBatteryCapabilityAndAccess()
    {
        SupportedDevice device;
        device.definition = strikepro::kStrikeProDeviceDefinition;
        device.productId = strikepro::kStrikeProWirelessProductId;
        device.interfaces.push_back(makeInterface(
            QStringLiteral("receiver"),
            strikepro::kStrikeProWirelessProductId,
            1,
            true,
            false));

        QVERIFY(device.supportsBattery());
        QVERIFY(device.batteryInterface() != nullptr);
        QVERIFY(!device.canQueryBattery());

        device.interfaces.first().writable = true;
        QVERIFY(device.canQueryBattery());
    }

    void groupsDifferentConfiguredModelsIndependently()
    {
        constexpr strikepro::DeviceDefinition compactKeyboard{
            .id = std::string_view{"compact-test"},
            .displayName = std::string_view{"Compact Test"},
            .vendorId = quint16{0x1234},
            .usbProductId = quint16{0x1001},
            .dongleProductId = quint16{0x1002},
            .artworkResource = {},
            .batteryProtocol = {},
            .batteryInterfaceNumber = -1,
        };
        constexpr std::array definitions{
            strikepro::kStrikeProDeviceDefinition,
            compactKeyboard,
        };

        const QList<SupportedDevice> devices = groupSupportedDevices(
            {
                makeInterface(
                    QStringLiteral("strike-usb"),
                    strikepro::kStrikeProWiredProductId,
                    1),
                makeInterface(
                    QStringLiteral("compact-usb"),
                    compactKeyboard.usbProductId,
                    1,
                    true,
                    true,
                    compactKeyboard.vendorId),
                makeInterface(
                    QStringLiteral("compact-dongle"),
                    compactKeyboard.dongleProductId,
                    1,
                    true,
                    true,
                    compactKeyboard.vendorId),
            },
            std::span<const strikepro::DeviceDefinition>{definitions});

        QCOMPARE(devices.size(), 2);
        QCOMPARE(
            devices.at(0).definition.idString(),
            QStringLiteral("strike-pro"));
        QCOMPARE(
            devices.at(1).definition.idString(),
            QStringLiteral("compact-test"));
        QCOMPARE(devices.at(1).interfaces.size(), 2);
        QVERIFY(!devices.at(1).supportsBattery());
    }

    void doesNotGuessAmbiguousTransportPairs()
    {
        const QList<SupportedDevice> devices = groupSupportedDevices({
            makeInterface(
                QStringLiteral("usb-a"),
                strikepro::kStrikeProWiredProductId,
                1),
            makeInterface(
                QStringLiteral("usb-b"),
                strikepro::kStrikeProWiredProductId,
                1),
            makeInterface(
                QStringLiteral("dongle-a"),
                strikepro::kStrikeProWirelessProductId,
                1),
            makeInterface(
                QStringLiteral("dongle-b"),
                strikepro::kStrikeProWirelessProductId,
                1),
        });

        QCOMPARE(devices.size(), 4);
        QSet<QString> ids;
        for (const SupportedDevice &device : devices) {
            QCOMPARE(device.interfaces.size(), 1);
            ids.insert(device.id);
        }
        QCOMPARE(ids.size(), 4);
    }

    void pairsTransportsUsingSharedUniqueId()
    {
        const auto interfaceFor = [](const QString &endpoint,
                                     const quint16 productId,
                                     const QString &uniqueId) {
            return makeInterface(
                endpoint,
                productId,
                1,
                true,
                true,
                strikepro::kMsiVendorId,
                uniqueId);
        };
        const QList<HidInterface> interfaces{
            interfaceFor(
                QStringLiteral("usb-b"),
                strikepro::kStrikeProWiredProductId,
                QStringLiteral("keyboard-b")),
            interfaceFor(
                QStringLiteral("dongle-a"),
                strikepro::kStrikeProWirelessProductId,
                QStringLiteral("keyboard-a")),
            interfaceFor(
                QStringLiteral("usb-a"),
                strikepro::kStrikeProWiredProductId,
                QStringLiteral("keyboard-a")),
            interfaceFor(
                QStringLiteral("dongle-b"),
                strikepro::kStrikeProWirelessProductId,
                QStringLiteral("keyboard-b")),
        };

        const QList<SupportedDevice> devices =
            groupSupportedDevices(interfaces);
        QCOMPARE(devices.size(), 2);
        QSet<QString> ids;
        for (const SupportedDevice &device : devices) {
            QCOMPARE(device.interfaces.size(), 2);
            ids.insert(device.id);
            const QString uniqueId = device.interfaces.first().uniqueId;
            QVERIFY(!uniqueId.isEmpty());
            for (const HidInterface &interface : device.interfaces) {
                QCOMPARE(interface.uniqueId, uniqueId);
            }
        }
        QCOMPARE(ids.size(), 2);

        QList<HidInterface> reversed = interfaces;
        std::ranges::reverse(reversed);
        const QList<SupportedDevice> regrouped =
            groupSupportedDevices(reversed);
        QSet<QString> regroupedIds;
        for (const SupportedDevice &device : regrouped) {
            regroupedIds.insert(device.id);
        }
        QCOMPARE(regroupedIds, ids);
    }

    void keepsSelectionAcrossTransportHotplug()
    {
        const QList<SupportedDevice> receiverOnly = groupSupportedDevices({
            makeInterface(
                QStringLiteral("receiver"),
                strikepro::kStrikeProWirelessProductId,
                1),
        });
        const QList<SupportedDevice> bothTransports = groupSupportedDevices({
            makeInterface(
                QStringLiteral("wired"),
                strikepro::kStrikeProWiredProductId,
                1),
            makeInterface(
                QStringLiteral("receiver"),
                strikepro::kStrikeProWirelessProductId,
                1),
        });
        const QList<SupportedDevice> wiredOnly = groupSupportedDevices({
            makeInterface(
                QStringLiteral("wired"),
                strikepro::kStrikeProWiredProductId,
                1),
        });

        QCOMPARE(receiverOnly.size(), 1);
        QCOMPARE(bothTransports.size(), 1);
        QCOMPARE(wiredOnly.size(), 1);
        QCOMPARE(receiverOnly.first().id, bothTransports.first().id);
        QCOMPARE(wiredOnly.first().id, bothTransports.first().id);
        QCOMPARE(
            retainedDeviceSelection(bothTransports, receiverOnly.first().id),
            receiverOnly.first().id);
        QVERIFY(
            retainedDeviceSelection({}, QStringLiteral("removed")).isEmpty());
    }
};

QTEST_GUILESS_MAIN(DeviceCatalogTest)

#include "DeviceCatalogTest.moc"
