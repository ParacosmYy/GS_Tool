/**
 * @file SerialConfigPanel.cpp
 * @brief 串口配置面板实现 - 串口参数配置、连接控制和状态指示
 *
 * 面板布局: 端口选择 → 参数配置 → 控制信号 → 驱动检测 → 连接按钮+状态指示器
 * 状态指示器通过QSS的statusIndicator[state="xxx"]控制圆点颜色。
 */

#include "serial/SerialConfigPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QSerialPortInfo>
#include <QIntValidator>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QTimer>

#include "serial/SerialDriverDetector.h"

// ---- 构造 ----

SerialConfigPanel::SerialConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    refreshPorts();
    updateDriverInfo();
}

// ---- UI布局 ----

void SerialConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ---- 端口选择区域 ----
    auto* portGroup = new QGroupBox(tr("端口"));
    portGroup->setObjectName("portGroup");
    auto* portLayout = new QHBoxLayout(portGroup);

    m_portCombo = new QComboBox;
    m_portCombo->setObjectName("portCombo");
    m_portCombo->setMinimumWidth(150);
    connect(m_portCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialConfigPanel::onPortComboChanged);

    m_refreshBtn = new QPushButton(tr("刷新"));
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setToolTip(tr("重新扫描系统中的串口设备"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &SerialConfigPanel::refreshPorts);

    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_refreshBtn);
    mainLayout->addWidget(portGroup);

    // ---- 串口参数区域 ----
    auto* paramGroup = new QGroupBox(tr("参数"));
    paramGroup->setObjectName("paramGroup");
    auto* formLayout = new QFormLayout(paramGroup);

    m_baudCombo = new QComboBox;
    m_baudCombo->setObjectName("baudCombo");
    m_baudCombo->setEditable(true);
    m_baudCombo->setToolTip(tr("通信速率(比特/秒)，常用值: 9600, 115200\n可直接输入自定义波特率"));
    m_baudCombo->addItems({"1200", "2400", "4800", "9600", "19200",
                           "38400", "57600", "115200", "230400",
                           "460800", "921600", "1000000"});
    m_baudCombo->setCurrentText("115200");
    m_baudCombo->lineEdit()->setValidator(new QIntValidator(300, 10000000, this));
    formLayout->addRow(tr("波特率:"), m_baudCombo);

    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->setObjectName("dataBitsCombo");
    m_dataBitsCombo->setToolTip(tr("每个数据帧的数据位数，绝大多数设备使用 8 位"));
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentIndex(3);
    formLayout->addRow(tr("数据位:"), m_dataBitsCombo);

    m_parityCombo = new QComboBox;
    m_parityCombo->setObjectName("parityCombo");
    m_parityCombo->setToolTip(tr("校验方式:\n无 - 不校验(最常用)\n偶校验/奇校验 - 简单错误检测"));
    m_parityCombo->addItems({tr("无"), tr("偶校验"), tr("奇校验"), tr("Mark"), tr("Space")});
    formLayout->addRow(tr("校验位:"), m_parityCombo);

    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->setObjectName("stopBitsCombo");
    m_stopBitsCombo->setToolTip(tr("帧结束的停止位数，绝大多数设备使用 1 位"));
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    formLayout->addRow(tr("停止位:"), m_stopBitsCombo);

    m_flowControlCombo = new QComboBox;
    m_flowControlCombo->setObjectName("flowControlCombo");
    m_flowControlCombo->setToolTip(tr("流量控制:\n无 - 不使用流控(最常用)\nRTS/CTS - 硬件流控\nXON/XOFF - 软件流控"));
    m_flowControlCombo->addItems({tr("无"), tr("RTS/CTS"), tr("XON/XOFF")});
    formLayout->addRow(tr("流控:"), m_flowControlCombo);

    mainLayout->addWidget(paramGroup);

    // ---- 控制信号 + 驱动检测 + 连接按钮 ----
    setupSignalAndConnectControls(mainLayout);

    mainLayout->addStretch();
}

/**
 * @brief 构建控制信号(DTR/RTS)、驱动检测信息和连接按钮区域
 *
 * 从 setupUI() 拆分出来，避免单个方法超过80行限制。
 * 包含: 控制信号GroupBox → 驱动检测信息 → 连接按钮+状态指示器
 */
