#include "gui/TrayIndicator.h"

#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QtTest>

using strikepro::TrayIndicator;

class TrayIndicatorTest final : public QObject {
    Q_OBJECT

  private slots:
    void connectedBatteryPresentation()
    {
        const TrayIndicator::State state{
            .deviceName = QStringLiteral("MSI Strike Pro"),
            .connectionState = TrayIndicator::ConnectionState::Connected,
            .batteryPercent = 67,
            .charging = false,
        };

        QCOMPARE(
            TrayIndicator::toolTipForState(state),
            QStringLiteral("MSI Strike Pro\nBattery: 67% · On battery"));
        const QPixmap pixmap =
            TrayIndicator::iconForState(state).pixmap(QSize(64, 64));
        QVERIFY(!pixmap.isNull());
        QVERIFY(pixmap.toImage().pixelColor(32, 32).alpha() > 0);
    }

    void chargingPresentation()
    {
        const TrayIndicator::State state{
            .deviceName = QStringLiteral("MSI Strike Pro"),
            .connectionState = TrayIndicator::ConnectionState::Connected,
            .batteryPercent = 84,
            .charging = true,
        };

        QCOMPARE(
            TrayIndicator::toolTipForState(state),
            QStringLiteral("MSI Strike Pro\nBattery: 84% · Charging"));
        QVERIFY(!TrayIndicator::iconForState(state).isNull());
    }

    void unavailablePresentation()
    {
        const TrayIndicator::State state;

        QCOMPARE(
            TrayIndicator::toolTipForState(state),
            QStringLiteral("MSI Keyboard\nNo supported keyboard detected"));
        QVERIFY(!TrayIndicator::iconForState(state).isNull());
    }

    void problemPresentationDoesNotClaimBatteryState()
    {
        const TrayIndicator::State state{
            .deviceName = QStringLiteral("MSI Strike Pro"),
            .connectionState = TrayIndicator::ConnectionState::Problem,
            .batteryPercent = 42,
            .charging = std::nullopt,
        };

        QCOMPARE(
            TrayIndicator::toolTipForState(state),
            QStringLiteral("MSI Strike Pro\nConnection problem"));
    }
};

QTEST_MAIN(TrayIndicatorTest)

#include "TrayIndicatorTest.moc"
