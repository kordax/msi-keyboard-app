#include "DebugWindow.h"

#include "device/HidMonitor.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFontDatabase>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QVBoxLayout>

#include <algorithm>

namespace strikepro {
namespace {

QString reportSourceName(ReportSource source)
{
    switch (source) {
    case ReportSource::Input:
        return QStringLiteral("INPUT");
    case ReportSource::Feature:
        return QStringLiteral("FEATURE");
    case ReportSource::Output:
        return QStringLiteral("OUTPUT");
    }
    return QStringLiteral("UNKNOWN");
}

QString spacedHex(const QByteArray &data)
{
    return QString::fromLatin1(data.toHex(' '));
}

} // namespace

DebugWindow::DebugWindow(HidMonitor *monitor, QWidget *parent)
    : QDialog(parent)
{
    buildUi(monitor);

    if (monitor != nullptr) {
        connect(
            monitor,
            &HidMonitor::interfacesChanged,
            this,
            &DebugWindow::setInterfaces);
        connect(
            monitor,
            &HidMonitor::reportReceived,
            this,
            &DebugWindow::recordReport);
        connect(
            monitor,
            &HidMonitor::diagnosticMessage,
            this,
            &DebugWindow::recordMessage);
        setInterfaces(monitor->interfaces());
    } else {
        m_snapshotButton->setEnabled(false);
    }
}

void DebugWindow::buildUi(HidMonitor *monitor)
{
    setObjectName(QStringLiteral("debugWindow"));
    setWindowTitle(QStringLiteral("Debug · MSI Keyboard"));
    setAttribute(Qt::WA_DeleteOnClose, false);
    setMinimumSize(820, 520);
    resize(1040, 680);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    m_tabs = new QTabWidget(this);

    auto *logsTab = new QWidget(m_tabs);
    auto *logsLayout = new QVBoxLayout(logsTab);
    logsLayout->setContentsMargins(12, 14, 12, 12);
    logsLayout->setSpacing(10);

    auto *logsHeader = new QHBoxLayout;
    auto *logsTitle = new QLabel(QStringLiteral("Application logs"), logsTab);
    logsTitle->setObjectName(QStringLiteral("sectionTitle"));
    auto *clearLogsButton = new QPushButton(QStringLiteral("Очистить"), logsTab);
    clearLogsButton->setProperty("role", QStringLiteral("quiet"));
    logsHeader->addWidget(logsTitle);
    logsHeader->addStretch();
    logsHeader->addWidget(clearLogsButton);
    logsLayout->addLayout(logsHeader);

    m_logView = new QPlainTextEdit(logsTab);
    m_logView->setObjectName(QStringLiteral("logView"));
    m_logView->setReadOnly(true);
    m_logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_logView->document()->setMaximumBlockCount(500);
    logsLayout->addWidget(m_logView, 1);
    m_tabs->addTab(logsTab, QStringLiteral("Logs"));

    auto *telemetryTab = new QWidget(m_tabs);
    auto *telemetryLayout = new QVBoxLayout(telemetryTab);
    telemetryLayout->setContentsMargins(12, 14, 12, 12);
    telemetryLayout->setSpacing(10);

    auto *telemetryHeader = new QHBoxLayout;
    auto *telemetryTitles = new QVBoxLayout;
    telemetryTitles->setSpacing(1);
    auto *telemetryTitle = new QLabel(QStringLiteral("HID telemetry"), telemetryTab);
    telemetryTitle->setObjectName(QStringLiteral("sectionTitle"));
    auto *telemetrySubtitle = new QLabel(
        QStringLiteral("Сырые отчёты и инструменты исследования протокола"),
        telemetryTab);
    telemetrySubtitle->setProperty("role", QStringLiteral("muted"));
    telemetryTitles->addWidget(telemetryTitle);
    telemetryTitles->addWidget(telemetrySubtitle);
    telemetryHeader->addLayout(telemetryTitles);
    telemetryHeader->addStretch();

    m_reportCount = new QLabel(QStringLiteral("0 пакетов"), telemetryTab);
    m_reportCount->setProperty("role", QStringLiteral("counter"));
    m_snapshotButton = new QPushButton(QStringLiteral("Снять снимок"), telemetryTab);
    m_snapshotButton->setProperty("role", QStringLiteral("quiet"));
    auto *reloadButton =
        new QPushButton(QStringLiteral("Перечитать профиль"), telemetryTab);
    reloadButton->setProperty("role", QStringLiteral("quiet"));
    auto *clearButton = new QPushButton(QStringLiteral("Очистить"), telemetryTab);
    clearButton->setProperty("role", QStringLiteral("quiet"));
    auto *exportButton = new QPushButton(QStringLiteral("Экспорт JSON"), telemetryTab);
    exportButton->setProperty("role", QStringLiteral("quiet"));

    telemetryHeader->addWidget(m_reportCount);
    telemetryHeader->addWidget(m_snapshotButton);
    telemetryHeader->addWidget(reloadButton);
    telemetryHeader->addWidget(clearButton);
    telemetryHeader->addWidget(exportButton);
    telemetryLayout->addLayout(telemetryHeader);

    m_reportTable = new QTableWidget(0, 4, telemetryTab);
    m_reportTable->setObjectName(QStringLiteral("reportTable"));
    m_reportTable->setHorizontalHeaderLabels(
        {QStringLiteral("Время"),
         QStringLiteral("Источник"),
         QStringLiteral("Отчёт"),
         QStringLiteral("HEX-данные")});
    m_reportTable->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_reportTable->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    m_reportTable->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    m_reportTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_reportTable->verticalHeader()->hide();
    m_reportTable->setAlternatingRowColors(true);
    m_reportTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_reportTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_reportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_reportTable->setShowGrid(false);
    m_reportTable->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    telemetryLayout->addWidget(m_reportTable, 1);
    m_tabs->addTab(telemetryTab, QStringLiteral("Telemetry"));

    root->addWidget(m_tabs, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::hide);
    connect(clearLogsButton, &QPushButton::clicked, this, &DebugWindow::clearLogs);
    connect(clearButton, &QPushButton::clicked, this, &DebugWindow::clearTelemetry);
    connect(exportButton, &QPushButton::clicked, this, &DebugWindow::exportDiagnostics);
    connect(
        reloadButton,
        &QPushButton::clicked,
        this,
        &DebugWindow::protocolReloadRequested);
    if (monitor != nullptr) {
        connect(
            m_snapshotButton,
            &QPushButton::clicked,
            monitor,
            &HidMonitor::takeReadOnlySnapshot);
    }
}

void DebugWindow::showTab(Tab tab)
{
    m_tabs->setCurrentIndex(static_cast<int>(tab));
    show();
    raise();
    activateWindow();
}

void DebugWindow::setInterfaces(const QList<HidInterface> &interfaces)
{
    m_interfaces = interfaces;
    const bool hasReadableVendorInterface =
        std::ranges::any_of(interfaces, [](const HidInterface &interface) {
            return interface.readable
                && (interface.interfaceNumber == 1
                    || interface.interfaceNumber == 2
                    || interface.interfaceNumber == 4);
        });
    m_snapshotButton->setEnabled(hasReadableVendorInterface);
}

void DebugWindow::recordReport(const HidReport &report)
{
    if (!isVisible()
        || m_tabs->currentIndex() != static_cast<int>(Tab::Telemetry)) {
        return;
    }

    constexpr int kMaximumRows = 300;
    if (m_reportTable->rowCount() >= kMaximumRows) {
        m_reportTable->removeRow(0);
    }

    const int reportId =
        report.data.isEmpty() ? -1 : static_cast<quint8>(report.data.front());
    const QString description =
        report.requestedReportId >= 0
        ? QStringLiteral("%1:if%2  ·  req 0x%3 / resp 0x%4  ·  %5 B")
              .arg(report.productId, 4, 16, QLatin1Char('0'))
              .arg(report.interfaceNumber)
              .arg(report.requestedReportId, 2, 16, QLatin1Char('0'))
              .arg(reportId, 2, 16, QLatin1Char('0'))
              .arg(report.data.size())
        : QStringLiteral("%1:if%2  ·  id 0x%3  ·  %4 B")
              .arg(report.productId, 4, 16, QLatin1Char('0'))
              .arg(report.interfaceNumber)
              .arg(reportId, 2, 16, QLatin1Char('0'))
              .arg(report.data.size());

    const int row = m_reportTable->rowCount();
    m_reportTable->insertRow(row);
    m_reportTable->setItem(
        row,
        0,
        new QTableWidgetItem(
            QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"))));
    m_reportTable->setItem(
        row, 1, new QTableWidgetItem(reportSourceName(report.source)));
    m_reportTable->setItem(row, 2, new QTableWidgetItem(description));
    m_reportTable->setItem(row, 3, new QTableWidgetItem(spacedHex(report.data)));
    m_reportTable->scrollToBottom();

    QJsonObject json;
    json.insert(
        QStringLiteral("timestamp"),
        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json.insert(QStringLiteral("source"), reportSourceName(report.source));
    json.insert(QStringLiteral("interface"), report.interfaceNumber);
    json.insert(
        QStringLiteral("product_id"),
        QStringLiteral("%1").arg(report.productId, 4, 16, QLatin1Char('0')));
    json.insert(QStringLiteral("report_id"), reportId);
    if (report.requestedReportId >= 0) {
        json.insert(QStringLiteral("requested_report_id"), report.requestedReportId);
    }
    json.insert(QStringLiteral("data_hex"), QString::fromLatin1(report.data.toHex()));
    m_reportLog.append(json);
    while (m_reportLog.size() > 500) {
        m_reportLog.removeFirst();
    }
    m_reportCount->setText(
        QStringLiteral("%1 %2")
            .arg(m_reportLog.size())
            .arg(m_reportLog.size() == 1 ? QStringLiteral("пакет")
                                        : QStringLiteral("пакетов")));
}

void DebugWindow::recordMessage(const QString &message)
{
    m_logView->appendPlainText(
        QStringLiteral("%1  %2")
            .arg(
                QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")),
                message));
}

void DebugWindow::clearLogs()
{
    m_logView->clear();
}

void DebugWindow::clearTelemetry()
{
    m_reportTable->setRowCount(0);
    m_reportLog = {};
    m_reportCount->setText(QStringLiteral("0 пакетов"));
}

void DebugWindow::exportDiagnostics()
{
    const QString path = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Экспорт диагностики"),
        QDir::homePath() + QStringLiteral("/msi-keyboard-diagnostics.json"),
        QStringLiteral("JSON (*.json)"));
    if (path.isEmpty()) {
        return;
    }

    QJsonArray interfaces;
    for (const HidInterface &interface : m_interfaces) {
        QJsonObject json;
        json.insert(QStringLiteral("interface"), interface.interfaceNumber);
        json.insert(QStringLiteral("vendor_id"), QStringLiteral("0db0"));
        json.insert(
            QStringLiteral("product_id"),
            QStringLiteral("%1").arg(interface.productId, 4, 16, QLatin1Char('0')));
        json.insert(QStringLiteral("readable"), interface.readable);
        json.insert(QStringLiteral("writable"), interface.writable);
        json.insert(
            QStringLiteral("report_descriptor_hex"),
            QString::fromLatin1(interface.reportDescriptor.toHex()));
        interfaces.append(json);
    }

    QJsonObject root;
    root.insert(QStringLiteral("format_version"), 1);
    root.insert(
        QStringLiteral("exported_at"),
        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    root.insert(QStringLiteral("interfaces"), interfaces);
    root.insert(QStringLiteral("reports"), m_reportLog);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        recordMessage(QStringLiteral("Не удалось открыть файл экспорта"));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        recordMessage(QStringLiteral("Не удалось завершить экспорт"));
        return;
    }
    recordMessage(QStringLiteral("Диагностика экспортирована: %1").arg(path));
}

} // namespace strikepro