void SerialConfigPanel::setupSignalAndConnectControls(QVBoxLayout* mainLayout)
{
    // ---- 控制信号 (DTR/RTS切换按钮，连接后可用) ----
    auto* signalGroup = new QGroupBox(tr("控制信号"));
    signalGroup->setObjectName("signalGroup");
    auto* signalLayout = new QHBoxLayout(signalGroup);

    m_dtrBtn = new QPushButton(tr("DTR HIGH"));
    m_dtrBtn->setObjectName("dtrBtn");
    m_dtrBtn->setCheckable(true);
    m_dtrBtn->setChecked(true);
    m_dtrBtn->setEnabled(false);
    m_dtrBtn->setToolTip(tr("数据终端就绪信号，点击切换 HIGH/LOW\n"
                            "部分设备需要 DTR 拉低才能复位(如 ESP32)"));

    m_rtsBtn = new QPushButton(tr("RTS HIGH"));
    m_rtsBtn->setObjectName("rtsBtn");
    m_rtsBtn->setCheckable(true);
    m_rtsBtn->setChecked(true);
    m_rtsBtn->setEnabled(false);
    m_rtsBtn->setToolTip(tr("请求发送信号，点击切换 HIGH/LOW\n"
                            "部分设备需要 RTS 拉低进入 bootloader(如 STM32)"));

    signalLayout->addWidget(m_dtrBtn);
    signalLayout->addWidget(m_rtsBtn);
    signalLayout->addStretch();

    connect(m_dtrBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_dtrState = checked;
        m_dtrBtn->setText(checked ? tr("DTR HIGH") : tr("DTR LOW"));
        refreshDtrStyle();
        emit dtrChanged(checked);
    });
    connect(m_rtsBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_rtsState = checked;
        m_rtsBtn->setText(checked ? tr("RTS HIGH") : tr("RTS LOW"));
        refreshRtsStyle();
        emit rtsChanged(checked);
    });
    // Initialize visual state to match default HIGH
    refreshDtrStyle();
    refreshRtsStyle();
    mainLayout->addWidget(signalGroup);

    // ---- 驱动检测信息 ----
    m_driverInfoLbl = new QLabel;
    m_driverInfoLbl->setObjectName("driverInfoLbl");
    m_driverInfoLbl->setWordWrap(true);
    mainLayout->addWidget(m_driverInfoLbl);

    // ---- 连接按钮 + 状态指示器 ----
    auto* connectLayout = new QHBoxLayout;

    m_statusIndicator = new QLabel;
    m_statusIndicator->setObjectName("statusIndicator");
    m_statusIndicator->setFixedSize(8, 8);
    m_statusIndicator->setToolTip(tr("未连接"));
    m_statusIndicator->setProperty("state", "disconnected");
    m_statusIndicator->style()->unpolish(m_statusIndicator);
    m_statusIndicator->style()->polish(m_statusIndicator);

    m_connectBtn = new QPushButton(tr("连接"));
    m_connectBtn->setObjectName("connectBtn");
    m_connectBtn->setMinimumHeight(36);
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connecting) return;
        if (m_connected) {
            emit disconnectRequested();
        } else {
            m_connecting = true;
            m_connectBtn->setEnabled(false);
            m_connectBtn->setText(tr("连接中..."));
            m_connectBtn->setProperty("state", "connecting");
            m_connectBtn->style()->unpolish(m_connectBtn);
            m_connectBtn->style()->polish(m_connectBtn);
            emit connectRequested();
        }
    });

    connectLayout->addWidget(m_statusIndicator);
    connectLayout->addWidget(m_connectBtn, 1);
    mainLayout->addLayout(connectLayout);
}

// ---- 状态管理 ----

void SerialConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connecting = false;

    m_connectBtn->setEnabled(true);
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_connectBtn->setProperty("state", connected ? "connected" : "");
    m_connectBtn->style()->unpolish(m_connectBtn);
    m_connectBtn->style()->polish(m_connectBtn);

    // 锁定/解锁配置控件
    m_portCombo->setEnabled(!connected);
    m_baudCombo->setEnabled(!connected);
    m_dataBitsCombo->setEnabled(!connected);
    m_parityCombo->setEnabled(!connected);
    m_stopBitsCombo->setEnabled(!connected);
    m_flowControlCombo->setEnabled(!connected);
    m_refreshBtn->setEnabled(!connected);
    m_dtrBtn->setEnabled(connected);
    m_rtsBtn->setEnabled(connected);

    // 更新状态指示器
    if (connected) {
        stopBreathAnimation();
        updateStatusIndicator("connected");
        m_statusIndicator->setToolTip(tr("已连接"));
    } else {
        stopBreathAnimation();
        updateStatusIndicator("disconnected");
        m_statusIndicator->setToolTip(tr("未连接"));
        updateConnectButtonState();
    }
}

void SerialConfigPanel::setError(const QString& errorMsg)
{
    m_connected = false;
    m_connecting = false;
    stopBreathAnimation();

    m_connectBtn->setEnabled(true);
    m_connectBtn->setText(tr("连接失败"));
    m_connectBtn->setProperty("state", "error");
    m_connectBtn->style()->unpolish(m_connectBtn);
    m_connectBtn->style()->polish(m_connectBtn);

    updateStatusIndicator("error");
    m_statusIndicator->setToolTip(tr("连接错误: ") + errorMsg);

    // 3秒后自动恢复为正常断开状态
    QTimer::singleShot(3000, this, [this]() {
        if (!m_connected && !m_connecting) {
            m_connectBtn->setText(tr("连接"));
            m_connectBtn->setProperty("state", "");
            m_connectBtn->style()->unpolish(m_connectBtn);
            m_connectBtn->style()->polish(m_connectBtn);
            updateStatusIndicator("disconnected");
            m_statusIndicator->setToolTip(tr("未连接"));
        }
    });
}

void SerialConfigPanel::setConnecting()
{
    m_connecting = true;
    updateStatusIndicator("connecting");
    m_statusIndicator->setToolTip(tr("正在连接..."));

    // 创建或复用透明度特效
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(
        m_statusIndicator->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_statusIndicator);
        m_statusIndicator->setGraphicsEffect(effect);
    }
    effect->setOpacity(1.0);

    // 销毁旧动画(如果存在)
    delete m_breathAnim;
    m_breathAnim = nullptr;

    // 构建呼吸动画: 顺序组 [0.3→1.0, 1500ms] + [1.0→0.3, 1500ms], 无限循环
    auto* group = new QSequentialAnimationGroup(this);

    // 上半周期: 0.3 → 1.0 (淡入)
    auto* fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setStartValue(0.3);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(1500);
    fadeIn->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeIn);

    // 下半周期: 1.0 → 0.3 (淡出)
    auto* fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.3);
    fadeOut->setDuration(1500);
    fadeOut->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeOut);

    group->setLoopCount(-1);  // 无限循环
    group->start(QAbstractAnimation::DeleteWhenStopped);
    m_breathAnim = group;
}

void SerialConfigPanel::updateStatusIndicator(const QString& state)
{
    m_statusIndicator->setProperty("state", state);
    m_statusIndicator->style()->unpolish(m_statusIndicator);
    m_statusIndicator->style()->polish(m_statusIndicator);
}

void SerialConfigPanel::stopBreathAnimation()
{
    if (m_breathAnim) {
        m_breathAnim->stop();
        delete m_breathAnim;
        m_breathAnim = nullptr;
    }
    // 重置透明度特效到完全不透明
    if (auto* effect = qobject_cast<QGraphicsOpacityEffect*>(
            m_statusIndicator->graphicsEffect())) {
        effect->setOpacity(1.0);
    }
}

// ---- 端口管理 ----

void SerialConfigPanel::refreshPorts()
{
    QString currentPort = m_portCombo->currentData().toString();
    m_portCombo->clear();

    auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        QString name = port.portName();
        QString desc = port.description();
        if (!desc.isEmpty()) m_portCombo->addItem(name + " - " + desc, name);
        else m_portCombo->addItem(name, name);
    }

    if (!currentPort.isEmpty()) {
        int idx = m_portCombo->findData(currentPort);
        if (idx >= 0) m_portCombo->setCurrentIndex(idx);
    }
    updateDriverInfo();
    updateConnectButtonState();
}

// ---- 配置读取 ----

