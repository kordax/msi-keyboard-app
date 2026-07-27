#pragma once

#include "DeviceDefinitions.h"

#include <chrono>

namespace strikepro {

enum class BatteryQueryBlockReason {
    None,
    TopologySettling,
    TransportDisabled,
    WiredUsbMayBePresent,
};

class BatteryQueryGuard final {
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    explicit BatteryQueryGuard(
        std::chrono::milliseconds settleInterval =
            std::chrono::milliseconds(1000),
        TimePoint now = Clock::now());

    void observeTopologyEvent(TimePoint now = Clock::now());

    [[nodiscard]] BatteryQueryBlockReason blockReason(
        const DeviceDefinition &definition,
        quint16 targetProductId,
        bool wiredUsbMayBePresent,
        TimePoint now = Clock::now()) const;

  private:
    std::chrono::milliseconds m_settleInterval;
    TimePoint m_resumeAfter;
};

} // namespace strikepro
