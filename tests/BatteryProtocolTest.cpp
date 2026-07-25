#include "device/BatteryProtocol.h"

#include <QtTest>

class BatteryProtocolTest final : public QObject {
    Q_OBJECT

  private slots:
    void exposesConfirmedStrikeProProtocol()
    {
        const strikepro::DeviceDefinition &definition =
            strikepro::kStrikeProDeviceDefinition;

        QVERIFY(strikepro::hasKnownBatteryProtocol(definition));
        const QByteArray query = strikepro::batteryQueryFor(definition);
        QCOMPARE(query.size(), 65);
        QCOMPARE(static_cast<quint8>(query.at(0)), quint8{0x00});
        QCOMPARE(static_cast<quint8>(query.at(1)), quint8{0x0d});
        QCOMPARE(static_cast<quint8>(query.at(2)), quint8{0xb0});
        QCOMPARE(static_cast<quint8>(query.at(3)), quint8{0x01});
        QCOMPARE(static_cast<quint8>(query.at(7)), quint8{0x05});

        const auto profile = strikepro::builtInBatteryProfileFor(definition);
        QVERIFY(profile.has_value());
        QCOMPARE(profile->interfaceNumber, definition.batteryInterfaceNumber);
        QVERIFY(profile->canDecodePercentage());
    }

    void rejectsUnknownProtocolWithoutSendingData()
    {
        constexpr strikepro::DeviceDefinition definition{
            .id = std::string_view{"unknown-protocol"},
            .displayName = std::string_view{"Unknown protocol"},
            .vendorId = quint16{0x1234},
            .usbProductId = quint16{0x5678},
            .dongleProductId = quint16{0},
            .artworkResource = {},
            .batteryProtocol = std::string_view{"not-implemented"},
            .batteryInterfaceNumber = 1,
        };

        QVERIFY(!strikepro::hasKnownBatteryProtocol(definition));
        QVERIFY(strikepro::batteryQueryFor(definition).isEmpty());
        QVERIFY(!strikepro::builtInBatteryProfileFor(definition).has_value());

        QString error;
        QVERIFY(
            !strikepro::batteryProfileFor(definition, {}, &error).has_value());
        QVERIFY(error.contains(QStringLiteral("not-implemented")));
    }
};

QTEST_GUILESS_MAIN(BatteryProtocolTest)

#include "BatteryProtocolTest.moc"
