#include "i18n/LanguageManager.h"

#include <QCoreApplication>
#include <QLocale>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

using strikepro::LanguageManager;

class LocalizationTest final : public QObject {
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QVERIFY(m_settingsDirectory.isValid());
        QCoreApplication::setOrganizationName(QStringLiteral("msi-keyboard-tests"));
        QCoreApplication::setApplicationName(QStringLiteral("localization"));
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(
            QSettings::IniFormat,
            QSettings::UserScope,
            m_settingsDirectory.path());
    }

    void init()
    {
        QSettings().clear();
    }

    void cleanup()
    {
        QLocale::setDefault(QLocale::c());
        QSettings().clear();
    }

    void englishIsDefaultRegardlessOfSystemLocale()
    {
        QLocale::setDefault(QLocale(QLocale::Russian));

        QCOMPARE(LanguageManager::storedLanguage(), QStringLiteral("en"));
        LanguageManager manager;
        QVERIFY(manager.setLanguage(LanguageManager::storedLanguage()));
        QCOMPARE(
            QCoreApplication::translate("strikepro::MainWindow", "Settings"),
            QStringLiteral("Settings"));
    }

    void russianCanBeSelected()
    {
        LanguageManager manager;
        QVERIFY(manager.setLanguage(QStringLiteral("ru")));

        QCOMPARE(manager.language(), QStringLiteral("ru"));
        QCOMPARE(
            QCoreApplication::translate("strikepro::MainWindow", "Settings"),
            QString::fromUtf8("Настройки"));
        QCOMPARE(
            QCoreApplication::translate(
                "CommandLine",
                "Run without the GUI."),
            QString::fromUtf8("Запустить без графического интерфейса."));
        QCOMPARE(
            QCoreApplication::translate(
                "msikeyboard::update",
                "Checking GitHub for a stable update..."),
            QString::fromUtf8("Проверка стабильного обновления на GitHub..."));
    }

    void explicitSelectionIsPersisted()
    {
        LanguageManager manager;
        QVERIFY(manager.setLanguage(QStringLiteral("ru"), true));

        QCOMPARE(LanguageManager::storedLanguage(), QStringLiteral("ru"));
    }

    void invalidStoredLanguageFallsBackToEnglish()
    {
        QSettings().setValue(QStringLiteral("ui/language"), QStringLiteral("fr"));

        QCOMPARE(LanguageManager::storedLanguage(), QStringLiteral("en"));
    }

    void missingRussianTranslationFallsBackToEnglish()
    {
        LanguageManager manager;
        QVERIFY(manager.setLanguage(QStringLiteral("ru")));

        QCOMPARE(
            QCoreApplication::translate(
                "strikepro::MainWindow",
                "Untranslated marker"),
            QStringLiteral("Untranslated marker"));
    }

private:
    QTemporaryDir m_settingsDirectory;
};

QTEST_GUILESS_MAIN(LocalizationTest)

#include "LocalizationTest.moc"
