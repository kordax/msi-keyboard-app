#include "LanguageManager.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QSettings>
#include <QTranslator>

static void initializeTranslationResources()
{
    Q_INIT_RESOURCE(strikepro_i18n_translations);
}

namespace strikepro {
namespace {

constexpr auto kLanguageSetting = "ui/language";
constexpr auto kRussianCatalog = ":/i18n/msi-keyboard_ru.qm";

QString normalizedLanguage(const QString &language)
{
    return language.trimmed().toLower();
}

} // namespace

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
{
    initializeTranslationResources();
}

LanguageManager::~LanguageManager()
{
    if (m_translator != nullptr && QCoreApplication::instance() != nullptr) {
        QCoreApplication::removeTranslator(m_translator.get());
    }
    if (m_qtTranslator != nullptr && QCoreApplication::instance() != nullptr) {
        QCoreApplication::removeTranslator(m_qtTranslator.get());
    }
}

QString LanguageManager::defaultLanguage()
{
    return QStringLiteral("en");
}

QString LanguageManager::storedLanguage()
{
    const QString stored = normalizedLanguage(
        QSettings().value(kLanguageSetting, defaultLanguage()).toString());
    return isSupported(stored) ? stored : defaultLanguage();
}

bool LanguageManager::isSupported(const QString &language)
{
    const QString normalized = normalizedLanguage(language);
    return normalized == QStringLiteral("en")
        || normalized == QStringLiteral("ru");
}

QString LanguageManager::language() const
{
    return m_language;
}

bool LanguageManager::setLanguage(const QString &language, bool persist)
{
    const QString normalized = normalizedLanguage(language);
    if (!isSupported(normalized)) {
        return false;
    }

    if (normalized == m_language
        && (normalized == QStringLiteral("en") || m_translator != nullptr)) {
        if (persist) {
            QSettings().setValue(kLanguageSetting, normalized);
        }
        return true;
    }

    std::unique_ptr<QTranslator> nextTranslator;
    std::unique_ptr<QTranslator> nextQtTranslator;
    if (normalized == QStringLiteral("ru")) {
        auto translator = std::make_unique<QTranslator>();
        if (!translator->load(QString::fromLatin1(kRussianCatalog))) {
            return false;
        }
        nextTranslator = std::move(translator);

        auto qtTranslator = std::make_unique<QTranslator>();
        if (qtTranslator->load(
                QStringLiteral("qtbase_ru"),
                QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            nextQtTranslator = std::move(qtTranslator);
        }
    }

    m_language = normalized;
    if (m_translator != nullptr) {
        QCoreApplication::removeTranslator(m_translator.get());
    }
    if (m_qtTranslator != nullptr) {
        QCoreApplication::removeTranslator(m_qtTranslator.get());
    }
    m_qtTranslator = std::move(nextQtTranslator);
    m_translator = std::move(nextTranslator);
    if (m_qtTranslator != nullptr) {
        QCoreApplication::installTranslator(m_qtTranslator.get());
    }
    if (m_translator != nullptr) {
        QCoreApplication::installTranslator(m_translator.get());
    }
    if (persist) {
        QSettings().setValue(kLanguageSetting, normalized);
    }

    emit languageChanged(m_language);
    return true;
}

} // namespace strikepro
