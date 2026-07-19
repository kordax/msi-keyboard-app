#include "MainWindow.h"

#include "BatteryGauge.h"
#include "DebugWindow.h"
#include "device/AppPaths.h"
#include "i18n/LanguageManager.h"

#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
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

constexpr int kBatteryPollIntervalMs = 2'000;
constexpr int kBatteryResponseTimeoutMs = 900;

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
    auto *layout = new QVBoxLayout(row);
    layout->setContentsMargins(0, 4, 0, 4);
    layout->setSpacing(2);

    *dot = new QLabel(row);
    (*dot)->setProperty("role", QStringLiteral("statusDot"));
    (*dot)->setProperty("tone", QStringLiteral("off"));
    (*dot)->setFixedSize(10, 10);
    auto *glow = new QGraphicsDropShadowEffect(*dot);
    glow->setBlurRadius(24.0);
    glow->setColor(QColor(QStringLiteral("#55e89b")));
    glow->setOffset(0.0, 0.0);
    glow->setEnabled(false);
    (*dot)->setGraphicsEffect(glow);

    auto *titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(10, 0, 0, 0);
    titleRow->setSpacing(12);
    auto *titleLabel = new QLabel(title, row);
    titleLabel->setProperty("role", QStringLiteral("statusTitle"));
    if (titleOutput != nullptr) {
        *titleOutput = titleLabel;
    }
    titleRow->addWidget(*dot, 0, Qt::AlignVCenter);
    titleRow->addWidget(titleLabel, 1, Qt::AlignVCenter);

    *detail = new QLabel(row);
    (*detail)->setProperty("role", QStringLiteral("statusDetail"));
    (*detail)->setProperty("tone", QStringLiteral("off"));
    (*detail)->setWordWrap(false);

    auto *detailRow = new QHBoxLayout;
    detailRow->setContentsMargins(32, 0, 0, 0);
    detailRow->addWidget(*detail, 1);

    layout->addLayout(titleRow);
    layout->addLayout(detailRow);
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

bool hasAccessibleBatteryInterface(const QList<HidInterface> &interfaces)
{
    return std::ranges::any_of(interfaces, [](const HidInterface &interface) {
        return interface.interfaceNumber == 1 && interface.readable
               && interface.writable;
    });
}

