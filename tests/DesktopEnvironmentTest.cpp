#include "gui/MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QEventLoop>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMenu>
#include <QPalette>
#include <QPixmap>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QWidget>
#include <QtTest>

namespace {

QPixmap renderWindow(strikepro::MainWindow &window)
{
    window.show();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QTest::qWait(50);
    return window.grab();
}

QPalette lightPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#f3f3f3")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#202020")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#ffffff")));
    palette.setColor(
        QPalette::AlternateBase,
        QColor(QStringLiteral("#ececec")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#202020")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#e6e6e6")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#202020")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#b62f49")));
    palette.setColor(
        QPalette::HighlightedText,
        QColor(QStringLiteral("#ffffff")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#ffffff")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#202020")));
    return palette;
}

} // namespace

class DesktopEnvironmentTest final : public QObject {
    Q_OBJECT

  private slots:
    void rendersWithLightTheme()
    {
        const QPalette original = qApp->palette();
        qApp->setPalette(lightPalette());
        QVERIFY(qApp->palette().color(QPalette::Window).lightness() > 128);

        strikepro::MainWindow window;
        const QPixmap capture = renderWindow(window);
        QVERIFY(window.isVisible());
        QVERIFY(window.centralWidget() != nullptr);
        QVERIFY(window.centralWidget()->size().isValid());
        QVERIFY(!capture.isNull());

        qApp->setPalette(original);
    }

    void rendersAtHighDpi()
    {
        strikepro::MainWindow window;
        const QPixmap capture = renderWindow(window);

        QVERIFY(window.devicePixelRatioF() >= 1.9);
        QVERIFY(capture.devicePixelRatio() >= 1.9);
        QCOMPARE(capture.deviceIndependentSize().toSize(), window.size());
    }

    void rendersDesignVariants_data()
    {
        QTest::addColumn<QString>("design");
        QTest::addColumn<int>("maximumWidth");
        QTest::newRow("balanced") << QStringLiteral("balanced") << 1280;
        QTest::newRow("compact") << QStringLiteral("compact") << 1060;
        QTest::newRow("showcase") << QStringLiteral("showcase") << 1440;
    }

    void rendersDesignVariants()
    {
        QFETCH(QString, design);
        QFETCH(int, maximumWidth);

        QSettings settings;
        const QVariant previous = settings.value(QStringLiteral("ui/design"));
        settings.setValue(QStringLiteral("ui/design"), design);

        strikepro::MainWindow window;
        const QPixmap capture = renderWindow(window);
        QWidget *content =
            window.findChild<QWidget *>(QStringLiteral("content"));
        QMenu *designMenu =
            window.findChild<QMenu *>(QStringLiteral("designMenu"));
        QVERIFY(content != nullptr);
        QVERIFY(designMenu != nullptr);
        QVERIFY(!designMenu->menuAction()->isVisible());
        QCOMPARE(content->maximumWidth(), maximumWidth);
        QVERIFY(!capture.isNull());

        if (previous.isValid()) {
            settings.setValue(QStringLiteral("ui/design"), previous);
        } else {
            settings.remove(QStringLiteral("ui/design"));
        }
    }

    void persistsTrayPreference()
    {
        QSettings settings;
        const QVariant previous =
            settings.value(QStringLiteral("ui/keepInTray"));
        settings.setValue(QStringLiteral("ui/keepInTray"), false);

        {
            strikepro::MainWindow window;
            QAction *closeToTray =
                window.findChild<QAction *>(QStringLiteral("keepInTrayAction"));
            QSystemTrayIcon *trayIcon = window.findChild<QSystemTrayIcon *>();
            QAction *mainQuitAction =
                window.findChild<QAction *>(QStringLiteral("mainQuitAction"));
            QAction *trayQuitAction =
                window.findChild<QAction *>(QStringLiteral("trayQuitAction"));
            QVERIFY(closeToTray != nullptr);
            QVERIFY(trayIcon != nullptr);
            QVERIFY(mainQuitAction != nullptr);
            QVERIFY(trayQuitAction != nullptr);
            QVERIFY(mainQuitAction != trayQuitAction);
            QCOMPARE(closeToTray->text(), QStringLiteral("Close to Tray"));
            QCOMPARE(mainQuitAction->text(), QStringLiteral("Quit"));
            QCOMPARE(trayQuitAction->text(), QStringLiteral("Quit"));
            QCOMPARE(
                mainQuitAction->shortcut(), QKeySequence(QKeySequence::Quit));
            QCOMPARE(mainQuitAction->menuRole(), QAction::QuitRole);
            QVERIFY(!closeToTray->isChecked());
            QVERIFY(QApplication::quitOnLastWindowClosed());

            closeToTray->setChecked(true);
            QCOMPARE(
                settings.value(QStringLiteral("ui/keepInTray")).toBool(),
                true);
            QVERIFY(!QApplication::quitOnLastWindowClosed());
            QVERIFY(trayIcon->isVisible());

            if (QSystemTrayIcon::isSystemTrayAvailable()) {
                window.show();
                QTRY_VERIFY(window.isVisible());
                QVERIFY(!window.close());
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                QVERIFY(!window.isVisible());
                QVERIFY(trayIcon->isVisible());
                QCOMPARE(trayQuitAction->text(), QStringLiteral("Quit"));
            }

            closeToTray->setChecked(false);
            QVERIFY(QApplication::quitOnLastWindowClosed());
            QVERIFY(!trayIcon->isVisible());
        }

        if (previous.isValid()) {
            settings.setValue(QStringLiteral("ui/keepInTray"), previous);
        } else {
            settings.remove(QStringLiteral("ui/keepInTray"));
        }
        QApplication::setQuitOnLastWindowClosed(true);
    }

    void rendersOnWayland()
    {
        QCOMPARE(QGuiApplication::platformName(), QStringLiteral("wayland"));

        strikepro::MainWindow window;
        const QPixmap capture = renderWindow(window);
        QVERIFY(window.isVisible());
        QVERIFY(!capture.isNull());
    }
};

QTEST_MAIN(DesktopEnvironmentTest)

#include "DesktopEnvironmentTest.moc"
