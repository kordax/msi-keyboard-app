#include "MainWindow.h"

#include "BatteryGauge.h"
#include "DebugWindow.h"
#include "TrayIndicator.h"
#include "device/AppPaths.h"
#include "i18n/LanguageManager.h"

#include <QAction>
#include <QActionGroup>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QSignalBlocker>
#include <QStyle>
#include <QToolTip>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

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

} // namespace

MainWindow::MainWindow(LanguageManager *languageManager, QWidget *parent)
    : QMainWindow(parent)
    , m_languageManager(languageManager)
    , m_monitor(new HidMonitor(this))
{
    buildUi();

    m_trayIndicator = new TrayIndicator(this, this);
    refreshConnectionUi();

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
    setMinimumSize(980, 620);
    resize(1180, 720);

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

    auto *deviceListCard = makeCard(central);
    deviceListCard->setMinimumWidth(235);
    deviceListCard->setMaximumWidth(285);
    auto *deviceListLayout = new QVBoxLayout(deviceListCard);
    deviceListLayout->setContentsMargins(15, 20, 15, 20);
    deviceListLayout->setSpacing(12);
    m_deviceListCaption = new QLabel(deviceListCard);
    m_deviceListCaption->setProperty("role", QStringLiteral("eyebrow"));
    deviceListLayout->addWidget(m_deviceListCaption);

    m_deviceList = new QListWidget(deviceListCard);
    m_deviceList->setObjectName(QStringLiteral("deviceList"));
    m_deviceList->setMouseTracking(true);
    m_deviceList->setSpacing(3);
    m_deviceList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    deviceListLayout->addWidget(m_deviceList, 1);
    connect(
        m_deviceList,
        &QListWidget::currentItemChanged,
        this,
        &MainWindow::selectDevice);
    connect(
        m_deviceList,
        &QListWidget::itemEntered,
        this,
        &MainWindow::showDeviceArtwork);

    m_emptyDevices = new QLabel(deviceListCard);
    m_emptyDevices->setObjectName(QStringLiteral("emptyDevices"));
    m_emptyDevices->setProperty("role", QStringLiteral("muted"));
    m_emptyDevices->setAlignment(Qt::AlignCenter);
    m_emptyDevices->setWordWrap(true);
    deviceListLayout->addWidget(m_emptyDevices, 1);
    dashboard->addWidget(deviceListCard);

    auto *detailCard = makeCard(central);
    auto *detailLayout = new QVBoxLayout(detailCard);
    detailLayout->setContentsMargins(0, 0, 0, 0);
    detailLayout->setSpacing(0);

    auto *batteryCard = new QWidget(detailCard);
    batteryCard->setObjectName(QStringLiteral("batterySection"));
    m_batterySection = batteryCard;
    batteryCard->setMinimumHeight(300);
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
    batteryLayout->addLayout(batteryDetails, 1);
    detailLayout->addWidget(batteryCard, 1);

    auto *deviceCard = new QWidget(detailCard);
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
    deviceHeader->addWidget(
        makeStatusRow(
            QString(),
            &m_deviceStatusTitle,
            &m_deviceDot,
            &m_deviceStatus,
            deviceCard),
        0,
        Qt::AlignVCenter);
    deviceLayout->addLayout(deviceHeader);

    m_deviceImage = new QLabel(deviceCard);
    m_deviceImage->setObjectName(QStringLiteral("deviceArtwork"));
    m_deviceImage->setAlignment(Qt::AlignCenter);
    m_deviceImage->setMinimumHeight(135);
    m_deviceImage->setVisible(false);
    m_deviceImage->clear();
    deviceLayout->addWidget(m_deviceImage);

    deviceLayout->addStretch();

    detailLayout->insertWidget(0, deviceCard);
    dashboard->addWidget(detailCard, 1);
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
        QListWidget#deviceList {
            background: transparent;
            border: none;
            outline: none;
            color: #c9c9c9;
        }
        QListWidget#deviceList::item {
            border: 1px solid transparent;
            border-left: 3px solid transparent;
            padding: 11px 10px;
            margin: 1px 0;
        }
        QListWidget#deviceList::item:hover {
            background: #222222;
            color: #ffffff;
        }
        QListWidget#deviceList::item:selected {
            background: #292929;
            border-color: #363636;
            border-left-color: #d44a62;
            color: #ffffff;
        }
        QLabel#emptyDevices { padding: 16px; }
        QWidget#batterySection { border-top: 1px solid #303030; }
        QLabel#title {
            color: #ffffff;
            font-size: 25px;
            font-weight: 750;
            letter-spacing: 1.6px;
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
    m_deviceListCaption->setText(tr("DEVICES"));
    m_emptyDevices->setText(tr("Connect a supported MSI keyboard."));
    m_batteryCaption->setText(tr("BATTERY"));
    m_deviceCaption->setText(tr("SELECTED DEVICE"));
    m_deviceStatusTitle->setText(tr("Status"));
    if (m_trayIndicator != nullptr) {
        m_trayIndicator->retranslateUi();
    }

    rebuildDeviceList();
    refreshConnectionUi();
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

