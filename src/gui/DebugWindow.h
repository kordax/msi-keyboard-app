#pragma once

#include "device/HidTypes.h"

#include <QDialog>
#include <QJsonArray>
#include <QList>

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTableWidget;
class QTabWidget;

namespace strikepro {

class HidMonitor;

class DebugWindow final : public QDialog {
    Q_OBJECT

public:
    enum class Tab {
        Logs,
        Telemetry,
    };

    explicit DebugWindow(HidMonitor *monitor, QWidget *parent = nullptr);

    void showTab(Tab tab);

public slots:
    void setInterfaces(const QList<strikepro::HidInterface> &interfaces);
    void recordReport(const strikepro::HidReport &report);
    void recordMessage(const QString &message);

signals:
    void protocolReloadRequested();

private slots:
    void clearLogs();
    void clearTelemetry();
    void exportDiagnostics();

private:
    void buildUi(HidMonitor *monitor);

    QList<HidInterface> m_interfaces;
    QJsonArray m_reportLog;
    QTabWidget *m_tabs = nullptr;
    QPlainTextEdit *m_logView = nullptr;
    QTableWidget *m_reportTable = nullptr;
    QLabel *m_reportCount = nullptr;
    QPushButton *m_snapshotButton = nullptr;
};

} // namespace strikepro
