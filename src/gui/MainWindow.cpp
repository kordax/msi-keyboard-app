#include "MainWindow.h"

#include "BatteryGauge.h"
#include "DebugWindow.h"
#include "device/AppPaths.h"
#include "i18n/LanguageManager.h"

#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QStyle>
#include <QVBoxLayout>

#include <algorithm>

namespace strikepro {
namespace {

constexpr int kBatteryPollIntervalMs = 15'000;
constexpr int kBatteryResponseTimeoutMs = 2'500;

void refreshStyle(QWidget *widget)
{
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

QFrame *makeCard(QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("card"));
    card->setAttribute(Qt::WA_StyledBackground);
    return card;
}

QWidget *makeMetaTile(
    const QString &value,
    const QString &caption,
    QLabel **valueOutput,
    QLabel **captionOutput,
    QWidget *parent)
{
    auto *tile = new QWidget(parent);
    tile->setProperty("role", QStringLiteral("metaTile"));
    tile->setAttribute(Qt::WA_StyledBackground);
    auto *layout = new QVBoxLayout(tile);
    layout->setContentsMargins(13, 10, 13, 10);
    layout->setSpacing(2);

    auto *valueLabel = new QLabel(value, tile);
    valueLabel->setProperty("role", QStringLiteral("metaValue"));
    auto *captionLabel = new QLabel(caption, tile);
    captionLabel->setProperty("role", QStringLiteral("muted"));
    if (valueOutput != nullptr) {
        *valueOutput = valueLabel;
    }
    if (captionOutput != nullptr) {
        *captionOutput = captionLabel;
    }
    layout->addWidget(valueLabel);
    layout->addWidget(captionLabel);
    return tile;
}

QWidget *makeStatusRow(
    const QString &title,
    QLabel **titleOutput,
    QLabel **dot,
    QLabel **detail,
    QWidget *parent)
{
    auto *row = new QWidget(parent);
    row->setMinimumHeight(50);
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 4, 0, 4);
    layout->setSpacing(12);

    *dot = new QLabel(QStringLiteral("●"), row);
    (*dot)->setProperty("role", QStringLiteral("statusDot"));
    (*dot)->setProperty("tone", QStringLiteral("off"));
    (*dot)->setFixedWidth(14);

    auto *labels = new QVBoxLayout;
    labels->setSpacing(2);
    auto *titleLabel = new QLabel(title, row);
    titleLabel->setProperty("role", QStringLiteral("statusTitle"));
    if (titleOutput != nullptr) {
        *titleOutput = titleLabel;
    }
    *detail = new QLabel(row);
    (*detail)->setProperty("role", QStringLiteral("muted"));
    (*detail)->setWordWrap(false);
    labels->addWidget(titleLabel);
    labels->addWidget(*detail);

    layout->addWidget(*dot, 0, Qt::AlignTop);
    layout->addLayout(labels, 1);
    return row;
}

const HidInterface *
preferredBatteryInterface(const QList<HidInterface> &interfaces)
{
    for (const quint16 productId :
         {kStrikeProWiredProductId, kStrikeProWirelessProductId}) {
        const auto found = std::ranges::find_if(
            interfaces,
            [productId](const HidInterface &interface) {
                return interface.productId == productId
                       && interface.interfaceNumber == 1;
            });
        if (found != interfaces.end()) {
            return &*found;
        }
    }
    return nullptr;
}

} // namespace

MainWindow::MainWindow(LanguageManager *languageManager, QWidget *parent)
    : QMainWindow(parent)
    , m_languageManager(languageManager)
    , m_monitor(new HidMonitor(this))
{
    buildUi();

    m_debugWindow = new DebugWindow(m_monitor, this);
    m_debugWindow->setStyleSheet(styleSheet());

    connect(
        m_monitor,
        &HidMonitor::interfacesChanged,
        this,
        &MainWindow::updateInterfaces);
    connect(
        m_monitor,
        &HidMonitor::reportReceived,
        this,
        &MainWindow::recordReport);
    connect(
        m_debugWindow,
        &DebugWindow::protocolReloadRequested,
        this,
        &MainWindow::reloadProtocolProfile);

    m_batteryPollTimer.setInterval(kBatteryPollIntervalMs);
    connect(
        &m_batteryPollTimer,
        &QTimer::timeout,
        this,
        &MainWindow::requestBattery);
    m_batteryPollTimer.start();

    reloadProtocolProfile();
    m_monitor->refresh();
}

