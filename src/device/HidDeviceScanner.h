#pragma once

#include "HidTypes.h"

#include <QList>
#include <QString>

namespace strikepro {

class HidDeviceScanner {
  public:
    [[nodiscard]] static QList<HidInterface> scan();
    [[nodiscard]] static bool
    usbDeviceMayBePresent(quint16 vendorId, quint16 productId);
    [[nodiscard]] static bool usbDeviceMayBePresent(
        quint16 vendorId, quint16 productId, const QString &usbDevicesPath);
};

} // namespace strikepro
