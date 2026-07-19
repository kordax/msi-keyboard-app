#pragma once

#include <QObject>
#include <QString>

#include <memory>

class QTranslator;

namespace strikepro {

class LanguageManager final : public QObject {
    Q_OBJECT

public:
    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager() override;

    [[nodiscard]] static QString defaultLanguage();
    [[nodiscard]] static QString storedLanguage();
    [[nodiscard]] static bool isSupported(const QString &language);

    [[nodiscard]] QString language() const;
    bool setLanguage(const QString &language, bool persist = false);

signals:
    void languageChanged(const QString &language);

private:
    QString m_language = defaultLanguage();
    std::unique_ptr<QTranslator> m_qtTranslator;
    std::unique_ptr<QTranslator> m_translator;
};

} // namespace strikepro
