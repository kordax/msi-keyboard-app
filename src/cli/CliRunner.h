#pragma once

#include "device/BatteryProtocol.h"
#include "device/HidMonitor.h"

#include <QHash>
#include <QJsonObject>
#include <QObject>

namespace strikepro {

struct CliOptions {
    bool json = false;
    bool once = false;
    bool battery = false;
    bool logs = false;
    QString profilePath;
};

class CliRunner final : public QObject {
    Q_OBJECT

  public:
    explicit CliRunner(CliOptions options, QObject *parent = nullptr);

  private:
    void handleInterfaces(const QList<strikepro::HidInterface> &interfaces);
    void handleReport(const strikepro::HidReport &report);
    void handleDiagnostic(const QString &message);
    void
    log(const QString &level,
        const QString &event,
        const QString &message,
        QJsonObject fields = {});
    void finishOnce(int exitCode);

    CliOptions m_options;
    HidMonitor m_monitor;
    QHash<QString, ProtocolProfile> m_profiles;
    bool m_batteryQueryStarted = false;
    bool m_finishScheduled = false;
};

} // namespace strikepro