void MainWindow::buildUi()
{
    setMinimumSize(920, 600);
    resize(1080, 680);

    menuBar()->setNativeMenuBar(false);
    m_settingsMenu = menuBar()->addMenu(QString());
    m_languageMenu = m_settingsMenu->addMenu(QString());
    m_languageActions = new QActionGroup(this);
    m_languageActions->setExclusive(true);
    m_englishAction = m_languageMenu->addAction(QString());
    m_englishAction->setCheckable(true);
    m_languageActions->addAction(m_englishAction);
    m_russianAction = m_languageMenu->addAction(QString());
    m_russianAction->setCheckable(true);
    m_languageActions->addAction(m_russianAction);
    connect(m_englishAction, &QAction::triggered, this, [this] {
        if (m_languageManager != nullptr) {
            m_languageManager->setLanguage(QStringLiteral("en"), true);
        }
    });
    connect(m_russianAction, &QAction::triggered, this, [this] {
        if (m_languageManager != nullptr) {
            m_languageManager->setLanguage(QStringLiteral("ru"), true);
        }
    });
    m_languageMenu->setEnabled(m_languageManager != nullptr);

    m_debugMenu = menuBar()->addMenu(QString());
    m_logsAction = m_debugMenu->addAction(QString());
    m_telemetryAction = m_debugMenu->addAction(QString());
    connect(
        m_logsAction,
        &QAction::triggered,
        this,
        &MainWindow::showDebugLogs);
    connect(
        m_telemetryAction,
        &QAction::triggered,
        this,
        &MainWindow::showDebugTelemetry);

    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("surface"));
    central->setAttribute(Qt::WA_StyledBackground);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(30, 22, 30, 28);
    root->setSpacing(20);

    auto *header = new QHBoxLayout;
    header->setSpacing(14);

    auto *titleColumn = new QVBoxLayout;
    titleColumn->setSpacing(1);
    m_title = new QLabel(central);
    m_title->setObjectName(QStringLiteral("title"));
    titleColumn->addWidget(m_title);
    header->addLayout(titleColumn);
    header->addStretch();

    m_connectionBadge = new QLabel(central);
    m_connectionBadge->setObjectName(QStringLiteral("connectionBadge"));
    m_connectionBadge->setProperty("tone", QStringLiteral("off"));
    header->addWidget(m_connectionBadge, 0, Qt::AlignVCenter);
    root->addLayout(header);

    auto *dashboard = new QHBoxLayout;
    dashboard->setSpacing(18);

    auto *batteryCard = makeCard(central);
    batteryCard->setMinimumHeight(390);
    auto *batteryLayout = new QHBoxLayout(batteryCard);
    batteryLayout->setContentsMargins(25, 24, 25, 24);
    batteryLayout->setSpacing(24);

    m_batteryGauge = new BatteryGauge(batteryCard);
    batteryLayout->addWidget(m_batteryGauge, 0, Qt::AlignCenter);

    auto *batteryDetails = new QVBoxLayout;
    batteryDetails->setSpacing(8);
    m_batteryCaption = new QLabel(batteryCard);
    m_batteryCaption->setProperty("role", QStringLiteral("eyebrow"));
    m_batteryValue = new QLabel(batteryCard);
    m_batteryValue->setObjectName(QStringLiteral("batteryHeadline"));
    m_batteryValue->setWordWrap(true);
    m_batteryState = new QLabel(batteryCard);
    m_batteryState->setProperty("role", QStringLiteral("muted"));
    m_batteryState->setWordWrap(true);
    batteryDetails->addWidget(m_batteryCaption);
    batteryDetails->addWidget(m_batteryValue);
    batteryDetails->addWidget(m_batteryState);
    batteryDetails->addStretch();

    auto *meta = new QHBoxLayout;
    meta->setSpacing(8);
    meta->addWidget(makeMetaTile(
        QString(),
        QString(),
        &m_modeValue,
        &m_modeCaption,
        batteryCard));
    batteryDetails->addLayout(meta);
    batteryLayout->addLayout(batteryDetails, 1);
    dashboard->addWidget(batteryCard, 3);

    auto *deviceCard = makeCard(central);
    deviceCard->setMinimumWidth(390);
    auto *deviceLayout = new QVBoxLayout(deviceCard);
    deviceLayout->setContentsMargins(23, 21, 23, 21);
    deviceLayout->setSpacing(9);

    m_deviceCaption = new QLabel(deviceCard);
    m_deviceCaption->setProperty("role", QStringLiteral("eyebrow"));
    m_deviceLabel = new QLabel(deviceCard);
    m_deviceLabel->setObjectName(QStringLiteral("deviceName"));
    m_deviceLabel->setWordWrap(true);
    deviceLayout->addWidget(m_deviceCaption);
    deviceLayout->addWidget(m_deviceLabel);

    m_deviceImage = new QLabel(deviceCard);
    m_deviceImage->setObjectName(QStringLiteral("deviceArtwork"));
    m_deviceImage->setAlignment(Qt::AlignCenter);
    m_deviceImage->setMinimumHeight(175);
    m_deviceImage->setVisible(false);
    QPixmap artwork(QStringLiteral(":/assets/keyboard/strike_pro.webp"));
    if (artwork.isNull()) {
        artwork.load(QStringLiteral(":/assets/keyboard/strike_pro.png"));
    }
    if (!artwork.isNull()) {
        m_deviceImage->setPixmap(artwork.scaled(
            360,
            230,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
    } else {
        m_deviceImage->setText(tr("MSI STRIKE PRO"));
    }
    deviceLayout->addWidget(m_deviceImage);

    deviceLayout->addWidget(makeStatusRow(
        QString(),
        &m_deviceStatusTitle,
        &m_deviceDot,
        &m_deviceStatus,
        deviceCard));
    deviceLayout->addStretch();

    dashboard->addWidget(deviceCard, 2);
    root->addLayout(dashboard, 1);
    setCentralWidget(central);

    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QDialog#debugWindow { background: #0c0e12; }
        QWidget {
            color: #f0f2f6;
            font-family: "Inter", "Noto Sans", sans-serif;
            font-size: 13px;
        }
        QWidget#surface { background: #0f1116; }
        QMenuBar {
            background: #0f1116;
            color: #aeb5c2;
            border-bottom: 1px solid #232832;
            padding: 3px 8px;
        }
        QMenuBar::item {
            background: transparent;
            padding: 6px 10px;
        }
        QMenuBar::item:selected { background: #242933; color: #ffffff; }
        QMenu {
            background: #1a1e25;
            color: #dce0e8;
            border: 1px solid #303642;
            padding: 5px;
        }
        QMenu::item { padding: 7px 26px 7px 10px; }
        QMenu::item:selected { background: #292f3a; color: #ffffff; }
        QLabel#title {
            color: #ffffff;
            font-size: 25px;
            font-weight: 750;
            letter-spacing: 1.6px;
        }
        QLabel#connectionBadge {
            border-radius: 12px;
            padding: 6px 11px;
            font-size: 10px;
            font-weight: 750;
            letter-spacing: 0.7px;
        }
        QLabel#connectionBadge[tone="off"] {
            background: #222630;
            color: #858c9a;
        }
        QLabel#connectionBadge[tone="ok"] {
            background: #123525;
            color: #73e2a8;
        }
        QFrame#card {
            background: #171a21;
            border: 1px solid #292e38;
            border-radius: 14px;
        }
        QLabel[role="eyebrow"] {
            color: #7f8796;
            font-size: 10px;
            font-weight: 750;
            letter-spacing: 1.4px;
        }
        QLabel[role="muted"] { color: #9299a8; }
        QLabel#batteryHeadline {
            color: #ffffff;
            font-size: 24px;
            font-weight: 700;
        }
        QLabel#deviceName {
            color: #ffffff;
            font-size: 17px;
            font-weight: 700;
        }
        QLabel#deviceArtwork { background: transparent; }
        QLabel#sectionTitle {
            color: #ffffff;
            font-size: 16px;
            font-weight: 700;
        }
        QWidget[role="metaTile"] {
            background: #20242c;
            border: 1px solid #2d323d;
            border-radius: 9px;
        }
        QLabel[role="metaValue"] {
            color: #f2f4f8;
            font-size: 12px;
            font-weight: 700;
        }
        QLabel[role="statusTitle"] {
            color: #dfe3ea;
            font-size: 12px;
            font-weight: 650;
        }
        QLabel[role="statusDot"] { font-size: 10px; }
        QLabel[role="statusDot"][tone="off"] { color: #4d5360; }
        QLabel[role="statusDot"][tone="ok"] { color: #57d696; }
        QLabel[role="statusDot"][tone="warn"] { color: #ffb454; }
        QLabel[role="counter"] {
            background: #222630;
            color: #aeb5c2;
            border-radius: 9px;
            padding: 4px 9px;
            font-size: 10px;
        }
        QPushButton {
            min-height: 18px;
            padding: 8px 13px;
            border-radius: 8px;
            font-weight: 600;
        }
        QPushButton[role="quiet"] {
            background: #22262f;
            color: #cdd2dc;
            border: 1px solid #343a46;
        }
        QPushButton[role="quiet"]:hover { background: #2b303b; }
        QPushButton:disabled {
            background: #20232a;
            color: #555c69;
            border-color: #292d35;
        }
        QTabWidget::pane {
            background: #15181e;
            border: 1px solid #2b303a;
            border-radius: 9px;
        }
        QTabBar::tab {
            background: #171a21;
            color: #8e96a5;
            border: 1px solid #2b303a;
            padding: 8px 16px;
        }
        QTabBar::tab:selected { background: #242933; color: #ffffff; }
        QPlainTextEdit#logView,
        QTableWidget#reportTable {
            background: #11141a;
            alternate-background-color: #15181e;
            color: #c7ccd6;
            border: 1px solid #292e38;
            border-radius: 9px;
            selection-background-color: #3a1b25;
            selection-color: #ffffff;
        }
        QHeaderView::section {
            background: #1c2027;
            color: #7f8796;
            border: none;
            border-bottom: 1px solid #2a2f39;
            padding: 8px;
            font-size: 10px;
            font-weight: 700;
        }
        QTableCornerButton::section {
            background: #1c2027;
            border: none;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            margin: 3px;
        }
        QScrollBar::handle:vertical {
            background: #353b47;
            border-radius: 3px;
            min-height: 24px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QToolTip {
            background: #252a33;
            color: #f4f5f7;
            border: 1px solid #3a414e;
            padding: 5px;
        }
    )"));
    retranslateUi();
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::LanguageChange && m_title != nullptr) {
        retranslateUi();
    }
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("MSI Keyboard"));
    m_settingsMenu->setTitle(tr("Settings"));
    m_languageMenu->setTitle(tr("Language"));
    m_englishAction->setText(tr("English"));
    m_russianAction->setText(tr("Russian"));
    m_debugMenu->setTitle(tr("Debug"));
    m_logsAction->setText(tr("Logs"));
    m_telemetryAction->setText(tr("Telemetry"));

    const QString language = m_languageManager == nullptr
                                 ? LanguageManager::defaultLanguage()
                                 : m_languageManager->language();
    m_englishAction->setChecked(language == QStringLiteral("en"));
    m_russianAction->setChecked(language == QStringLiteral("ru"));

    m_title->setText(tr("MSI Keyboard manager for Linux"));
    m_batteryCaption->setText(tr("BATTERY"));
    m_modeValue->setText(tr("READ ONLY"));
    m_modeCaption->setText(tr("mode"));
    m_deviceCaption->setText(tr("DEVICE"));
    m_deviceStatusTitle->setText(tr("Connection"));

    updateInterfaces(m_interfaces);
    if (!m_interfaces.isEmpty() && m_lastBatteryReading.has_value()) {
        setBattery(*m_lastBatteryReading);
    }
    m_batteryGauge->update();
}

QString MainWindow::profilePath() const
{
    return defaultProtocolProfilePath();
}

void MainWindow::setStatus(
    QLabel *dot, QLabel *detail, const QString &tone, const QString &text)
{
    dot->setProperty("tone", tone);
    detail->setText(text);
    refreshStyle(dot);
}

void MainWindow::logDebug(const QString &message)
{
    if (m_debugWindow != nullptr) {
        m_debugWindow->recordMessage(message);
    }
}

void MainWindow::reloadProtocolProfile()
{
    QString error;
    m_profile = BatteryDecoder::loadProfile(profilePath(), &error);
    if (m_profile.has_value() && m_profile->canDecodePercentage()) {
        logDebug(tr("Protocol profile loaded: %1").arg(m_profile->path));
        QTimer::singleShot(0, this, &MainWindow::requestBattery);
    } else {
        m_profile.reset();
        logDebug(
            error.isEmpty() ? tr("Battery profile unavailable")
                            : tr("Profile error: %1").arg(error));
    }
}

void MainWindow::updateInterfaces(const QList<HidInterface> &interfaces)
{
    m_interfaces = interfaces;
    const bool connected = !interfaces.isEmpty();
    const HidInterface *batteryInterface =
        preferredBatteryInterface(interfaces);
    const quint16 activeProductId =
        batteryInterface == nullptr ? 0 : batteryInterface->productId;
    const bool transportChanged = activeProductId != m_activeProductId;
    m_activeProductId = activeProductId;
    m_canQueryBattery = batteryInterface != nullptr
                        && batteryInterface->readable
                        && batteryInterface->writable;

    m_batteryGauge->setDeviceConnected(connected);
    m_deviceImage->setVisible(connected);

    m_connectionBadge->setText(
        connected ? tr("CONNECTED") : tr("NOT CONNECTED"));
    m_connectionBadge->setProperty(
        "tone",
        connected ? QStringLiteral("ok") : QStringLiteral("off"));
    refreshStyle(m_connectionBadge);

    if (!connected) {
        if (transportChanged) {
            logDebug(tr("MSI Strike Pro disconnected"));
        }
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_batteryGauge->setValue(std::nullopt);
        m_batteryValue->setText(tr("No device"));
        m_batteryState->setText(
            tr("Waiting for a USB receiver or wired connection."));
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("off"),
            tr("Keyboard not detected"));
        return;
    }

    const bool wired = activeProductId == kStrikeProWiredProductId;
    m_deviceLabel->setText(
        wired ? tr("MSI Strike Pro · USB") : tr("MSI Strike Pro · 2.4 GHz"));
    setStatus(
        m_deviceDot,
        m_deviceStatus,
        QStringLiteral("ok"),
        wired ? tr("Wired connection") : tr("Wireless receiver"));

    if (transportChanged) {
        logDebug(
            wired ? tr("MSI Strike Pro detected over USB")
                  : tr("MSI Strike Pro detected through the 2.4 GHz receiver"));
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_batteryGauge->setValue(std::nullopt);
        m_batteryValue->setText(tr("Reading battery…"));
        m_batteryState->setText(tr("The first query runs automatically."));
    }

    if (m_canQueryBattery) {
        QTimer::singleShot(0, this, &MainWindow::requestBattery);
    } else {
        if (!m_batteryGauge->value().has_value()) {
            m_batteryValue->setText(tr("HID access required"));
            m_batteryState->setText(tr(
                "The device was found, but the system denied battery access."));
        }
    }
}