MainWindow::DeviceRuntime *MainWindow::selectedDevice()
{
    const auto found = m_devices.find(m_selectedDeviceId);
    return found == m_devices.end() ? nullptr : &found.value();
}

const MainWindow::DeviceRuntime *MainWindow::selectedDevice() const
{
    const auto found = m_devices.constFind(m_selectedDeviceId);
    return found == m_devices.cend() ? nullptr : &found.value();
}

QString MainWindow::deviceName(const SupportedDevice &device) const
{
    const QString configuredName = device.definition.displayNameString();
    if (!configuredName.isEmpty()) {
        return configuredName;
    }
    return device.name.isEmpty() ? tr("Supported MSI keyboard") : device.name;
}

QString MainWindow::transportName(const SupportedDevice &device) const
{
    const bool wired = std::ranges::any_of(
        device.interfaces,
        [&device](const HidInterface &interface) {
            return interface.vendorId == device.definition.vendorId
                   && interface.productId == device.definition.usbProductId;
        });
    const bool wireless = std::ranges::any_of(
        device.interfaces,
        [&device](const HidInterface &interface) {
            return device.definition.dongleProductId != 0
                   && interface.vendorId == device.definition.vendorId
                   && interface.productId == device.definition.dongleProductId;
        });
    if (wired && wireless) {
        return tr("USB + 2.4 GHz");
    }
    return transportName(device, device.productId);
}

QString MainWindow::transportName(
    const SupportedDevice &device, const quint16 productId) const
{
    if (!device.definition.matchesProduct(productId)) {
        return tr("HID");
    }
    return device.definition.transportForProduct(productId)
                   == DeviceTransport::Dongle
               ? tr("2.4 GHz")
               : tr("USB");
}

QString MainWindow::transportName(const quint16 productId) const
{
    const DeviceDefinition *definition =
        findDeviceDefinitionByProductId(productId);
    if (definition == nullptr) {
        return tr("HID");
    }
    return definition->transportForProduct(productId) == DeviceTransport::Dongle
               ? tr("2.4 GHz")
               : tr("USB");
}

QString MainWindow::deviceListStatus(const DeviceRuntime &runtime) const
{
    switch (runtime.connectionState) {
    case ConnectionState::Absent:
        return tr("Disconnected");
    case ConnectionState::Probing:
        return tr("Checking connection");
    case ConnectionState::Connected:
        return transportName(runtime.device);
    case ConnectionState::AccessDenied:
        return tr("HID access required");
    case ConnectionState::Unresponsive:
        return tr("Not responding");
    }
    return QString();
}

void MainWindow::rebuildDeviceList()
{
    if (m_deviceList == nullptr) {
        return;
    }

    const QSignalBlocker blocker(m_deviceList);
    m_deviceList->clear();
    for (const QString &deviceId : std::as_const(m_deviceOrder)) {
        const auto found = m_devices.constFind(deviceId);
        if (found == m_devices.cend()) {
            continue;
        }
        const DeviceRuntime &runtime = found.value();
        auto *item = new QListWidgetItem(
            QStringLiteral("%1 · %2").arg(
                deviceName(runtime.device),
                deviceListStatus(runtime)),
            m_deviceList);
        item->setData(Qt::UserRole, deviceId);
        item->setSizeHint(QSize(0, 58));
        if (deviceId == m_selectedDeviceId) {
            m_deviceList->setCurrentItem(item);
        }
    }

    const bool empty = m_deviceOrder.isEmpty();
    m_deviceList->setVisible(!empty);
    m_emptyDevices->setVisible(empty);
}