bool SerialConfigPanel::isConnected() const { return m_connected; }
QString SerialConfigPanel::currentPortData() const { return m_portCombo->currentData().toString(); }
int SerialConfigPanel::currentBaudRate() const { return m_baudCombo->currentText().toInt(); }
int SerialConfigPanel::currentDataBitsIndex() const { return m_dataBitsCombo->currentIndex(); }
int SerialConfigPanel::currentParityIndex() const { return m_parityCombo->currentIndex(); }
int SerialConfigPanel::currentStopBitsIndex() const { return m_stopBitsCombo->currentIndex(); }
int SerialConfigPanel::currentFlowControlIndex() const { return m_flowControlCombo->currentIndex(); }
bool SerialConfigPanel::dtrEnabled() const { return m_dtrState; }
bool SerialConfigPanel::rtsEnabled() const { return m_rtsState; }

void SerialConfigPanel::restoreConfig(const QVariantMap& config)
{
    if (config.contains("portName")) {
        int idx = m_portCombo->findData(config["portName"].toString());
        if (idx >= 0) m_portCombo->setCurrentIndex(idx);
    }
    if (config.contains("baudRate"))
        m_baudCombo->setCurrentText(QString::number(config["baudRate"].toInt()));
    if (config.contains("dataBits") && config["dataBits"].toInt() >= 0
        && config["dataBits"].toInt() < m_dataBitsCombo->count())
        m_dataBitsCombo->setCurrentIndex(config["dataBits"].toInt());
    if (config.contains("parity") && config["parity"].toInt() >= 0
        && config["parity"].toInt() < m_parityCombo->count())
        m_parityCombo->setCurrentIndex(config["parity"].toInt());
    if (config.contains("stopBits") && config["stopBits"].toInt() >= 0
        && config["stopBits"].toInt() < m_stopBitsCombo->count())
        m_stopBitsCombo->setCurrentIndex(config["stopBits"].toInt());
    if (config.contains("flowControl") && config["flowControl"].toInt() >= 0
        && config["flowControl"].toInt() < m_flowControlCombo->count())
        m_flowControlCombo->setCurrentIndex(config["flowControl"].toInt());
    if (config.contains("dtr")) {
        m_dtrState = config["dtr"].toBool();
        m_dtrBtn->blockSignals(true);
        m_dtrBtn->setChecked(m_dtrState);
        m_dtrBtn->setText(m_dtrState ? tr("DTR HIGH") : tr("DTR LOW"));
        m_dtrBtn->blockSignals(false);
        refreshDtrStyle();
    }
    if (config.contains("rts")) {
        m_rtsState = config["rts"].toBool();
        m_rtsBtn->blockSignals(true);
        m_rtsBtn->setChecked(m_rtsState);
        m_rtsBtn->setText(m_rtsState ? tr("RTS HIGH") : tr("RTS LOW"));
        m_rtsBtn->blockSignals(false);
        refreshRtsStyle();
    }
}

// ---- 内部方法 ----

/** @brief 刷新DTR按钮视觉状态，通过QSS property驱动颜色切换 */
void SerialConfigPanel::refreshDtrStyle()
{
    m_dtrBtn->setProperty("signalState", m_dtrState ? "high" : "low");
    m_dtrBtn->style()->unpolish(m_dtrBtn);
    m_dtrBtn->style()->polish(m_dtrBtn);
}

/** @brief 刷新RTS按钮视觉状态，通过QSS property驱动颜色切换 */
void SerialConfigPanel::refreshRtsStyle()
{
    m_rtsBtn->setProperty("signalState", m_rtsState ? "high" : "low");
    m_rtsBtn->style()->unpolish(m_rtsBtn);
    m_rtsBtn->style()->polish(m_rtsBtn);
}

void SerialConfigPanel::updateDriverInfo()
{
    m_driverInfoLbl->setText(SerialDriverDetector::driverStatusSummary());
}

void SerialConfigPanel::onPortComboChanged() { updateConnectButtonState(); }

void SerialConfigPanel::updateConnectButtonState()
{
    if (!m_connected && !m_connecting) {
        bool hasPorts = m_portCombo->count() > 0;
        m_connectBtn->setEnabled(hasPorts);
        m_connectBtn->setText(hasPorts ? tr("连接") : tr("无端口"));
    }
}
