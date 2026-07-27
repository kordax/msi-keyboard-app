#include "BatteryQueryGuard.h"

namespace strikepro {

BatteryQueryGuard::BatteryQueryGuard(
    const std::chrono::milliseconds settleInterval, const TimePoint now)
    : m_settleInterval(settleInterval)
    , m_resumeAfter(now + settleInterval)
{}

void BatteryQueryGuard::observeTopologyEvent(const TimePoint now)
{
    m_resumeAfter = now + m_settleInterval;
}

BatteryQueryBlockReason BatteryQueryGuard::blockReason(
    const DeviceDefinition &definition,
    const quint16 targetProductId,
    const bool wiredUsbMayBePresent,
    const TimePoint now) const
{
    if (!definition.canQueryBatteryOver(targetProductId)) {
        return BatteryQueryBlockReason::TransportDisabled;
    }
    if (targetProductId == definition.dongleProductId && wiredUsbMayBePresent) {
        return BatteryQueryBlockReason::WiredUsbMayBePresent;
    }
    if (now < m_resumeAfter) {
        return BatteryQueryBlockReason::TopologySettling;
    }
    return BatteryQueryBlockReason::None;
}

} // namespace strikepro
