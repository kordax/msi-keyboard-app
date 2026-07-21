#pragma once

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

    explicit TrayIndicator(QWidget *window, QObject *parent = nullptr);

    void setState(const State &state);
    void retranslateUi();

    [[nodiscard]] static QIcon iconForState(const State &state);
    [[nodiscard]] static QString toolTipForState(const State &state);

  private:
    void showWindow();

    QWidget *m_window = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_menu = nullptr;
    QAction *m_showAction = nullptr;
    QAction *m_quitAction = nullptr;
    State m_state;
};

} // namespace strikepro
