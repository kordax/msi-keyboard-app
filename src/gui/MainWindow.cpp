#include "MainWindow.h"

#include "BatteryGauge.h"
#include "DebugWindow.h"
#include "device/AppPaths.h"

#include <QAction>
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
    layout->addWidget(valueLabel);
    layout->addWidget(captionLabel);
    return tile;
}

QWidget *makeStatusRow(
    const QString &title,
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
    *detail = new QLabel(row);
    (*detail)->setProperty("role", QStringLiteral("muted"));
    (*detail)->setWordWrap(false);
    labels->addWidget(titleLabel);
    labels->addWidget(*detail);

    layout->addWidget(*dot, 0, Qt::AlignTop);
    layout->addLayout(labels, 1);
    return row;
}

const HidInterface *preferredBatteryInterface(const QList<HidInterface> &interfaces)
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
    setWindowTitle(QStringLiteral("MSI Keyboard"));
    setMinimumSize(920, 600);
    resize(1080, 680);

    menuBar()->setNativeMenuBar(false);
    QMenu *debugMenu = menuBar()->addMenu(QStringLiteral("Debug"));
    QAction *logsAction = debugMenu->addAction(QStringLiteral("Logs"));
    QAction *telemetryAction = debugMenu->addAction(QStringLiteral("Telemetry"));
    connect(logsAction, &QAction::triggered, this, &MainWindow::showDebugLogs);
    connect(
        telemetryAction,
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
    auto *title = new QLabel(QStringLiteral("STRIKE PRO"), central);
    title->setObjectName(QStringLiteral("title"));
    auto *subtitle = new QLabel(
        QStringLiteral("Battery manager for Linux"),
        central);
    subtitle->setProperty("role", QStringLiteral("muted"));
    titleColumn->addWidget(title);
    titleColumn->addWidget(subtitle);
    header->addLayout(titleColumn);
    header->addStretch();

    m_connectionBadge = new QLabel(QStringLiteral("НЕ ПОДКЛЮЧЕНА"), central);
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
    auto *batteryCaption = new QLabel(QStringLiteral("АККУМУЛЯТОР"), batteryCard);
    batteryCaption->setProperty("role", QStringLiteral("eyebrow"));
    m_batteryValue = new QLabel(QStringLiteral("Нет данных"), batteryCard);
    m_batteryValue->setObjectName(QStringLiteral("batteryHeadline"));
    m_batteryValue->setWordWrap(true);
    m_batteryState = new QLabel(
        QStringLiteral("Подключите клавиатуру, чтобы начать мониторинг."),
        batteryCard);
    m_batteryState->setProperty("role", QStringLiteral("muted"));
    m_batteryState->setWordWrap(true);
    batteryDetails->addWidget(batteryCaption);
    batteryDetails->addWidget(m_batteryValue);
    batteryDetails->addWidget(m_batteryState);
    batteryDetails->addStretch();

    auto *meta = new QHBoxLayout;
    meta->setSpacing(8);
    meta->addWidget(
        makeMetaTile(QStringLiteral("AUTO"), QStringLiteral("опрос"), batteryCard));
    meta->addWidget(
        makeMetaTile(QStringLiteral("15 сек"), QStringLiteral("интервал"), batteryCard));
    meta->addWidget(
        makeMetaTile(QStringLiteral("READ ONLY"), QStringLiteral("режим"), batteryCard));
    batteryDetails->addLayout(meta);
    batteryLayout->addLayout(batteryDetails, 1);
    dashboard->addWidget(batteryCard, 3);

    auto *deviceCard = makeCard(central);
    deviceCard->setMinimumWidth(390);
    auto *deviceLayout = new QVBoxLayout(deviceCard);
    deviceLayout->setContentsMargins(23, 21, 23, 21);
    deviceLayout->setSpacing(9);

    auto *deviceCaption = new QLabel(QStringLiteral("УСТРОЙСТВО"), deviceCard);
    deviceCaption->setProperty("role", QStringLiteral("eyebrow"));
    m_deviceLabel = new QLabel(QStringLiteral("MSI Strike Pro"), deviceCard);
    m_deviceLabel->setObjectName(QStringLiteral("deviceName"));
    m_deviceLabel->setWordWrap(true);
    deviceLayout->addWidget(deviceCaption);
    deviceLayout->addWidget(m_deviceLabel);

    m_deviceImage = new QLabel(deviceCard);
    m_deviceImage->setObjectName(QStringLiteral("deviceArtwork"));
    m_deviceImage->setAlignment(Qt::AlignCenter);
    m_deviceImage->setMinimumHeight(175);
    m_deviceImage->setVisible(false);
    const QPixmap artwork(QStringLiteral(":/assets/keyboard/strike_pro.png"));
    if (!artwork.isNull()) {
        m_deviceImage->setPixmap(
            artwork.scaled(
                360,
                230,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
    } else {
        m_deviceImage->setText(QStringLiteral("MSI STRIKE PRO"));
    }
    deviceLayout->addWidget(m_deviceImage);

    deviceLayout->addWidget(
        makeStatusRow(
            QStringLiteral("Подключение"),
            &m_deviceDot,
            &m_deviceStatus,
            deviceCard));
    deviceLayout->addWidget(
        makeStatusRow(
            QStringLiteral("Доступ к батарее"),
            &m_accessDot,
            &m_accessStatus,
            deviceCard));
    deviceLayout->addWidget(
        makeStatusRow(
            QStringLiteral("Протокол"),
            &m_protocolDot,
            &m_protocolStatus,
            deviceCard));

    m_accessLabel = new QLabel(deviceCard);
    m_accessLabel->setObjectName(QStringLiteral("accessBanner"));
    m_accessLabel->setWordWrap(true);
    m_accessLabel->setProperty("tone", QStringLiteral("off"));
    deviceLayout->addWidget(m_accessLabel);
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
        QLabel#accessBanner {
            border-radius: 8px;
            padding: 9px 11px;
            font-size: 11px;
        }
        QLabel#accessBanner[tone="off"] {
            background: #20242c;
            color: #868e9d;
        }
        QLabel#accessBanner[tone="ok"] {
            background: #123024;
            color: #72dba7;
        }
        QLabel#accessBanner[tone="warn"] {
            background: #352619;
            color: #f1bb75;
        }
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
}