void MainWindow::selectDevice(
    QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    const QString deviceId =
        current == nullptr ? QString() : current->data(Qt::UserRole).toString();
    if (deviceId == m_selectedDeviceId) {
        return;
    }

    ++m_batteryRequestGeneration;
    m_batteryRequestPending = false;
    m_pendingBatteryDeviceId.clear();
    m_selectedDeviceId = deviceId;

    if (DeviceRuntime *runtime = selectedDevice()) {
        if (!runtime->device.supportsBattery()) {
            runtime->connectionState = ConnectionState::Connected;
        } else if (!runtime->device.canQueryBattery()) {
            runtime->connectionState = ConnectionState::AccessDenied;
        } else if (!runtime->battery.has_value()) {
            runtime->connectionState = ConnectionState::Probing;
        }
    }

    rebuildDeviceList();
    refreshConnectionUi();
    QTimer::singleShot(0, this, &MainWindow::requestBattery);
}

void MainWindow::showDeviceArtwork(QListWidgetItem *item)
{
    if (item == nullptr) {
        return;
    }
    const auto found = m_devices.constFind(item->data(Qt::UserRole).toString());
    if (found == m_devices.cend()) {
        return;
    }

    const DeviceRuntime &runtime = found.value();
    const QString artwork =
        runtime.device.definition.artworkResourceString().toHtmlEscaped();
    const QString image =
        artwork.isEmpty()
            ? QString()
            : QStringLiteral("<br/><img src='%1' width='360'/>").arg(artwork);
    const QString popover =
        QStringLiteral("<div style='white-space:nowrap'><b>%1 · %2</b>%3</div>")
            .arg(
                deviceName(runtime.device).toHtmlEscaped(),
                transportName(runtime.device).toHtmlEscaped(),
                image);
    QToolTip::showText(QCursor::pos(), popover, m_deviceList);
}

void MainWindow::clearBattery(DeviceRuntime &runtime)
{
    runtime.battery.reset();
    if (runtime.device.id == m_selectedDeviceId) {
        m_batteryGauge->setValue(std::nullopt);
    }
}

void MainWindow::setConnectionState(
    DeviceRuntime &runtime, const ConnectionState state)
{
    runtime.connectionState = state;
    rebuildDeviceList();
    if (runtime.device.id == m_selectedDeviceId) {
        refreshConnectionUi();
    }
}

void MainWindow::refreshTrayIndicator()
{
    if (m_trayIndicator == nullptr) {
        return;
    }

    TrayIndicator::State state;
    const DeviceRuntime *runtime = selectedDevice();
    if (runtime != nullptr) {
        state.deviceName = deviceName(runtime->device);
        if (runtime->battery.has_value()) {
            state.batteryPercent = runtime->battery->percent;
            state.charging = runtime->battery->charging;
        }
        switch (runtime->connectionState) {
        case ConnectionState::Absent:
            state.connectionState = TrayIndicator::ConnectionState::Unavailable;
            break;
        case ConnectionState::Probing:
            state.connectionState = TrayIndicator::ConnectionState::Probing;
            break;
        case ConnectionState::Connected:
            state.connectionState = TrayIndicator::ConnectionState::Connected;
            break;
        case ConnectionState::AccessDenied:
        case ConnectionState::Unresponsive:
            state.connectionState = TrayIndicator::ConnectionState::Problem;
            break;
        }
    }
    m_trayIndicator->setState(state);
}

