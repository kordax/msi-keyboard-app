#include "BatteryGauge.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPropertyAnimation>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace strikepro {
namespace {

constexpr int kStartAngle = 225 * 16;
constexpr int kFullSpan = -270 * 16;

} // namespace

BatteryGauge::BatteryGauge(QWidget *parent)
    : QWidget(parent)
    , m_animation(new QPropertyAnimation(this, "displayedValue", this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_animation->setDuration(1600);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
}

QColor BatteryGauge::colorForValue(qreal value)
{
    const qreal bounded = std::clamp(value, 0.0, 100.0);
    return QColor::fromHsvF(static_cast<float>(bounded / 300.0), 0.82F, 0.93F);
}

void BatteryGauge::setValue(std::optional<int> value)
{
    if (!value.has_value()) {
        m_animation->stop();
        m_value.reset();
        m_displayedValue = 0.0;
        update();
        return;
    }

    const int bounded = std::clamp(*value, 0, 100);
    const bool hadValue = m_value.has_value();
    const qreal previous = hadValue ? m_displayedValue : bounded;
    m_value = bounded;
    m_animation->stop();
    if (!hadValue || qAbs(previous - bounded) < 0.5) {
        m_displayedValue = bounded;
        update();
        return;
    }

    const int duration = std::clamp(
        1100 + static_cast<int>(qAbs(previous - bounded) * 12.0),
        1100,
        2300);
    m_animation->setDuration(duration);
    m_animation->setStartValue(previous);
    m_animation->setEndValue(bounded);
    m_animation->start();
}

void BatteryGauge::setDeviceConnected(bool connected)
{
    if (m_deviceConnected == connected) {
        return;
    }
    m_deviceConnected = connected;
    update();
}

QSize BatteryGauge::sizeHint() const
{
    return {250, 250};
}

QSize BatteryGauge::minimumSizeHint() const
{
    return {210, 210};
}

void BatteryGauge::setDisplayedValue(qreal value)
{
    m_displayedValue = value;
    update();
}

void BatteryGauge::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const qreal side = std::min(width(), height());
    const QPointF center(width() / 2.0, height() / 2.0);
    const QRectF arcRect(
        center.x() - side / 2.0 + 22.0,
        center.y() - side / 2.0 + 22.0,
        side - 44.0,
        side - 44.0);

    QPen trackPen(
        QColor(QStringLiteral("#303030")),
        13.0,
        Qt::SolidLine,
        Qt::RoundCap);
    painter.setPen(trackPen);
    painter.drawArc(arcRect, kStartAngle, kFullSpan);

    if (m_value.has_value()) {
        const QColor indicatorColor = colorForValue(m_displayedValue);
        const qreal progress = std::clamp(m_displayedValue / 100.0, 0.0, 1.0);
        const qreal totalSpan = kFullSpan * progress;

        QColor glowColor = indicatorColor;
        glowColor.setAlpha(38);
        painter.setPen(QPen(glowColor, 22.0, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(arcRect, kStartAngle, qRound(totalSpan));

        QColor tailColor = indicatorColor;
        tailColor.setAlpha(54);
        QColor middleColor = indicatorColor;
        middleColor.setAlpha(178);
        QLinearGradient indicatorGradient(
            arcRect.bottomLeft(),
            arcRect.bottomRight());
        indicatorGradient.setColorAt(0.0, tailColor);
        indicatorGradient.setColorAt(0.52, middleColor);
        indicatorGradient.setColorAt(1.0, indicatorColor);
        painter.setPen(
            QPen(QBrush(indicatorGradient), 13.0, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(arcRect, kStartAngle, qRound(totalSpan));

        const qreal radius = arcRect.width() / 2.0;
        const qreal headAngle =
            (kStartAngle + totalSpan) / 16.0 * std::numbers::pi / 180.0;
        const QPointF head(
            center.x() + radius * std::cos(headAngle),
            center.y() - radius * std::sin(headAngle));
        painter.setPen(Qt::NoPen);
        painter.setBrush(indicatorColor);
        painter.drawEllipse(head, 6.5, 6.5);
    }

    const QColor primary = m_deviceConnected
                               ? QColor(QStringLiteral("#f7f8fb"))
                               : QColor(QStringLiteral("#777e8d"));
    painter.setPen(primary);
    QFont valueFont = font();
    valueFont.setPixelSize(static_cast<int>(side * 0.22));
    valueFont.setWeight(QFont::DemiBold);
    painter.setFont(valueFont);
    const QString valueText =
        m_value.has_value()
            ? QStringLiteral("%1%").arg(qRound(m_displayedValue))
            : QStringLiteral("—");
    painter.drawText(
        QRectF(0.0, center.y() - side * 0.16, width(), side * 0.25),
        Qt::AlignCenter,
        valueText);

    painter.setPen(QColor(QStringLiteral("#969696")));
    QFont captionFont = font();
    captionFont.setPixelSize(std::max(10, static_cast<int>(side * 0.052)));
    captionFont.setWeight(QFont::DemiBold);
    captionFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.3);
    painter.setFont(captionFont);
    painter.drawText(
        QRectF(0.0, center.y() + side * 0.09, width(), side * 0.12),
        Qt::AlignHCenter | Qt::AlignTop,
        tr("BATTERY"));
}

} // namespace strikepro