QString MainWindow::profilePath() const
{
    return defaultProtocolProfilePath();
}

void MainWindow::setStatus(
    QLabel *dot,
    QLabel *detail,
    const QString &tone,
    const QString &text)
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
        m_protocolStatus->setToolTip(m_profile->path);
        setStatus(
            m_protocolDot,
            m_protocolStatus,
            QStringLiteral("ok"),
            QStringLiteral("Декодер готов"));
        logDebug(QStringLiteral("Профиль протокола загружен: %1").arg(m_profile->path));
        QTimer::singleShot(0, this, &MainWindow::requestBattery);
    } else {
        m_profile.reset();
        m_protocolStatus->setToolTip(profilePath());
        setStatus(
            m_protocolDot,
            m_protocolStatus,
            QStringLiteral("warn"),
            QStringLiteral("Профиль батареи недоступен"));
        logDebug(
            error.isEmpty()
                ? QStringLiteral("Профиль батареи недоступен")
                : QStringLiteral("Ошибка профиля: %1").arg(error));
    }
}

void MainWindow::updateInterfaces(const QList<HidInterface> &interfaces)
{
    m_interfaces = interfaces;
    const bool connected = !interfaces.isEmpty();
    const HidInterface *batteryInterface = preferredBatteryInterface(interfaces);
    const quint16 activeProductId =
        batteryInterface == nullptr ? 0 : batteryInterface->productId;
    const bool transportChanged = activeProductId != m_activeProductId;
    m_activeProductId = activeProductId;
    m_canQueryBattery =
        batteryInterface != nullptr && batteryInterface->readable
        && batteryInterface->writable;

    m_batteryGauge->setDeviceConnected(connected);
    m_deviceImage->setVisible(connected);

    m_connectionBadge->setText(
        connected ? QStringLiteral("ПОДКЛЮЧЕНА") : QStringLiteral("НЕ ПОДКЛЮЧЕНА"));
    m_connectionBadge->setProperty(
        "tone", connected ? QStringLiteral("ok") : QStringLiteral("off"));
    refreshStyle(m_connectionBadge);

    if (!connected) {
        if (transportChanged) {
            logDebug(QStringLiteral("MSI Strike Pro отключена"));
        }
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_batteryGauge->setValue(std::nullopt);
        m_batteryValue->setText(QStringLiteral("Нет устройства"));
        m_batteryState->setText(
            QStringLiteral("Ожидается USB-приёмник или проводное подключение."));
        setStatus(
            m_deviceDot,
            m_deviceStatus,
            QStringLiteral("off"),
            QStringLiteral("Клавиатура не обнаружена"));
        setStatus(
            m_accessDot,
            m_accessStatus,
            QStringLiteral("off"),
            QStringLiteral("Ожидание устройства"));
        m_accessLabel->setText(
            QStringLiteral("Подключите клавиатуру. Поиск выполняется автоматически."));
        m_accessLabel->setProperty("tone", QStringLiteral("off"));
        refreshStyle(m_accessLabel);
        return;
    }

    const bool wired = activeProductId == kStrikeProWiredProductId;
    m_deviceLabel->setText(
        wired
            ? QStringLiteral("MSI Strike Pro · USB")
            : QStringLiteral("MSI Strike Pro · 2.4 ГГц"));
    setStatus(
        m_deviceDot,
        m_deviceStatus,
        QStringLiteral("ok"),
        wired
            ? QStringLiteral("Проводное подключение")
            : QStringLiteral("Беспроводной приёмник"));

    if (transportChanged) {
        logDebug(
            wired
                ? QStringLiteral("MSI Strike Pro обнаружена через USB")
                : QStringLiteral("MSI Strike Pro обнаружена через приёмник 2.4 ГГц"));
        ++m_batteryRequestGeneration;
        m_batteryRequestPending = false;
        m_batteryGauge->setValue(std::nullopt);
        m_batteryValue->setText(QStringLiteral("Получение заряда…"));
        m_batteryState->setText(QStringLiteral("Первый запрос выполняется автоматически."));
    }

    if (m_canQueryBattery) {
        setStatus(
            m_accessDot,
            m_accessStatus,
            QStringLiteral("ok"),
            QStringLiteral("Канал батареи доступен"));
        m_accessLabel->setText(
            QStringLiteral("Заряд обновляется автоматически каждые 15 секунд."));
        m_accessLabel->setProperty("tone", QStringLiteral("ok"));
        QTimer::singleShot(0, this, &MainWindow::requestBattery);
    } else {
        setStatus(
            m_accessDot,
            m_accessStatus,
            QStringLiteral("warn"),
            QStringLiteral("Нет доступа к hidraw"));
        m_accessLabel->setText(
            QStringLiteral("Установите udev-правило, чтобы читать заряд."));
        m_accessLabel->setProperty("tone", QStringLiteral("warn"));
        if (!m_batteryGauge->value().has_value()) {
            m_batteryValue->setText(QStringLiteral("Нужен доступ к HID"));
            m_batteryState->setText(
                QStringLiteral("Устройство найдено, но чтение батареи закрыто системой."));
        }
    }
    refreshStyle(m_accessLabel);
}