void MainWindow::refreshConnectionUi()
{
    refreshTrayIndicator();
    const DeviceRuntime *runtime = selectedDevice();
    if (runtime == nullptr) {
        m_deviceLabel->setText(tr("No MSI keyboard detected"));
        m_deviceImage->setVisible(false);
        m_batterySection->setVisible(false);
        m_batteryGauge->setDeviceConnected(false);
        m_batteryGauge->setValue(std::nullopt);
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("off"),
            tr("No supported keyboard detected"));
        return;
    }

    const QString name = deviceName(runtime->device);
    const QString transport = transportName(runtime->device);
    const QString connectionTransport =
        runtime->activeProductId == 0
            ? transport
            : transportName(runtime->device, runtime->activeProductId);
    m_deviceLabel->setText(QStringLiteral("%1 · %2").arg(name, transport));

    QPixmap artwork(runtime->device.definition.artworkResourceString());
    m_deviceImage->setPixmap(QPixmap());
    m_deviceImage->setText(QString());
    if (!artwork.isNull()) {
        m_deviceImage->setPixmap(artwork.scaled(
            320,
            175,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
    } else {
        m_deviceImage->setText(name);
    }
    m_deviceImage->setVisible(true);

    const bool supportsBattery = runtime->device.supportsBattery();
    const bool connected =
        runtime->connectionState == ConnectionState::Connected;
    m_batterySection->setVisible(supportsBattery);
    m_batteryGauge->setDeviceConnected(connected);
    if (supportsBattery) {
        if (runtime->battery.has_value()) {
            const BatteryReading &reading = *runtime->battery;
            m_batteryGauge->setValue(reading.percent);
            m_batteryValue->setText(QStringLiteral("%1%").arg(reading.percent));
            if (reading.charging.has_value()) {
                m_batteryState->setText(
                    *reading.charging
                        ? tr("The keyboard is charging.")
                        : tr("The keyboard is running on battery."));
            } else {
                m_batteryState->setText(
                    tr("Battery status received over USB HID."));
            }
        } else {
            m_batteryGauge->setValue(std::nullopt);
            switch (runtime->connectionState) {
            case ConnectionState::AccessDenied:
                m_batteryValue->setText(tr("HID access required"));
                m_batteryState->setText(
                    tr("The device is present, but Linux denied HID access."));
                break;
            case ConnectionState::Unresponsive:
                m_batteryValue->setText(tr("No response"));
                m_batteryState->setText(tr("The USB transport is present, but "
                                           "the keyboard did not answer."));
                break;
            default:
                m_batteryValue->setText(tr("Reading battery…"));
                m_batteryState->setText(tr("Waiting for battery data."));
                break;
            }
        }
    }

    switch (runtime->connectionState) {
    case ConnectionState::Absent:
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("off"),
            tr("Disconnected"));
        break;
    case ConnectionState::Probing:
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("off"),
            tr("Checking connection"));
        break;
    case ConnectionState::Connected:
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("ok"),
            tr("Connected via %1").arg(connectionTransport));
        break;
    case ConnectionState::AccessDenied:
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("problem"),
            tr("Permission problem"));
        break;
    case ConnectionState::Unresponsive:
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
    const QList<SupportedDevice> detected = groupSupportedDevices(interfaces);
    QHash<QString, DeviceRuntime> updated;
    QStringList order;
    order.reserve(detected.size());

    for (const SupportedDevice &device : detected) {
        DeviceRuntime runtime;
        const auto previous = m_devices.constFind(device.id);
        if (previous != m_devices.cend()) {
            runtime = previous.value();
        }
        runtime.device = device;
        const bool activeTransportPresent =
            runtime.activeProductId == 0
            || std::ranges::any_of(
                device.interfaces,
                [&runtime](const HidInterface &interface) {
                    return interface.productId == runtime.activeProductId;
                });
        if (!device.supportsBattery()) {
            runtime.connectionState = ConnectionState::Connected;
            runtime.activeProductId = device.productId;
        } else if (!device.canQueryBattery()) {
            runtime.connectionState = ConnectionState::AccessDenied;
            runtime.activeProductId = 0;
            runtime.battery.reset();
        } else if (
            previous == m_devices.cend()
            || previous->connectionState == ConnectionState::AccessDenied
            || !activeTransportPresent) {
            runtime.connectionState = ConnectionState::Probing;
            if (!activeTransportPresent) {
                runtime.activeProductId = 0;
                runtime.battery.reset();
            }
        }
        updated.insert(device.id, runtime);
        order.push_back(device.id);

        if (previous == m_devices.cend()) {
            logDebug(tr("%1 detected via %2")
                         .arg(deviceName(device), transportName(device)));
        }
    }

    for (auto previous = m_devices.cbegin(); previous != m_devices.cend();
         ++previous) {
        if (!updated.contains(previous.key())) {
            logDebug(tr("%1 disconnected").arg(deviceName(previous->device)));
        }
    }

    if (!m_pendingBatteryDeviceId.isEmpty()
        && !updated.contains(m_pendingBatteryDeviceId)) {
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_pendingBatteryDeviceId.clear();
    }

    const QString previousSelection = m_selectedDeviceId;
    m_devices = std::move(updated);
    m_deviceOrder = std::move(order);
    m_selectedDeviceId = retainedDeviceSelection(detected, previousSelection);
    if (m_selectedDeviceId != previousSelection) {
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_pendingBatteryDeviceId.clear();
    }

    rebuildDeviceList();
    refreshConnectionUi();
    if (const DeviceRuntime *runtime = selectedDevice();
        runtime != nullptr
        && runtime->connectionState == ConnectionState::Probing) {
        QTimer::singleShot(0, this, &MainWindow::requestBattery);
    }
}

