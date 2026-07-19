#pragma once

#include "HidTypes.h"

#include <QList>

namespace strikepro {

class HidDeviceScanner {
  public:
    [[nodiscard]] static QList<HidInterface> scan();
};

} // namespace strikepro
