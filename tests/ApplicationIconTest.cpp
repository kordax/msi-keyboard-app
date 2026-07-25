#include "gui/ApplicationIcon.h"

#include <QFile>
#include <QIcon>
#include <QtTest>

class ApplicationIconTest : public QObject {
    Q_OBJECT

  private slots:
    void selectsIconForDesktop_data()
    {
        QTest::addColumn<QString>("desktop");
        QTest::addColumn<QString>("expected");

        const QString defaultIcon =
            QStringLiteral(":/assets/icons/io.github.kordax.MsiKeyboard.svg");
        const QString cinnamonIcon = QStringLiteral(
            ":/assets/icons/io.github.kordax.MsiKeyboard-cinnamon.svg");

        QTest::newRow("unspecified") << QString() << defaultIcon;
        QTest::newRow("gnome") << QStringLiteral("GNOME") << defaultIcon;
        QTest::newRow("plasma") << QStringLiteral("KDE") << defaultIcon;
        QTest::newRow("cinnamon")
            << QStringLiteral("X-Cinnamon") << cinnamonIcon;
        QTest::newRow("mixed-desktop")
            << QStringLiteral("X-Cinnamon:GNOME") << cinnamonIcon;
        QTest::newRow("case-insensitive")
            << QStringLiteral("cinnamon") << cinnamonIcon;
        QTest::newRow("not-a-component")
            << QStringLiteral("NotCinnamon") << defaultIcon;
    }

    void selectsIconForDesktop()
    {
        QFETCH(QString, desktop);
        QFETCH(QString, expected);

        QCOMPARE(strikepro::applicationIconResource(desktop), expected);
    }

    void selectsDesktopFileForEnvironment()
    {
        QCOMPARE(
            strikepro::applicationDesktopFileName(QStringLiteral("GNOME")),
            QStringLiteral("io.github.kordax.MsiKeyboard"));
        QCOMPARE(
            strikepro::applicationDesktopFileName(QStringLiteral("X-Cinnamon")),
            QStringLiteral("io.github.kordax.MsiKeyboard.Cinnamon"));
    }

    void embedsBothIcons()
    {
        QVERIFY(QFile::exists(
            QStringLiteral(":/assets/icons/io.github.kordax.MsiKeyboard.svg")));
        QVERIFY(QFile::exists(QStringLiteral(
            ":/assets/icons/io.github.kordax.MsiKeyboard-cinnamon.svg")));
        QVERIFY(
            !QIcon(strikepro::applicationIconResource(QStringLiteral("GNOME")))
                 .isNull());
        QVERIFY(!QIcon(strikepro::applicationIconResource(
                           QStringLiteral("X-Cinnamon")))
                     .isNull());
    }
};

QTEST_MAIN(ApplicationIconTest)
#include "ApplicationIconTest.moc"
