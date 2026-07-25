#include "TrayIndicator.h"

#include <QAction>
#include <QApplication>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPolygonF>
#include <QSystemTrayIcon>
#include <QWidget>

#include <algorithm>
#include <utility>

namespace strikepro {
namespace {

QColor statusColor(const TrayIndicator::State &state)
{
    switch (state.connectionState) {
    case TrayIndicator::ConnectionState::Unavailable:
        return QColor(QStringLiteral("#737780"));
    case TrayIndicator::ConnectionState::Probing:
        return QColor(QStringLiteral("#e6b85c"));
    case TrayIndicator::ConnectionState::Connected:
        if (state.charging == true) {
            return QColor(QStringLiteral("#55e89b"));
        }
        if (state.batteryPercent.has_value() && *state.batteryPercent <= 15) {
            return QColor(QStringLiteral("#ef5f67"));
        }
        if (state.batteryPercent.has_value() && *state.batteryPercent <= 30) {
            return QColor(QStringLiteral("#e6b85c"));
        }
        return QColor(QStringLiteral("#55cfe8"));
    case TrayIndicator::ConnectionState::Problem:
        return QColor(QStringLiteral("#ef7777"));
    }
    return QColor(QStringLiteral("#737780"));
}

QPixmap renderTrayPixmap(const TrayIndicator::State &state, const int edge)
{
    QPixmap pixmap(edge, edge);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(edge / 64.0, edge / 64.0);

    const QRectF body(4.5, 13.5, 51.0, 37.0);
    const QRectF interior(8.5, 17.5, 43.0, 29.0);
    const QColor status = statusColor(state);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#15171b")));
    painter.drawRoundedRect(body, 7.0, 7.0);
    painter.drawRoundedRect(QRectF(55.0, 24.0, 6.0, 16.0), 2.0, 2.0);

    if (state.connectionState == TrayIndicator::ConnectionState::Connected
        && state.batteryPercent.has_value()) {
        const int percent = std::clamp(*state.batteryPercent, 0, 100);
        QPainterPath fillClip;
        fillClip.addRoundedRect(interior, 4.0, 4.0);
        painter.setClipPath(fillClip);
        painter.setBrush(status);
        painter.drawRect(QRectF(
            interior.left(),
            interior.top(),
            interior.width() * percent / 100.0,
            interior.height()));
        painter.setClipping(false);
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(
        QPen(status, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawRoundedRect(body, 7.0, 7.0);
    painter.drawLine(QPointF(56.0, 25.0), QPointF(59.0, 25.0));
    painter.drawLine(QPointF(59.5, 25.0), QPointF(59.5, 39.0));
    painter.drawLine(QPointF(59.0, 39.0), QPointF(56.0, 39.0));

    QRectF textRect(7.0, 14.0, 46.0, 35.0);
    if (state.connectionState == TrayIndicator::ConnectionState::Connected
        && state.charging == true && state.batteryPercent.has_value()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(QStringLiteral("#ffffff")));
        painter.drawPolygon(QPolygonF{
            QPointF(12.5, 22.0),
            QPointF(8.0, 31.0),
            QPointF(11.5, 31.0),
            QPointF(9.5, 39.5),
            QPointF(18.0, 28.5),
            QPointF(14.5, 28.5),
            QPointF(17.0, 22.0),
        });
        textRect = QRectF(16.0, 14.0, 37.0, 35.0);
    }

    QFont font = painter.font();
    font.setBold(true);
    const QString text = TrayIndicator::iconTextForState(state);
    font.setPixelSize(text.size() >= 4 ? 20 : 24);
    painter.setFont(font);
    painter.setPen(QColor(0, 0, 0, 180));
    painter.drawText(textRect.translated(1.0, 1.0), Qt::AlignCenter, text);
    painter.setPen(QColor(QStringLiteral("#ffffff")));
    painter.drawText(textRect, Qt::AlignCenter, text);

    return pixmap;
}

} // namespace

TrayIndicator::TrayIndicator(QWidget *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_menu(new QMenu(window))
{
    m_showAction = m_menu->addAction(QString());
    m_devicesMenu = m_menu->addMenu(QString());
    m_devicesMenu->setObjectName(QStringLiteral("trayDevicesMenu"));
    m_menu->addSeparator();
    m_quitAction = m_menu->addAction(QString());
    m_quitAction->setObjectName(QStringLiteral("trayQuitAction"));

    connect(m_showAction, &QAction::triggered, this, [this] { showWindow(); });
    connect(m_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(
        m_trayIcon,
        &QSystemTrayIcon::activated,
        this,
        [this](const QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger
                || reason == QSystemTrayIcon::DoubleClick) {
                showWindow();
            }
        });

    m_trayIcon->setContextMenu(m_menu);
    m_trayIcon->setIcon(iconForState(m_state));
    retranslateUi();
}

void TrayIndicator::setEnabled(const bool enabled)
{
    m_enabled = enabled;
    if (enabled) {
        m_trayIcon->show();
    } else {
        m_trayIcon->hide();
    }
}

bool TrayIndicator::isEnabled() const
{
    return m_enabled;
}

bool TrayIndicator::isAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayIndicator::setState(const State &state)
{
    m_state = state;
    if (m_state.batteryPercent.has_value()) {
        m_state.batteryPercent = std::clamp(*m_state.batteryPercent, 0, 100);
    }
    m_trayIcon->setIcon(iconForState(m_state));
    m_trayIcon->setToolTip(toolTipForState(m_state));
}

void TrayIndicator::setDevices(const QList<DeviceEntry> &devices)
{
    if (m_devices == devices) {
        return;
    }
    m_devices = devices;
    rebuildDeviceMenu();
}

void TrayIndicator::retranslateUi()
{
    m_showAction->setText(tr("Open MSI Keyboard"));
    m_devicesMenu->setTitle(tr("Devices"));
    m_quitAction->setText(tr("Quit"));
    m_trayIcon->setToolTip(toolTipForState(m_state));
    rebuildDeviceMenu();
}

QIcon TrayIndicator::iconForState(const State &state)
{
    QIcon icon;
    for (const int edge : {16, 22, 32, 64, 128}) {
        icon.addPixmap(renderTrayPixmap(state, edge));
    }
    return icon;
}

QString TrayIndicator::iconTextForState(const State &state)
{
    switch (state.connectionState) {
    case ConnectionState::Probing:
        return QStringLiteral("…");
    case ConnectionState::Problem:
        return QStringLiteral("!");
    case ConnectionState::Unavailable:
        return QStringLiteral("—");
    case ConnectionState::Connected:
        return state.batteryPercent.has_value()
                   ? QStringLiteral("%1%").arg(
                         std::clamp(*state.batteryPercent, 0, 100))
                   : QStringLiteral("—");
    }
    return QStringLiteral("—");
}

QString TrayIndicator::toolTipForState(const State &state)
{
    const QString name =
        state.deviceName.isEmpty() ? tr("MSI Keyboard") : state.deviceName;
    QString detail;
    switch (state.connectionState) {
    case ConnectionState::Unavailable:
        detail = tr("No supported keyboard detected");
        break;
    case ConnectionState::Probing:
        detail = tr("Checking connection");
        break;
    case ConnectionState::Problem:
        detail = tr("Connection problem");
        break;
    case ConnectionState::Connected:
        if (!state.batteryPercent.has_value()) {
            detail = tr("Connected");
        } else if (state.charging == true) {
            detail = tr("Battery: %1% · Charging").arg(*state.batteryPercent);
        } else if (state.charging == false) {
            detail = tr("Battery: %1% · On battery").arg(*state.batteryPercent);
        } else {
            detail = tr("Battery: %1%").arg(*state.batteryPercent);
        }
        break;
    }
    return QStringLiteral("%1\n%2").arg(name, detail);
}

void TrayIndicator::rebuildDeviceMenu()
{
    m_devicesMenu->clear();
    if (m_devices.isEmpty()) {
        QAction *empty =
            m_devicesMenu->addAction(tr("No supported keyboard detected"));
        empty->setEnabled(false);
        return;
    }

    for (const DeviceEntry &device : std::as_const(m_devices)) {
        const QString text =
            device.detail.isEmpty()
                ? device.name
                : QStringLiteral("%1 · %2").arg(device.name, device.detail);
        QAction *action = m_devicesMenu->addAction(text);
        action->setObjectName(QStringLiteral("trayDeviceAction"));
        action->setData(device.id);
        action->setCheckable(true);
        action->setChecked(device.selected);
        connect(action, &QAction::triggered, this, [this, id = device.id] {
            emit deviceSelected(id);
        });
    }
}

void TrayIndicator::showWindow()
{
    if (m_window == nullptr) {
        return;
    }
    if (m_window->isMinimized()) {
        m_window->showNormal();
    } else {
        m_window->show();
    }
    m_window->raise();
    m_window->activateWindow();
}

} // namespace strikepro
