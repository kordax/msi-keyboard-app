#include "device/BatteryDecoder.h"

#include <QTest>

using namespace strikepro;

class BatteryDecoderTest final : public QObject {
    Q_OBJECT

  private slots:
    void decodesCapturedWirelessBatteryResponse();
    void decodesCapturedWiredChargingResponse();
    void rejectsUnavailableWirelessResponse();
    void rejectsDifferentVendorResponse();
    void decodesConfirmedByte();
    void decodesFeatureByRequestedId();
    void decodesQueriedInputByRequestedId();
    void rejectsWrongReport();
    void rejectsOutOfRangePercentage();
};

void BatteryDecoderTest::decodesCapturedWirelessBatteryResponse()
{
    const ProtocolProfile profile = BatteryDecoder::confirmedStrikeProProfile();
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 1,
        .productId = kStrikeProWirelessProductId,
        .source = ReportSource::Input,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0db00100000005024e02000000000000"),
    };

    const auto reading = BatteryDecoder::decode(report, profile);
    QVERIFY(reading.has_value());
    QCOMPARE(reading->percent, 78);
    QVERIFY(reading->charging.has_value());
    QVERIFY(!*reading->charging);
}

void BatteryDecoderTest::decodesCapturedWiredChargingResponse()
{
    const ProtocolProfile profile = BatteryDecoder::confirmedStrikeProProfile();
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 1,
        .productId = kStrikeProWiredProductId,
        .source = ReportSource::Input,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0db00100000005025101000000000000"),
    };

    const auto reading = BatteryDecoder::decode(report, profile);
    QVERIFY(reading.has_value());
    QCOMPARE(reading->percent, 81);
    QVERIFY(reading->charging.has_value());
    QVERIFY(*reading->charging);
}

void BatteryDecoderTest::rejectsUnavailableWirelessResponse()
{
    const ProtocolProfile profile = BatteryDecoder::confirmedStrikeProProfile();
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 1,
        .productId = kStrikeProWirelessProductId,
        .source = ReportSource::Input,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0db00100000005020001000000000000"),
    };

    QVERIFY(!BatteryDecoder::decode(report, profile).has_value());
}

void BatteryDecoderTest::rejectsDifferentVendorResponse()
{
    const ProtocolProfile profile = BatteryDecoder::confirmedStrikeProProfile();
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 1,
        .productId = kStrikeProWirelessProductId,
        .source = ReportSource::Input,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0db00100000004024e02000000000000"),
    };

    QVERIFY(!BatteryDecoder::decode(report, profile).has_value());
}

void BatteryDecoderTest::decodesConfirmedByte()
{
    const ProtocolProfile profile{
        .source = ReportSource::Input,
        .interfaceNumber = 4,
        .reportId = 0x0d,
        .percentageOffset = 3,
        .chargingOffset = 4,
        .chargingMask = 0x02,
        .transportOffset = -1,
        .wirelessTransportValue = -1,
        .wiredTransportValue = -1,
        .matchPrefix = {},
        .path = {},
    };
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 4,
        .productId = 0,
        .source = ReportSource::Input,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0d00004b02"),
    };

    const auto reading = BatteryDecoder::decode(report, profile);
    QVERIFY(reading.has_value());
    QCOMPARE(reading->percent, 75);
    QVERIFY(reading->charging.has_value());
    QVERIFY(*reading->charging);
}

void BatteryDecoderTest::decodesFeatureByRequestedId()
{
    const ProtocolProfile profile{
        .source = ReportSource::Feature,
        .interfaceNumber = 2,
        .reportId = 0x0c,
        .percentageOffset = 2,
        .transportOffset = -1,
        .wirelessTransportValue = -1,
        .wiredTransportValue = -1,
        .matchPrefix = {},
        .path = {},
    };
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 2,
        .productId = 0,
        .source = ReportSource::Feature,
        .requestedReportId = 0x0c,
        .data = QByteArray::fromHex("000050"),
    };

    const auto reading = BatteryDecoder::decode(report, profile);
    QVERIFY(reading.has_value());
    QCOMPARE(reading->percent, 80);
}

void BatteryDecoderTest::decodesQueriedInputByRequestedId()
{
    const ProtocolProfile profile{
        .source = ReportSource::Input,
        .interfaceNumber = 4,
        .reportId = 0x0d,
        .percentageOffset = 2,
        .transportOffset = -1,
        .wirelessTransportValue = -1,
        .wiredTransportValue = -1,
        .matchPrefix = {},
        .path = {},
    };
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 4,
        .productId = 0,
        .source = ReportSource::Input,
        .requestedReportId = 0x0d,
        .data = QByteArray::fromHex("000050"),
    };

    const auto reading = BatteryDecoder::decode(report, profile);
    QVERIFY(reading.has_value());
    QCOMPARE(reading->percent, 80);
}

void BatteryDecoderTest::rejectsWrongReport()
{
    const ProtocolProfile profile{
        .source = ReportSource::Feature,
        .interfaceNumber = 2,
        .reportId = 0x07,
        .percentageOffset = 2,
        .transportOffset = -1,
        .wirelessTransportValue = -1,
        .wiredTransportValue = -1,
        .matchPrefix = {},
        .path = {},
    };
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 2,
        .productId = 0,
        .source = ReportSource::Feature,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0c0050"),
    };

    QVERIFY(!BatteryDecoder::decode(report, profile).has_value());
}

void BatteryDecoderTest::rejectsOutOfRangePercentage()
{
    const ProtocolProfile profile{
        .source = ReportSource::Input,
        .interfaceNumber = 4,
        .reportId = 0x0d,
        .percentageOffset = 1,
        .transportOffset = -1,
        .wirelessTransportValue = -1,
        .wiredTransportValue = -1,
        .matchPrefix = {},
        .path = {},
    };
    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 4,
        .productId = 0,
        .source = ReportSource::Input,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0dff"),
    };

    QVERIFY(!BatteryDecoder::decode(report, profile).has_value());
}

QTEST_MAIN(BatteryDecoderTest)
#include "BatteryDecoderTest.moc"
