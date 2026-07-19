#pragma once

#include <QColor>
#include <QWidget>
#include <optional>

class QPropertyAnimation;

namespace strikepro {

class BatteryGauge final : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal displayedValue READ displayedValue WRITE setDisplayedValue)

public:
    explicit BatteryGauge(QWidget *parent = nullptr);

    void setValue(std::optional<int> value);
    void setDeviceConnected(bool connected);

    [[nodiscard]] static QColor colorForValue(qreal value);
    [[nodiscard]] std::optional<int> value() const { return m_value; }
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    [[nodiscard]] qreal displayedValue() const { return m_displayedValue; }
    void setDisplayedValue(qreal value);

    std::optional<int> m_value;
    qreal m_displayedValue = 0.0;
    bool m_deviceConnected = false;
    QPropertyAnimation *m_animation = nullptr;
};

} // namespace strikepro
