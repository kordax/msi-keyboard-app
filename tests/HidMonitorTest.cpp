#include "device/HidMonitor.h"

#include <QtTest>

using strikepro::HidInterface;
using strikepro::HidMonitor;

class HidMonitorTest final : public QObject {
    Q_OBJECT

  private slots:
    void publishesInitialScan()
    {
        HidMonitor monitor;
        int scanCount = 0;
        connect(
            &monitor,
            &HidMonitor::interfacesChanged,
            this,
            [&scanCount](const QList<HidInterface> &) { ++scanCount; });

        QTRY_COMPARE_WITH_TIMEOUT(scanCount, 1, 1000);
    }
};

QTEST_GUILESS_MAIN(HidMonitorTest)

#include "HidMonitorTest.moc"
