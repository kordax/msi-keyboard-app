#pragma once

#include "device/BatteryDecoder.h"
#include "device/HidMonitor.h"

#include <QMainWindow>
#include <QTimer>
#include <optional>

class QLabel;
class QAction;
class QActionGroup;
class QEvent;
class QMenu;

namespace strikepro {

class BatteryGauge;
class DebugWindow;
class LanguageManager;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(
        LanguageManager *languageManager = nullptr,
        QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void updateInterfaces(const QList<strikepro::HidInterface> &interfaces);
    void recordReport(const strikepro::HidReport &report);
    void reloadProtocolProfile();
    void requestBattery();
    void showDebugLogs();
    void showDebugTelemetry();

private:
    void buildUi();
    void retranslateUi();
    void setBattery(const BatteryReading &reading);
    void setStatus(QLabel *dot, QLabel *detail, const QString &tone, const QString &text);
    void logDebug(const QString &message);
    [[nodiscard]] QString profilePath() const;

    LanguageManager *m_languageManager = nullptr;
    HidMonitor *m_monitor = nullptr;
    DebugWindow *m_debugWindow = nullptr;
    std::optional<ProtocolProfile> m_profile;
    QList<HidInterface> m_interfaces;
    QTimer m_batteryPollTimer;
    quint64 m_batteryRequestGeneration = 0;
    quint16 m_activeProductId = 0;
    bool m_canQueryBattery = false;
    bool m_batteryRequestPending = false;
    std::optional<BatteryReading> m_lastBatteryReading;

    QMenu *m_settingsMenu = nullptr;
    QMenu *m_languageMenu = nullptr;
    QMenu *m_debugMenu = nullptr;
    QActionGroup *m_languageActions = nullptr;
    QAction *m_englishAction = nullptr;
    QAction *m_russianAction = nullptr;
    QAction *m_logsAction = nullptr;
    QAction *m_telemetryAction = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_connectionBadge = nullptr;
    QLabel *m_batteryCaption = nullptr;
    QLabel *m_modeValue = nullptr;
    QLabel *m_modeCaption = nullptr;
    QLabel *m_deviceCaption = nullptr;
    QLabel *m_deviceLabel = nullptr;
    QLabel *m_deviceImage = nullptr;
    QLabel *m_batteryValue = nullptr;
    QLabel *m_batteryState = nullptr;
    QLabel *m_deviceStatusTitle = nullptr;
    QLabel *m_deviceDot = nullptr;
    QLabel *m_deviceStatus = nullptr;
    BatteryGauge *m_batteryGauge = nullptr;
};

} // namespace strikepro
