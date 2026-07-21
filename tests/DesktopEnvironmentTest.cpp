#include "gui/MainWindow.h"

#include <QApplication>
#include <QEventLoop>
#include <QGuiApplication>
#include <QPalette>
#include <QPixmap>
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
