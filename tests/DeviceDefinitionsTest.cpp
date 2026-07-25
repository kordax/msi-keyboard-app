#include "device/DeviceDefinitions.h"
#include "device/BatteryProtocol.h"

#include <QSet>
#include <QtTest>

class DeviceDefinitionsTest final : public QObject {
    Q_OBJECT

  private slots:
    void definitionsAreExplicitAndUnique()
    {
        const std::span definitions = strikepro::supportedDeviceDefinitions();
        QVERIFY(!definitions.empty());

        QSet<QString> ids;
        QSet<quint32> usbIds;
        for (const strikepro::DeviceDefinition &definition : definitions) {
            QVERIFY(!definition.id.empty());
            QVERIFY(!definition.displayName.empty());
            QVERIFY(definition.vendorId != 0);
            QVERIFY(definition.usbProductId != 0);
            QVERIFY(!ids.contains(definition.idString()));
            ids.insert(definition.idString());

            const auto usbKey =
                (static_cast<quint32>(definition.vendorId) << 16U)
                | definition.usbProductId;
            QVERIFY(!usbIds.contains(usbKey));
            usbIds.insert(usbKey);
            QCOMPARE(
                strikepro::findDeviceDefinition(
                    definition.vendorId,
                    definition.usbProductId),
                &definition);
            QCOMPARE(
                definition.transportForProduct(definition.usbProductId),
                strikepro::DeviceTransport::Usb);

            if (definition.dongleProductId != 0) {
                const auto dongleKey =
                    (static_cast<quint32>(definition.vendorId) << 16U)
                    | definition.dongleProductId;
                QVERIFY(!usbIds.contains(dongleKey));
                usbIds.insert(dongleKey);
                QCOMPARE(
                    strikepro::findDeviceDefinition(
                        definition.vendorId,
                        definition.dongleProductId),
                    &definition);
                QCOMPARE(
                    definition.transportForProduct(definition.dongleProductId),
                    strikepro::DeviceTransport::Dongle);
            }

            QCOMPARE(
                definition.supportsBattery(),
                !definition.batteryProtocol.empty()
                    && definition.batteryInterfaceNumber >= 0);
            if (definition.supportsBattery()) {
                QVERIFY(strikepro::hasKnownBatteryProtocol(definition));
                QVERIFY(!strikepro::batteryQueryFor(definition).isEmpty());
                QVERIFY(strikepro::builtInBatteryProfileFor(definition)
                            .has_value());
            }
            if (definition.batteryQueryOverUsb) {
                QVERIFY(definition.supportsBattery());
            }
            if (definition.batteryQueryOverDongle) {
                QVERIFY(definition.supportsBattery());
                QVERIFY(definition.dongleProductId != 0);
            }
        }
    }

    void strikeProCompatibilityAliasesFollowTheCatalog()
    {
        const strikepro::DeviceDefinition &definition =
            strikepro::kStrikeProDeviceDefinition;
        QCOMPARE(definition.idString(), QStringLiteral("strike-pro"));
        QCOMPARE(strikepro::kMsiVendorId, definition.vendorId);
        QCOMPARE(strikepro::kStrikeProWiredProductId, definition.usbProductId);
        QCOMPARE(
            strikepro::kStrikeProWirelessProductId,
            definition.dongleProductId);
        QVERIFY(!definition.canQueryBatteryOver(definition.usbProductId));
        QVERIFY(definition.canQueryBatteryOver(definition.dongleProductId));
    }
};

QTEST_GUILESS_MAIN(DeviceDefinitionsTest)

#include "DeviceDefinitionsTest.moc"
