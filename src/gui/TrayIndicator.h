#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include <optional>

class QAction;
class QIcon;
class QMenu;
class QSystemTrayIcon;
class QWidget;

namespace strikepro {

class TrayIndicator final : public QObject {
    Q_OBJECT

  public:
    enum class ConnectionState {
        Unavailable,
        Probing,
        Connected,
        Problem,
    };

    struct State {
        QString deviceName;
        ConnectionState connectionState = ConnectionState::Unavailable;
        std::optional<int> batteryPercent;
        std::optional<bool> charging;
    };

    struct DeviceEntry {
        QString id;
        QString name;
        QString detail;
        bool selected = false;

        bool operator==(const DeviceEntry &) const = default;
    };

    explicit TrayIndicator(QWidget *window, QObject *parent = nullptr);

    void setEnabled(bool enabled);
    [[nodiscard]] bool isEnabled() const;
    [[nodiscard]] bool isAvailable() const;
    void setState(const State &state);
    void setDevices(const QList<DeviceEntry> &devices);
    void retranslateUi();

    [[nodiscard]] static QIcon iconForState(const State &state);
    [[nodiscard]] static QString iconTextForState(const State &state);
    [[nodiscard]] static QString toolTipForState(const State &state);

  signals:
    void deviceSelected(const QString &deviceId);

  private:
    void rebuildDeviceMenu();
    void showWindow();

    QWidget *m_window = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_menu = nullptr;
    QMenu *m_devicesMenu = nullptr;
    QAction *m_showAction = nullptr;
    QAction *m_quitAction = nullptr;
    State m_state;
    QList<DeviceEntry> m_devices;
    bool m_enabled = false;
};

} // namespace strikepro