void MainWindow::requestBattery()
{
    if (!m_canQueryBattery || m_batteryRequestPending || !m_profile.has_value()
        || !m_profile->canDecodePercentage()) {
        return;
    }

    QString error;
    if (!m_monitor->requestBattery(&error)) {
        logDebug(tr("Battery query: %1").arg(error));
        if (!m_batteryGauge->value().has_value()) {
            m_batteryValue->setText(tr("No response"));
            m_batteryState->setText(error);
        }
        return;
    }

    m_batteryRequestPending = true;
    const quint64 generation = ++m_batteryRequestGeneration;
    if (!m_batteryGauge->value().has_value()) {
        m_batteryValue->setText(tr("Reading battery…"));
    }

    QTimer::singleShot(kBatteryResponseTimeoutMs, this, [this, generation] {
        if (!m_batteryRequestPending
            || generation != m_batteryRequestGeneration) {
            return;
        }
        m_batteryRequestPending = false;
        logDebug(tr("The keyboard did not answer the battery query"));
        if (!m_batteryGauge->value().has_value()) {
            m_batteryValue->setText(tr("No response"));
            m_batteryState->setText(
                tr("The next attempt will run automatically."));
        }
    });
}

void MainWindow::recordReport(const HidReport &report)
{
    if (!m_profile.has_value()) {
        return;
    }
    const auto reading = BatteryDecoder::decode(report, *m_profile);
    if (!reading.has_value()) {
        return;
    }

    m_batteryRequestPending = false;
    setBattery(*reading);
}

void MainWindow::setBattery(const BatteryReading &reading)
{
    m_lastBatteryReading = reading;
    const std::optional<int> previousValue = m_batteryGauge->value();
    m_batteryGauge->setValue(reading.percent);
    m_batteryValue->setText(QStringLiteral("%1%").arg(reading.percent));
    if (reading.charging.has_value()) {
        m_batteryState->setText(
            *reading.charging ? tr("The keyboard is charging.")
                              : tr("The keyboard is running on battery."));
    } else {
        m_batteryState->setText(tr("Battery status received over USB HID."));
    }
    if (!previousValue.has_value() || *previousValue != reading.percent) {
        logDebug(tr("Battery updated: %1%").arg(reading.percent));
    }
}

void MainWindow::showDebugLogs()
{
    m_debugWindow->showTab(DebugWindow::Tab::Logs);
}

void MainWindow::showDebugTelemetry()
{
    m_debugWindow->showTab(DebugWindow::Tab::Telemetry);
}

} // namespace strikepro
