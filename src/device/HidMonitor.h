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

    [[nodiscard]] const QList<HidInterface> &interfaces() const { return m_interfaces; }
    [[nodiscard]] bool requestBattery(QString *error = nullptr);

public slots:
    void refresh();
    void takeReadOnlySnapshot();

signals:
    void interfacesChanged(const QList<strikepro::HidInterface> &interfaces);
    void reportReceived(const strikepro::HidReport &report);
    void diagnosticMessage(const QString &message);

private:
    void rebuildReaders();
    void closeReaders();
    void readAvailable(const QString &devNode);

    QTimer m_refreshTimer;
    QList<HidInterface> m_interfaces;
    QHash<QString, int> m_fds;
    QHash<QString, QSocketNotifier *> m_notifiers;
    bool m_hasRefreshed = false;
};

} // namespace strikepro
