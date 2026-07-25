#include "gui/TrayIndicator.h"

#include <QAction>
#include <QIcon>
#include <QImage>
#include <QMenu>
#include <QPixmap>
#include <QSignalSpy>
#include <QWidget>
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

        QCOMPARE(TrayIndicator::iconTextForState(state), QStringLiteral("67%"));
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

        QCOMPARE(TrayIndicator::iconTextForState(state), QStringLiteral("84%"));
        QCOMPARE(
            TrayIndicator::toolTipForState(state),
            QStringLiteral("MSI Strike Pro\nBattery: 84% · Charging"));
        QVERIFY(!TrayIndicator::iconForState(state).isNull());
    }

    void unavailablePresentation()
    {
        const TrayIndicator::State state;

        QCOMPARE(TrayIndicator::iconTextForState(state), QStringLiteral("—"));
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

        QCOMPARE(TrayIndicator::iconTextForState(state), QStringLiteral("!"));
        QCOMPARE(
            TrayIndicator::toolTipForState(state),
            QStringLiteral("MSI Strike Pro\nConnection problem"));
    }

    void deviceMenuSelectsLogicalDevice()
    {
        QWidget window;
        TrayIndicator indicator(&window);
        indicator.setDevices({
            TrayIndicator::DeviceEntry{
                .id = QStringLiteral("keyboard-a"),
                .name = QStringLiteral("MSI Strike Pro #1"),
                .detail = QStringLiteral("67% · USB"),
                .selected = true,
            },
            TrayIndicator::DeviceEntry{
                .id = QStringLiteral("keyboard-b"),
                .name = QStringLiteral("MSI Strike Pro #2"),
                .detail = QStringLiteral("2.4 GHz"),
                .selected = false,
            },
        });

        QMenu *devicesMenu =
            window.findChild<QMenu *>(QStringLiteral("trayDevicesMenu"));
        QVERIFY(devicesMenu != nullptr);
        QCOMPARE(devicesMenu->actions().size(), 2);
        QVERIFY(devicesMenu->actions().at(0)->isChecked());
        QCOMPARE(
            devicesMenu->actions().at(1)->data().toString(),
            QStringLiteral("keyboard-b"));

        QSignalSpy selected(&indicator, &TrayIndicator::deviceSelected);
        devicesMenu->actions().at(1)->trigger();
        QCOMPARE(selected.count(), 1);
        QCOMPARE(selected.at(0).at(0).toString(), QStringLiteral("keyboard-b"));
    }
};

QTEST_MAIN(TrayIndicatorTest)

#include "TrayIndicatorTest.moc"
