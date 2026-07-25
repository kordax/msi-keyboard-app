#pragma once

#include <QObject>
#include <QString>
#include <memory>

class QLocalServer;
class QLockFile;

namespace strikepro {

class SingleInstance final : public QObject {
    Q_OBJECT

  public:
    enum class Role {
        Primary,
        Secondary,
        Error,
    };

    explicit SingleInstance(QString serverName, QObject *parent = nullptr);
    ~SingleInstance() override;

    [[nodiscard]] Role start(QString *error = nullptr);

  signals:
    void activationRequested();

  private:
    [[nodiscard]] bool notifyPrimary(int timeoutMs = 750) const;
    void acceptConnections();

    QString m_serverName;
    std::unique_ptr<QLockFile> m_lockFile;
    QLocalServer *m_server = nullptr;
    bool m_primary = false;
};

} // namespace strikepro
