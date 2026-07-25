#pragma once

#include "device/BatteryProtocol.h"
#include "device/DeviceCatalog.h"
#include "device/HidMonitor.h"

#include <QHash>
#include <QMainWindow>
#include <QStringList>
#include <QTimer>
#include <optional>

class QAction;
class QActionGroup;
class QBoxLayout;
class QCloseEvent;
class QEvent;
class QFrame;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QVBoxLayout;
class QWidget;

namespace strikepro {

class BatteryGauge;
class DebugWindow;
class LanguageManager;
class TrayIndicator;

class MainWindow final : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(
        LanguageManager *languageManager = nullptr, QWidget *parent = nullptr);

  protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

  private slots:
    void updateInterfaces(const QList<strikepro::HidInterface> &interfaces);
    void handleDeviceEvent();
    void recordReport(const strikepro::HidReport &report);
    void reloadProtocolProfile();
    void requestBattery();
    void selectDevice(QListWidgetItem *current, QListWidgetItem *previous);
    void selectDeviceFromTray(const QString &deviceId);
    void showDeviceArtwork(QListWidgetItem *item);
    void showDebugLogs();
    void showDebugTelemetry();

  private:
    enum class ConnectionState {
        Absent,
        Probing,
        Connected,
        AccessDenied,
        Unresponsive,
    };

    enum class UiDesign {
        Balanced,
        Compact,
        Showcase,
    };

    struct DeviceRuntime {
        SupportedDevice device;
        ConnectionState connectionState = ConnectionState::Probing;
        quint16 activeProductId = 0;
        quint16 failedProductId = 0;
        std::optional<BatteryReading> battery;
    };

    void buildUi();
    void retranslateUi();
    void rebuildDeviceList();
    void refreshConnectionUi();
    void refreshTrayIndicator();
    void clearBattery(DeviceRuntime &runtime);
    void handleBatteryQueryFailure(
        DeviceRuntime &runtime, quint16 failedProductId);
    void setConnectionState(DeviceRuntime &runtime, ConnectionState state);
    void setBattery(DeviceRuntime &runtime, const BatteryReading &reading);
    void setStatus(
        QLabel *dot, QLabel *detail, const QString &tone, const QString &text);
    void logDebug(const QString &message);
    void activateDevice(const QString &deviceId);
    void setKeepInTray(bool enabled, bool persist);
    void setDesign(UiDesign design, bool persist);
    void applyDesign();
    [[nodiscard]] QString profilePath() const;
    [[nodiscard]] const ProtocolProfile *
    profileForDevice(const SupportedDevice &device) const;
    [[nodiscard]] QString deviceName(const SupportedDevice &device) const;
    [[nodiscard]] QString transportName(const SupportedDevice &device) const;
    [[nodiscard]] QString
    transportName(const SupportedDevice &device, quint16 productId) const;
    [[nodiscard]] QString transportName(quint16 productId) const;
    [[nodiscard]] QString deviceListStatus(const DeviceRuntime &runtime) const;
    [[nodiscard]] DeviceRuntime *selectedDevice();
    [[nodiscard]] const DeviceRuntime *selectedDevice() const;

    LanguageManager *m_languageManager = nullptr;
    HidMonitor *m_monitor = nullptr;
    DebugWindow *m_debugWindow = nullptr;
    TrayIndicator *m_trayIndicator = nullptr;
    QHash<QString, ProtocolProfile> m_profiles;
    QHash<QString, DeviceRuntime> m_devices;
    QStringList m_deviceOrder;
    QString m_selectedDeviceId;
    QString m_pendingBatteryDeviceId;
    QTimer m_batteryPollTimer;
    quint64 m_batteryRequestGeneration = 0;
    bool m_batteryRequestPending = false;
    bool m_keepInTray = false;
    UiDesign m_design = UiDesign::Balanced;

    QMenu *m_settingsMenu = nullptr;
    QMenu *m_languageMenu = nullptr;
    QMenu *m_designMenu = nullptr;
    QMenu *m_debugMenu = nullptr;
    QActionGroup *m_languageActions = nullptr;
    QActionGroup *m_designActions = nullptr;
    QAction *m_englishAction = nullptr;
    QAction *m_russianAction = nullptr;
    QAction *m_balancedDesignAction = nullptr;
    QAction *m_compactDesignAction = nullptr;
    QAction *m_showcaseDesignAction = nullptr;
    QAction *m_keepInTrayAction = nullptr;
    QAction *m_quitAction = nullptr;
    QAction *m_logsAction = nullptr;
    QAction *m_telemetryAction = nullptr;
    QWidget *m_content = nullptr;
    QFrame *m_deviceListCard = nullptr;
    QFrame *m_detailCard = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QBoxLayout *m_dashboardLayout = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_subtitle = nullptr;
    QLabel *m_deviceListCaption = nullptr;
    QLabel *m_emptyDevices = nullptr;
    QListWidget *m_deviceList = nullptr;
    QWidget *m_batterySection = nullptr;
    QLabel *m_batteryCaption = nullptr;
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
