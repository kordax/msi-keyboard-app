#pragma once

#include "device/BatteryDecoder.h"
#include "device/HidMonitor.h"

#include <QMainWindow>
#include <QTimer>
#include <optional>

class QLabel;

namespace strikepro {

class BatteryGauge;
class DebugWindow;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void updateInterfaces(const QList<strikepro::HidInterface> &interfaces);
    void recordReport(const strikepro::HidReport &report);
    void reloadProtocolProfile();
    void requestBattery();
    void showDebugLogs();
    void showDebugTelemetry();

private:
    void buildUi();
    void setBattery(const BatteryReading &reading);
    void setStatus(QLabel *dot, QLabel *detail, const QString &tone, const QString &text);
    void logDebug(const QString &message);
    [[nodiscard]] QString profilePath() const;

    HidMonitor *m_monitor = nullptr;
    DebugWindow *m_debugWindow = nullptr;
    std::optional<ProtocolProfile> m_profile;
    QList<HidInterface> m_interfaces;
    QTimer m_batteryPollTimer;
    quint64 m_batteryRequestGeneration = 0;
    quint16 m_activeProductId = 0;
    bool m_canQueryBattery = false;
    bool m_batteryRequestPending = false;

    QLabel *m_connectionBadge = nullptr;
    QLabel *m_deviceLabel = nullptr;
    QLabel *m_deviceImage = nullptr;
    QLabel *m_accessLabel = nullptr;
    QLabel *m_batteryValue = nullptr;
    QLabel *m_batteryState = nullptr;
    QLabel *m_deviceDot = nullptr;
    QLabel *m_deviceStatus = nullptr;
    QLabel *m_accessDot = nullptr;
    QLabel *m_accessStatus = nullptr;
    QLabel *m_protocolDot = nullptr;
    QLabel *m_protocolStatus = nullptr;
    BatteryGauge *m_batteryGauge = nullptr;
};

} // namespace strikepro