void MainWindow::requestBattery()
{
    if (!m_canQueryBattery || m_batteryRequestPending || !m_profile.has_value()
        || !m_profile->canDecodePercentage()) {
        return;
    }

    QString error;
    if (!m_monitor->requestBattery(&error)) {
        logDebug(QStringLiteral("Battery query: %1").arg(error));
        if (!m_batteryGauge->value().has_value()) {
            m_batteryValue->setText(QStringLiteral("Нет ответа"));
            m_batteryState->setText(error);
        }
        return;
    }

    m_batteryRequestPending = true;
    const quint64 generation = ++m_batteryRequestGeneration;
    if (!m_batteryGauge->value().has_value()) {
        m_batteryValue->setText(QStringLiteral("Получение заряда…"));
    }

    QTimer::singleShot(
        kBatteryResponseTimeoutMs,
        this,
        [this, generation] {
            if (!m_batteryRequestPending
                || generation != m_batteryRequestGeneration) {
                return;
            }
            m_batteryRequestPending = false;
            logDebug(QStringLiteral("Клавиатура не ответила на battery-query"));
            if (!m_batteryGauge->value().has_value()) {
                m_batteryValue->setText(QStringLiteral("Нет ответа"));
                m_batteryState->setText(
                    QStringLiteral("Следующая попытка будет выполнена автоматически."));
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
    const std::optional<int> previousValue = m_batteryGauge->value();
    m_batteryGauge->setValue(reading.percent);
    m_batteryValue->setText(QStringLiteral("%1%").arg(reading.percent));
    if (reading.charging.has_value()) {
        m_batteryState->setText(
            *reading.charging
                ? QStringLiteral("Клавиатура подключена к зарядке.")
                : QStringLiteral("Клавиатура работает от аккумулятора."));
    } else {
        m_batteryState->setText(QStringLiteral("Заряд получен через USB HID."));
    }
    if (!previousValue.has_value() || *previousValue != reading.percent) {
        logDebug(QStringLiteral("Заряд обновлён: %1%").arg(reading.percent));
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