bool hasProduct(const QList<HidInterface> &interfaces, const quint16 productId)
{
    return std::ranges::any_of(
        interfaces,
        [productId](const HidInterface &interface) {
            return interface.productId == productId;
        });
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
        m_monitor,
        &HidMonitor::deviceEvent,
        this,
        &MainWindow::handleDeviceEvent);
    connect(
        m_debugWindow,
        &DebugWindow::protocolReloadRequested,
        this,
        &MainWindow::reloadProtocolProfile);

    m_batteryPollTimer.setInterval(kBatteryPollIntervalMs);
    m_batteryPollTimer.setTimerType(Qt::CoarseTimer);
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
    root->setContentsMargins(28, 20, 28, 26);
    root->setSpacing(14);

    auto *header = new QHBoxLayout;
    header->setSpacing(14);

    auto *titleColumn = new QVBoxLayout;
    titleColumn->setSpacing(1);
    m_title = new QLabel(central);
    m_title->setObjectName(QStringLiteral("title"));
    titleColumn->addWidget(m_title);
    header->addLayout(titleColumn);
    header->addStretch();
    root->addLayout(header);

    auto *dashboard = new QHBoxLayout;
    dashboard->setSpacing(1);

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

    auto *deviceHeader = new QHBoxLayout;
    deviceHeader->setSpacing(12);
    auto *deviceTitleColumn = new QVBoxLayout;
    deviceTitleColumn->setSpacing(3);
    m_deviceCaption = new QLabel(deviceCard);
    m_deviceCaption->setProperty("role", QStringLiteral("eyebrow"));
    m_deviceLabel = new QLabel(deviceCard);
    m_deviceLabel->setObjectName(QStringLiteral("deviceName"));
    m_deviceLabel->setWordWrap(true);
    deviceTitleColumn->addWidget(m_deviceCaption);
    deviceTitleColumn->addWidget(m_deviceLabel);
    deviceHeader->addLayout(deviceTitleColumn, 1);

    m_connectionBadge = new QLabel(deviceCard);
    m_connectionBadge->setObjectName(QStringLiteral("connectionBadge"));
    m_connectionBadge->setProperty("tone", QStringLiteral("ok"));
    m_connectionBadge->setVisible(false);
    deviceHeader->addWidget(m_connectionBadge, 0, Qt::AlignVCenter);
    deviceLayout->addLayout(deviceHeader);

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
        QMainWindow, QDialog#debugWindow { background: #0d0d0d; }
        QWidget {
            color: #f1f1f1;
            font-family: "Inter", "Noto Sans", sans-serif;
            font-size: 13px;
        }
        QWidget#surface { background: #111111; }
        QMenuBar {
            background: #111111;
            color: #b6b6b6;
            border-bottom: 1px solid #292929;
            padding: 3px 8px;
        }
        QMenuBar::item {
            background: transparent;
            padding: 6px 10px;
        }
        QMenuBar::item:selected { background: #292929; color: #ffffff; }
        QMenu {
            background: #1b1b1b;
            color: #dddddd;
            border: 1px solid #343434;
            padding: 5px;
        }
        QMenu::item { padding: 7px 26px 7px 10px; }
        QMenu::item:selected { background: #303030; color: #ffffff; }
        QLabel#title {
            color: #ffffff;
            font-size: 25px;
            font-weight: 750;
            letter-spacing: 1.6px;
        }
        QLabel#connectionBadge {
            padding: 4px 0 4px 10px;
            font-size: 10px;
            font-weight: 750;
            letter-spacing: 0.7px;
        }
        QLabel#connectionBadge[tone="off"] {
            background: transparent;
            color: #a0a0a0;
        }
        QLabel#connectionBadge[tone="ok"] {
            background: transparent;
            color: #cfcfcf;
            border: none;
            border-left: 2px solid #666666;
        }
        QLabel#connectionBadge[tone="problem"] {
            background: transparent;
            color: #ef8b8b;
            border: none;
            border-left: 2px solid #b84f4f;
        }
        QFrame#card {
            background: #181818;
            border: none;
            border-top: 1px solid #353535;
            border-bottom: 1px solid #2c2c2c;
            border-radius: 0;
        }
        QLabel[role="eyebrow"] {
            color: #8c8c8c;
            font-size: 10px;
            font-weight: 750;
            letter-spacing: 1.4px;
        }
        QLabel[role="muted"] { color: #9c9c9c; }
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
            background: transparent;
            border: none;
            border-left: 2px solid #484848;
            border-radius: 0;
        }
        QLabel[role="metaValue"] {
            color: #eeeeee;
            font-size: 12px;
            font-weight: 700;
        }
        QLabel[role="statusTitle"] {
            color: #dedede;
            font-size: 12px;
            font-weight: 650;
        }
        QLabel[role="statusDot"] {
            border-radius: 5px;
            background: #555555;
        }
        QLabel[role="statusDot"][tone="off"] { background: #555555; }
        QLabel[role="statusDot"][tone="ok"] {
            background: #55e89b;
            border: 1px solid #9affc7;
        }
        QLabel[role="statusDot"][tone="warn"],
        QLabel[role="statusDot"][tone="problem"] {
            background: #df6464;
            border: 1px solid #f28c8c;
        }
        QLabel[role="statusDetail"] { color: #9c9c9c; }
        QLabel[role="statusDetail"][tone="ok"] { color: #65e6a0; }
        QLabel[role="statusDetail"][tone="problem"] { color: #ef7777; }
        QLabel[role="counter"] {
            background: #282828;
            color: #b9b9b9;
            border-radius: 9px;
            padding: 4px 9px;
            font-size: 10px;
        }
        QPushButton {
            min-height: 18px;
            padding: 8px 13px;
            border-radius: 3px;
            font-weight: 600;
        }
        QPushButton[role="quiet"] {
            background: #262626;
            color: #d0d0d0;
            border: 1px solid #3a3a3a;
        }
        QPushButton[role="quiet"]:hover { background: #313131; }
        QPushButton:disabled {
            background: #202020;
            color: #616161;
            border-color: #2b2b2b;
        }
        QTabWidget::pane {
            background: #161616;
            border: 1px solid #303030;
            border-radius: 0;
        }
        QTabBar::tab {
            background: #191919;
            color: #999999;
            border: 1px solid #303030;
            padding: 8px 16px;
        }
        QTabBar::tab:selected { background: #303030; color: #ffffff; }
        QPlainTextEdit#logView,
        QTableWidget#reportTable {
            background: #121212;
            alternate-background-color: #181818;
            color: #cecece;
            border: 1px solid #303030;
            border-radius: 9px;
            selection-background-color: #3a1b25;
            selection-color: #ffffff;
        }
        QHeaderView::section {
            background: #202020;
            color: #949494;
            border: none;
            border-bottom: 1px solid #303030;
            padding: 8px;
            font-size: 10px;
            font-weight: 700;
        }
        QTableCornerButton::section {
            background: #202020;
            border: none;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            margin: 3px;
        }
        QScrollBar::handle:vertical {
            background: #3b3b3b;
            border-radius: 3px;
            min-height: 24px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QToolTip {
            background: #292929;
            color: #f4f5f7;
            border: 1px solid #444444;
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
    m_modeValue->setText(tr("READ ONLY MODE"));
    m_modeCaption->clear();
    m_modeCaption->setVisible(false);
    m_deviceCaption->setText(tr("DEVICE"));
    m_deviceStatusTitle->setText(tr("Status"));
    m_connectionBadge->setText(tr("CONNECTED"));

    refreshConnectionUi();
    if (m_connectionState == ConnectionState::Connected
        && m_lastBatteryReading.has_value()) {
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
    detail->setProperty("tone", tone);
    detail->setText(text);
    if (auto *glow =
            qobject_cast<QGraphicsDropShadowEffect *>(dot->graphicsEffect())) {
        glow->setEnabled(tone == QStringLiteral("ok"));
    }
    refreshStyle(dot);
    refreshStyle(detail);
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

void MainWindow::clearBattery()
{
    m_lastBatteryReading.reset();
    m_batteryGauge->setValue(std::nullopt);
}

void MainWindow::setConnectionState(const ConnectionState state)
{
    m_connectionState = state;
    refreshConnectionUi();
}

void MainWindow::refreshConnectionUi()
{
    const bool connected = m_connectionState == ConnectionState::Connected;
    m_connectionBadge->setVisible(connected);
    m_connectionBadge->setProperty("tone", QStringLiteral("ok"));
    refreshStyle(m_connectionBadge);
    m_batteryGauge->setDeviceConnected(connected);
    m_deviceImage->setVisible(connected);

    switch (m_connectionState) {
    case ConnectionState::Absent:
        m_deviceLabel->setText(tr("No MSI keyboard detected"));
        m_batteryValue->setText(tr("No device"));
        m_batteryState->setText(tr("Connect a keyboard or its USB receiver."));
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("off"),
            tr("No supported keyboard detected"));
        break;
    case ConnectionState::Probing:
        m_deviceLabel->setText(tr("MSI Strike Pro"));
        m_batteryValue->setText(tr("Checking…"));
        m_batteryState->setText(
            tr("Waiting for a response from the keyboard."));
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("off"),
            tr("Checking connection"));
        break;
    case ConnectionState::Connected: {
        const bool wired = m_activeProductId == kStrikeProWiredProductId;
        m_deviceLabel->setText(
            wired ? tr("MSI Strike Pro · USB")
                  : tr("MSI Strike Pro · 2.4 GHz"));
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("ok"),
            wired ? tr("Connected via USB") : tr("Connected via 2.4 GHz"));
        if (!m_lastBatteryReading.has_value()) {
            m_batteryValue->setText(tr("Reading battery…"));
            m_batteryState->setText(tr("Waiting for battery data."));
        }
        break;
    }
    case ConnectionState::AccessDenied:
        m_deviceLabel->setText(tr("MSI Strike Pro"));
        m_batteryValue->setText(tr("HID access required"));
        m_batteryState->setText(
            tr("The device is present, but Linux denied HID access."));
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("problem"),
            tr("Permission problem"));
        break;
    case ConnectionState::Unresponsive:
        m_deviceLabel->setText(tr("MSI Strike Pro receiver"));
        m_batteryValue->setText(tr("No response"));
        m_batteryState->setText(tr(
            "The USB transport is present, but the keyboard did not answer."));
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("problem"),
            tr("Keyboard not responding"));
        break;
    }
}

void MainWindow::updateInterfaces(const QList<HidInterface> &interfaces)
{
    const ConnectionState previousState = m_connectionState;
    m_interfaces = interfaces;
    const HidInterface *batteryInterface =
        preferredBatteryInterface(interfaces);
    const bool transportPresent = batteryInterface != nullptr;
    m_canQueryBattery = hasAccessibleBatteryInterface(interfaces);

    if (!transportPresent) {
        if (previousState != ConnectionState::Absent) {
            logDebug(tr("MSI Strike Pro disconnected"));
        }
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_activeProductId = 0;
        clearBattery();
        setConnectionState(ConnectionState::Absent);
        return;
    }

    if (!m_canQueryBattery) {
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_activeProductId = 0;
        clearBattery();
        setConnectionState(ConnectionState::AccessDenied);
        return;
    }

    const bool activeTransportStillPresent =
        m_activeProductId != 0 && hasProduct(interfaces, m_activeProductId);
    if (m_connectionState == ConnectionState::Connected
        && activeTransportStillPresent) {
        refreshConnectionUi();
        return;
    }

    ++m_batteryRequestGeneration;
    m_batteryRequestPending = false;
    m_activeProductId = 0;
    clearBattery();
    setConnectionState(ConnectionState::Probing);
    QTimer::singleShot(0, this, &MainWindow::requestBattery);
}

void MainWindow::handleDeviceEvent()
{
    ++m_batteryRequestGeneration;
    m_batteryRequestPending = false;
    m_activeProductId = 0;
    clearBattery();
    if (m_canQueryBattery) {
        setConnectionState(ConnectionState::Probing);
    }
    QTimer::singleShot(125, this, &MainWindow::requestBattery);
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
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_activeProductId = 0;
        clearBattery();
        setConnectionState(
            m_canQueryBattery ? ConnectionState::Unresponsive
                              : ConnectionState::AccessDenied);
        return;
    }

    m_batteryRequestPending = true;
    const quint64 generation = ++m_batteryRequestGeneration;

    QTimer::singleShot(kBatteryResponseTimeoutMs, this, [this, generation] {
        if (!m_batteryRequestPending
            || generation != m_batteryRequestGeneration) {
            return;
        }
        m_batteryRequestPending = false;
        logDebug(tr("The keyboard did not answer the battery query"));
        m_activeProductId = 0;
        clearBattery();
        setConnectionState(ConnectionState::Unresponsive);
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
    m_activeProductId = report.productId;
    if (m_connectionState != ConnectionState::Connected) {
        logDebug(
            report.productId == kStrikeProWiredProductId
                ? tr("MSI Strike Pro answered over USB")
                : tr("MSI Strike Pro answered through the 2.4 GHz receiver"));
    }
    setConnectionState(ConnectionState::Connected);
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
