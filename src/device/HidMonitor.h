#pragma once

#include "HidTypes.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QTimer>

class QSocketNotifier;

namespace strikepro {

class HidMonitor final : public QObject {
    Q_OBJECT

  public:
    explicit HidMonitor(QObject *parent = nullptr);
    ~HidMonitor() override;

    [[nodiscard]] const QList<HidInterface> &interfaces() const
    {
        return m_interfaces;
    }
    [[nodiscard]] bool requestBattery(QString *error = nullptr);
    [[nodiscard]] bool
    requestBattery(const QString &devNode, QString *error = nullptr);

  public slots:
    void refresh();
    void takeReadOnlySnapshot();

  signals:
    void interfacesChanged(const QList<strikepro::HidInterface> &interfaces);
    void reportReceived(const strikepro::HidReport &report);
    void diagnosticMessage(const QString &message);
    void deviceEvent();

  private:
    void setupUeventMonitor();
    void closeUeventMonitor();
    void readUevents();
    void rebuildReaders();
    void closeReaders();
    void readAvailable(const QString &devNode);

    QTimer m_refreshTimer;
    QTimer m_eventRefreshTimer;
    QList<HidInterface> m_interfaces;
    QHash<QString, int> m_fds;
    QHash<QString, QSocketNotifier *> m_notifiers;
    int m_ueventFd = -1;
    QSocketNotifier *m_ueventNotifier = nullptr;
    bool m_hasRefreshed = false;
};

} // namespace strikepro
