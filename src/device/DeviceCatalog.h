#pragma once

#include "HidTypes.h"

#include <QList>
#include <QString>

namespace strikepro {

struct SupportedDevice {
    QString id;
    QString name;
    quint16 productId = 0;
    QList<HidInterface> interfaces;

    [[nodiscard]] bool supportsBattery() const;
    [[nodiscard]] const HidInterface *batteryInterface() const;
    [[nodiscard]] bool canQueryBattery() const;
};

[[nodiscard]] QList<SupportedDevice>
groupSupportedDevices(const QList<HidInterface> &interfaces);

[[nodiscard]] QString retainedDeviceSelection(
    const QList<SupportedDevice> &devices, const QString &selectedDeviceId);

} // namespace strikepro