void MainWindow::handleDeviceEvent()
{
    ++m_batteryRequestGeneration;
    m_batteryRequestPending = false;
    m_pendingBatteryDeviceId.clear();
    QTimer::singleShot(125, this, &MainWindow::requestBattery);
}

void MainWindow::requestBattery()
{
    DeviceRuntime *runtime = selectedDevice();
    const HidInterface *batteryInterface =
        runtime == nullptr ? nullptr : runtime->device.batteryInterface();
    if (runtime == nullptr || batteryInterface == nullptr
        || !runtime->device.canQueryBattery() || m_batteryRequestPending
        || !m_profile.has_value() || !m_profile->canDecodePercentage()) {
        return;
    }

    QString error;
    if (!m_monitor->requestBattery(batteryInterface->devNode, &error)) {
        logDebug(tr("Battery query: %1").arg(error));
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_pendingBatteryDeviceId.clear();
        runtime->activeProductId = 0;
        clearBattery(*runtime);
        setConnectionState(
            *runtime,
            runtime->device.canQueryBattery() ? ConnectionState::Unresponsive
                                              : ConnectionState::AccessDenied);
        return;
    }

    m_batteryRequestPending = true;
    m_pendingBatteryDeviceId = runtime->device.id;
    const QString deviceId = runtime->device.id;
    const quint64 generation = ++m_batteryRequestGeneration;

    QTimer::singleShot(
        kBatteryResponseTimeoutMs,
        this,
        [this, generation, deviceId] {
            if (!m_batteryRequestPending
                || generation != m_batteryRequestGeneration
                || m_pendingBatteryDeviceId != deviceId) {
                return;
            }
            m_batteryRequestPending = false;
            m_pendingBatteryDeviceId.clear();
            logDebug(tr("The keyboard did not answer the battery query"));
            auto found = m_devices.find(deviceId);
            if (found == m_devices.end()) {
                return;
            }
            found->activeProductId = 0;
            clearBattery(found.value());
            setConnectionState(found.value(), ConnectionState::Unresponsive);
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

    auto runtime = std::ranges::find_if(
        m_devices,
        [&report](const DeviceRuntime &candidate) {
            return std::ranges::any_of(
                candidate.device.interfaces,
                [&report](const HidInterface &interface) {
                    return interface.devNode == report.devNode;
                });
        });
    if (runtime == m_devices.end() && report.devNode.isEmpty()) {
        runtime = std::ranges::find_if(
            m_devices,
            [&report](const DeviceRuntime &candidate) {
                return std::ranges::any_of(
                    candidate.device.interfaces,
                    [&report](const HidInterface &interface) {
                        return interface.productId == report.productId;
                    });
            });
    }
    if (runtime == m_devices.end()) {
        return;
    }

    if (m_pendingBatteryDeviceId == runtime->device.id) {
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_pendingBatteryDeviceId.clear();
    }
    if (runtime->connectionState != ConnectionState::Connected) {
        logDebug(tr("%1 answered over %2")
                     .arg(
                         deviceName(runtime->device),
                         transportName(runtime->device, report.productId)));
    }
    runtime->connectionState = ConnectionState::Connected;
    runtime->activeProductId = report.productId;
    setBattery(runtime.value(), *reading);
    rebuildDeviceList();
    if (runtime->device.id == m_selectedDeviceId) {
        refreshConnectionUi();
    }
}

void MainWindow::setBattery(
    DeviceRuntime &runtime, const BatteryReading &reading)
{
    const std::optional<int> previousValue =
        runtime.battery.has_value()
            ? std::optional<int>(runtime.battery->percent)
            : std::nullopt;
    runtime.battery = reading;
    if (!previousValue.has_value() || *previousValue != reading.percent) {
        logDebug(tr("Battery updated: %1%").arg(reading.percent));
    }
    if (runtime.device.id == m_selectedDeviceId) {
        refreshConnectionUi();
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
