#include "TrayIndicator.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPolygonF>
#include <QSystemTrayIcon>
#include <QWidget>

#include <algorithm>

namespace strikepro {
namespace {

QColor statusColor(const TrayIndicator::State &state)
{
    switch (state.connectionState) {
    case TrayIndicator::ConnectionState::Unavailable:
        return QColor(QStringLiteral("#707070"));
    case TrayIndicator::ConnectionState::Probing:
        return QColor(QStringLiteral("#e6b85c"));
    case TrayIndicator::ConnectionState::Connected:
        return QColor(QStringLiteral("#55e89b"));
    case TrayIndicator::ConnectionState::Problem:
        return QColor(QStringLiteral("#ef7777"));
    }
    return QColor(QStringLiteral("#707070"));
}

QPixmap renderTrayPixmap(const TrayIndicator::State &state, const int edge)
{
    QPixmap pixmap(edge, edge);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(edge / 64.0, edge / 64.0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#171717")));
    painter.drawEllipse(QRectF(2.0, 2.0, 60.0, 60.0));

    const QColor status = statusColor(state);
    const QColor outline =
        state.connectionState == TrayIndicator::ConnectionState::Problem
            ? status
            : QColor(QStringLiteral("#ededed"));
    painter.setBrush(Qt::NoBrush);
    painter.setPen(
        QPen(outline, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawRoundedRect(QRectF(11.5, 18.5, 37.0, 27.0), 5.0, 5.0);
    painter.drawLine(QPointF(49.0, 27.0), QPointF(53.0, 27.0));
    painter.drawLine(QPointF(53.0, 27.0), QPointF(53.0, 37.0));
    painter.drawLine(QPointF(53.0, 37.0), QPointF(49.0, 37.0));

    painter.setPen(Qt::NoPen);
    if (state.batteryPercent.has_value()) {
        const int percent = std::clamp(*state.batteryPercent, 0, 100);
        const QColor fill =
            percent <= 20 ? QColor(QStringLiteral("#e6b85c")) : status;
        QPainterPath fillClip;
        fillClip.addRoundedRect(QRectF(15.5, 22.5, 29.0, 19.0), 3.0, 3.0);
        painter.setClipPath(fillClip);
        painter.setBrush(fill);
        painter.drawRect(QRectF(15.5, 22.5, 29.0 * percent / 100.0, 19.0));
        painter.setClipping(false);

        if (state.charging == true) {
            painter.setBrush(QColor(QStringLiteral("#ffffff")));
            painter.drawPolygon(QPolygonF{
                QPointF(31.0, 21.0),
                QPointF(24.5, 33.0),
                QPointF(29.5, 33.0),
                QPointF(27.0, 43.0),
                QPointF(39.5, 29.5),
                QPointF(34.0, 29.5),
                QPointF(37.0, 21.0),
            });
        }
    } else {
        painter.setBrush(status);
        painter.drawEllipse(QRectF(25.0, 25.0, 14.0, 14.0));
    }

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
    m_menu->addSeparator();
    m_quitAction = m_menu->addAction(QString());

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
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon->show();
    }
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

void TrayIndicator::retranslateUi()
{
    m_showAction->setText(tr("Open MSI Keyboard"));
    m_quitAction->setText(tr("Quit"));
    m_trayIcon->setToolTip(toolTipForState(m_state));
}

QIcon TrayIndicator::iconForState(const State &state)
{
    QIcon icon;
    for (const int edge : {16, 22, 32, 64, 128}) {
        icon.addPixmap(renderTrayPixmap(state, edge));
    }
    return icon;
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
