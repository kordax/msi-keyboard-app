#include "device/DeviceCatalog.h"

#include <QtTest>

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
    const bool writable = true)
{
    return HidInterface{
        .devNode =
            QStringLiteral("/dev/%1-if%2").arg(deviceId).arg(interfaceNumber),
        .sysfsPath = deviceId,
        .name = QStringLiteral("MSI Strike Pro"),
        .vendorId = strikepro::kMsiVendorId,
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
            strikepro::kStrikeProWiredProductId);
    }

    void fallsBackToAccessibleDongleTransport()
    {
        SupportedDevice device;
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
            strikepro::kStrikeProWiredProductId);
    }

    void exposesBatteryCapabilityAndAccess()
    {
        SupportedDevice device;
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
