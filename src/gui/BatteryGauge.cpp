#include "BatteryGauge.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPropertyAnimation>

#include <algorithm>

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
    m_animation->setDuration(520);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

QColor BatteryGauge::colorForValue(qreal value)
{
    const qreal bounded = std::clamp(value, 0.0, 100.0);
    return QColor::fromHsvF(bounded / 300.0, 0.82, 0.93);
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
    const qreal previous = m_value.has_value() ? m_displayedValue : bounded;
    m_value = bounded;
    m_animation->stop();
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

    QPen trackPen(QColor(QStringLiteral("#2b303a")), 13.0, Qt::SolidLine, Qt::RoundCap);
    if (!m_value.has_value()) {
        trackPen.setStyle(Qt::CustomDashLine);
        trackPen.setDashPattern({1.0, 1.1});
    }
    painter.setPen(trackPen);
    painter.drawArc(arcRect, kStartAngle, kFullSpan);

    if (m_value.has_value()) {
        const QColor indicatorColor = colorForValue(m_displayedValue);
        QLinearGradient gradient(arcRect.topLeft(), arcRect.bottomRight());
        gradient.setColorAt(0.0, indicatorColor.lighter(112));
        gradient.setColorAt(0.55, indicatorColor);
        gradient.setColorAt(1.0, indicatorColor.darker(118));
        painter.setPen(QPen(QBrush(gradient), 13.0, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(
            arcRect,
            kStartAngle,
            static_cast<int>(kFullSpan * (m_displayedValue / 100.0)));
    }

    const QColor primary = m_deviceConnected ? QColor(QStringLiteral("#f7f8fb"))
                                             : QColor(QStringLiteral("#777e8d"));
    painter.setPen(primary);
    QFont valueFont = font();
    valueFont.setPixelSize(static_cast<int>(side * 0.22));
    valueFont.setWeight(QFont::DemiBold);
    painter.setFont(valueFont);
    const QString valueText =
        m_value.has_value() ? QStringLiteral("%1%").arg(qRound(m_displayedValue))
                            : QStringLiteral("—");
    painter.drawText(
        QRectF(0.0, center.y() - side * 0.16, width(), side * 0.25),
        Qt::AlignCenter,
        valueText);

    painter.setPen(QColor(QStringLiteral("#8d95a5")));
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
