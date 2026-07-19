#include "gui/BatteryGauge.h"
#include "gui/DebugWindow.h"

#include <QCoreApplication>
#include <QImage>
#include <QPainter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTest>

using namespace strikepro;

class BatteryGaugeTest final : public QObject {
    Q_OBJECT

  private slots:
    void loadsKeyboardArtwork();
    void keepsTelemetryOptional();
    void rendersUnknownState();
    void boundsPercentage();
    void usesBatteryLevelColors();
};

void BatteryGaugeTest::loadsKeyboardArtwork()
{
    QPixmap artwork(QStringLiteral(":/assets/keyboard/strike_pro.webp"));
    if (artwork.isNull()) {
        QVERIFY(
            artwork.load(QStringLiteral(":/assets/keyboard/strike_pro.png")));
    }
    QVERIFY(!artwork.isNull());
    QVERIFY(artwork.width() > 500);
    QVERIFY(artwork.height() > 300);
}

void BatteryGaugeTest::keepsTelemetryOptional()
{
    DebugWindow debugWindow(nullptr);
    auto *tabs = debugWindow.findChild<QTabWidget *>();
    auto *table =
        debugWindow.findChild<QTableWidget *>(QStringLiteral("reportTable"));
    QVERIFY(tabs != nullptr);
    QVERIFY(table != nullptr);
    QCOMPARE(tabs->count(), 2);
    QCOMPARE(tabs->tabText(0), QStringLiteral("Logs"));
    QCOMPARE(tabs->tabText(1), QStringLiteral("Telemetry"));

    const HidReport report{
        .devNode = QStringLiteral("/dev/hidraw-test"),
        .interfaceNumber = 1,
        .productId = kStrikeProWirelessProductId,
        .source = ReportSource::Input,
        .requestedReportId = -1,
        .data = QByteArray::fromHex("0db00100000005025d02"),
    };

    debugWindow.showTab(DebugWindow::Tab::Logs);
    QCoreApplication::processEvents();
    debugWindow.recordReport(report);
    QCOMPARE(table->rowCount(), 0);

    debugWindow.showTab(DebugWindow::Tab::Telemetry);
    QCoreApplication::processEvents();
    debugWindow.recordReport(report);
    QCOMPARE(table->rowCount(), 1);
}

void BatteryGaugeTest::rendersUnknownState()
{
    BatteryGauge gauge;
    gauge.resize(250, 250);
    gauge.setDeviceConnected(true);

    QImage image(gauge.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    gauge.render(&painter);

    int paintedPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alpha() != 0) {
                ++paintedPixels;
            }
        }
    }
    QVERIFY(paintedPixels > 1000);
    QVERIFY(!gauge.value().has_value());
}

void BatteryGaugeTest::boundsPercentage()
{
    BatteryGauge gauge;
    gauge.setValue(140);
    QVERIFY(gauge.value().has_value());
    QCOMPARE(*gauge.value(), 100);

    gauge.setValue(-20);
    QCOMPARE(*gauge.value(), 0);

    gauge.setValue(std::nullopt);
    QVERIFY(!gauge.value().has_value());
}

void BatteryGaugeTest::usesBatteryLevelColors()
{
    const QColor low = BatteryGauge::colorForValue(0.0);
    const QColor middle = BatteryGauge::colorForValue(50.0);
    const QColor high = BatteryGauge::colorForValue(100.0);

    QVERIFY(low.red() > low.green());
    QVERIFY(middle.red() > 200);
    QVERIFY(middle.green() > 200);
    QVERIFY(middle.blue() < 100);
    QVERIFY(high.green() > high.red());
    QCOMPARE(BatteryGauge::colorForValue(-20.0), low);
    QCOMPARE(BatteryGauge::colorForValue(120.0), high);
}

QTEST_MAIN(BatteryGaugeTest)
#include "BatteryGaugeTest.moc"
