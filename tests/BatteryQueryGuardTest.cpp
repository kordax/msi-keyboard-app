#include "device/BatteryQueryGuard.h"
#include "device/HidDeviceScanner.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include <chrono>

using strikepro::BatteryQueryBlockReason;
using strikepro::BatteryQueryGuard;
using strikepro::HidDeviceScanner;
using namespace std::chrono_literals;

namespace {

bool writeAttribute(const QString &path, const QByteArray &value)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate)
           && file.write(value) == value.size();
}

} // namespace

class BatteryQueryGuardTest final : public QObject {
    Q_OBJECT

  private slots:
    void waitsForStableTopology()
    {
        const BatteryQueryGuard::TimePoint start{};
        BatteryQueryGuard guard(1s, start);

        QVERIFY(
            guard.blockReason(
                strikepro::kStrikeProDeviceDefinition,
                strikepro::kStrikeProWirelessProductId,
                false,
                start + 999ms)
            == BatteryQueryBlockReason::TopologySettling);
        QVERIFY(
            guard.blockReason(
                strikepro::kStrikeProDeviceDefinition,
                strikepro::kStrikeProWirelessProductId,
                false,
                start + 1s)
            == BatteryQueryBlockReason::None);

        guard.observeTopologyEvent(start + 2s);
        QVERIFY(
            guard.blockReason(
                strikepro::kStrikeProDeviceDefinition,
                strikepro::kStrikeProWirelessProductId,
                false,
                start + 2500ms)
            == BatteryQueryBlockReason::TopologySettling);
        QVERIFY(
            guard.blockReason(
                strikepro::kStrikeProDeviceDefinition,
                strikepro::kStrikeProWirelessProductId,
                false,
                start + 3s)
            == BatteryQueryBlockReason::None);
    }

    void blocksDongleWhileWiredUsbMayBePresent()
    {
        const BatteryQueryGuard::TimePoint start{};
        const BatteryQueryGuard guard(1s, start);

        QVERIFY(
            guard.blockReason(
                strikepro::kStrikeProDeviceDefinition,
                strikepro::kStrikeProWirelessProductId,
                true,
                start + 2s)
            == BatteryQueryBlockReason::WiredUsbMayBePresent);
    }

    void preservesPerTransportWritePolicy()
    {
        const BatteryQueryGuard::TimePoint start{};
        const BatteryQueryGuard guard(1s, start);

        QVERIFY(
            guard.blockReason(
                strikepro::kStrikeProDeviceDefinition,
                strikepro::kStrikeProWiredProductId,
                false,
                start + 2s)
            == BatteryQueryBlockReason::TransportDisabled);
    }

    void detectsWiredUsbBeforeHidrawExists()
    {
        QTemporaryDir usbDevices;
        QVERIFY(usbDevices.isValid());
        QDir root(usbDevices.path());
        QVERIFY(root.mkpath(QStringLiteral("1-2")));
        QVERIFY(writeAttribute(
            root.filePath(QStringLiteral("1-2/idVendor")),
            QByteArray("0db0\n")));
        QVERIFY(writeAttribute(
            root.filePath(QStringLiteral("1-2/idProduct")),
            QByteArray("b231\n")));

        QVERIFY(HidDeviceScanner::usbDeviceMayBePresent(
            strikepro::kStrikeProDeviceDefinition.vendorId,
            strikepro::kStrikeProWiredProductId,
            usbDevices.path()));
    }

    void ignoresOtherUsbProducts()
    {
        QTemporaryDir usbDevices;
        QVERIFY(usbDevices.isValid());
        QDir root(usbDevices.path());
        QVERIFY(root.mkpath(QStringLiteral("1-2")));
        QVERIFY(writeAttribute(
            root.filePath(QStringLiteral("1-2/idVendor")),
            QByteArray("0db0\n")));
        QVERIFY(writeAttribute(
            root.filePath(QStringLiteral("1-2/idProduct")),
            QByteArray("1620\n")));

        QVERIFY(!HidDeviceScanner::usbDeviceMayBePresent(
            strikepro::kStrikeProDeviceDefinition.vendorId,
            strikepro::kStrikeProWiredProductId,
            usbDevices.path()));
    }

    void failsClosedWhenUsbTopologyIsUnavailable()
    {
        QTemporaryDir usbDevices;
        QVERIFY(usbDevices.isValid());

        QVERIFY(HidDeviceScanner::usbDeviceMayBePresent(
            strikepro::kStrikeProDeviceDefinition.vendorId,
            strikepro::kStrikeProWiredProductId,
            usbDevices.filePath(QStringLiteral("missing"))));
    }
};

QTEST_GUILESS_MAIN(BatteryQueryGuardTest)

#include "BatteryQueryGuardTest.moc"
