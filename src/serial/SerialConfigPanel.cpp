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
#include <QCheckBox>
#include <QSpinBox>

#include "serial/SerialDriverDetector.h"
#include "core/AnimatedButton.h"

// ---- 构造 ----

SerialConfigPanel::SerialConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    refreshPorts();
    updateDriverInfo();
}

// ---- UI布局 ----
/** @brief 初始化串口配置面板UI(端口/波特率/数据位/校验/停止位/流控/DTR-RTS) */
void SerialConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    mainLayout->addWidget(createPortGroup());
    mainLayout->addWidget(createParamGroup());
    setupSignalAndConnectControls(mainLayout);
}

QGroupBox* SerialConfigPanel::createPortGroup()
{
    auto* portGroup = new QGroupBox(tr("端口"));
    portGroup->setObjectName("portGroup");
    auto* portLayout = new QHBoxLayout(portGroup);

    m_portCombo = new QComboBox;
    m_portCombo->setObjectName("portCombo");
    m_portCombo->setMinimumWidth(150);
    connect(m_portCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialConfigPanel::onPortComboChanged);

    m_refreshBtn = new AnimatedButton(tr("刷新"));
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setToolTip(tr("重新扫描系统中的串口设备"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &SerialConfigPanel::refreshPorts);

    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_refreshBtn);
    return portGroup;
}

QGroupBox* SerialConfigPanel::createParamGroup()
{
    auto* paramGroup = new QGroupBox(tr("参数"));
    paramGroup->setObjectName("paramGroup");
    auto* formLayout = new QFormLayout(paramGroup);

    m_baudCombo = new QComboBox;
    m_baudCombo->setObjectName("baudCombo");
    m_baudCombo->setEditable(true);
    m_baudCombo->setToolTip(tr("通信速率(比特/秒)，常用值: 9600, 115200\n可直接输入自定义波特率"));
    m_baudCombo->addItems({"1200","2400","4800","9600","19200","38400","57600","115200","230400","460800","921600","1000000"});
    m_baudCombo->setCurrentText("115200");
    m_baudCombo->lineEdit()->setValidator(new QIntValidator(300, 10000000, this));
    // 运行时波特率切换: 连接后用户修改波特率时通知上层
    connect(m_baudCombo, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        if (m_connected) {
            bool ok = false;
            qint32 baud = text.toInt(&ok);
            if (ok && baud > 0) {
                emit baudRateChanged(baud);
            }
        }
    });
    formLayout->addRow(tr("波特率:"), m_baudCombo);

    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->setObjectName("dataBitsCombo");
    m_dataBitsCombo->setToolTip(tr("每个数据帧的数据位数，绝大多数设备使用 8 位"));
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentIndex(3);
    formLayout->addRow(tr("数据位:"), m_dataBitsCombo);

    m_parityCombo = new QComboBox;
    m_parityCombo->setObjectName("parityCombo");
    m_parityCombo->setToolTip(tr("校验方式:\n无 - 不校验(最常用，适合短距离稳定通信)\n偶校验 - 数据位+校验位1的个数为偶数\n奇校验 - 数据位+校验位1的个数为奇数"));
    m_parityCombo->addItems({tr("无"), tr("偶校验"), tr("奇校验"), tr("Mark"), tr("Space")});
    formLayout->addRow(tr("校验位:"), m_parityCombo);

    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->setObjectName("stopBitsCombo");
    m_stopBitsCombo->setToolTip(tr("停止位数:\n1位 - 标准设置(绝大多数设备)\n1.5位 - 极少见\n2位 - 调制解调器/低速通信"));
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    formLayout->addRow(tr("停止位:"), m_stopBitsCombo);

    m_flowControlCombo = new QComboBox;
    m_flowControlCombo->setObjectName("flowControlCombo");
    m_flowControlCombo->setToolTip(tr("流量控制:\n无 - 不使用流控(最常用，短距离无需流控)\nRTS/CTS - 硬件流控(需额外2根信号线，高速通信推荐)\nXON/XOFF - 软件流控(XOFF=0x13暂停, XON=0x11恢复)"));
    m_flowControlCombo->addItems({tr("无"), tr("RTS/CTS"), tr("XON/XOFF")});
    formLayout->addRow(tr("流控:"), m_flowControlCombo);

    return paramGroup;
}

/**
 * @brief 构建控制信号(DTR/RTS)、驱动检测信息和连接按钮区域
 *
 * 从 setupUI() 拆分出来，避免单个方法超过80行限制。
 * 包含: 控制信号GroupBox → 驱动检测信息 → 连接按钮+状态指示器
 */
/** @brief 连接信号槽并组装底部控制区域(连接按钮+DTR-RTS+自动重连) @param mainLayout 主布局 */
void SerialConfigPanel::setupSignalAndConnectControls(QVBoxLayout* mainLayout)
{
    // ---- 控制信号 (DTR/RTS) ----
    mainLayout->addWidget(createControlSignalsGroup());

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

    m_connectBtn = new AnimatedButton(tr("连接"));
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
    mainLayout->addLayout(createAutoReconnectLayout());
}

/** @brief 创建DTR/RTS控制信号分组(含按钮、工具提示、信号连接和视觉刷新) */
QGroupBox* SerialConfigPanel::createControlSignalsGroup()
{
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

    m_breakBtn = new QPushButton(tr("BRK"));
    m_breakBtn->setObjectName("breakBtn");
    m_breakBtn->setToolTip(tr("发送Break信号(用于STM32/ESP32进入Bootloader)"));
    m_breakBtn->setEnabled(false);
    connect(m_breakBtn, &QPushButton::clicked, this, [this]() { emit breakRequested(100); });

    signalLayout->addWidget(m_dtrBtn);
    signalLayout->addWidget(m_rtsBtn);
    signalLayout->addWidget(m_breakBtn);

    // ---- 输入信号线状态LED ----
    auto makeLed = [](const char* n) -> QLabel* {
        auto* l = new QLabel(n); l->setObjectName("signalLed");
        l->setAlignment(Qt::AlignCenter); l->setFixedSize(36, 20);
        l->setProperty("active", false); return l;
    };
    m_ctsLed = makeLed("CTS"); m_dsrLed = makeLed("DSR");
    m_dcdLed = makeLed("DCD"); m_riLed = makeLed("RI");
    for (auto* w : {m_ctsLed, m_dsrLed, m_dcdLed, m_riLed}) signalLayout->addWidget(w);
    signalLayout->addStretch();

    connect(m_dtrBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_dtrState = checked;
        m_dtrBtn->setText(checked ? tr("DTR HIGH") : tr("DTR LOW"));
        refreshSignalStyle(m_dtrBtn, m_dtrState);
        emit dtrChanged(checked);
    });
    connect(m_rtsBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_rtsState = checked;
        m_rtsBtn->setText(checked ? tr("RTS HIGH") : tr("RTS LOW"));
        refreshSignalStyle(m_rtsBtn, m_rtsState);
        emit rtsChanged(checked);
    });
    // 初始化视觉状态为默认HIGH
    refreshSignalStyle(m_dtrBtn, m_dtrState);
    refreshSignalStyle(m_rtsBtn, m_rtsState);
    return signalGroup;
}

// ---- 状态管理 ----

/** @brief 设置连接状态(更新按钮文字、启用/禁用配置控件) @param connected true=已连接 */
void SerialConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connecting = false;

    m_connectBtn->setEnabled(true);
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_connectBtn->setProperty("state", connected ? "connected" : "");
    m_connectBtn->style()->unpolish(m_connectBtn);
    m_connectBtn->style()->polish(m_connectBtn);

    // 锁定/解锁配置控件(波特率保持可编辑，支持运行时切换)
    m_portCombo->setEnabled(!connected);
    for (auto* w : {m_dataBitsCombo, m_parityCombo, m_stopBitsCombo, m_flowControlCombo})
        static_cast<QWidget*>(w)->setEnabled(!connected);
    m_refreshBtn->setEnabled(!connected);
    m_dtrBtn->setEnabled(connected); m_rtsBtn->setEnabled(connected); m_breakBtn->setEnabled(connected);

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

/** @brief 显示错误状态(红色指示器+错误信息，停止呼吸动画) @param errorMsg 错误描述 */
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

/** @brief 设置连接中状态(黄色指示器+呼吸动画+按钮禁用) */
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

    // 销毁旧动画
    delete m_breathAnim;

    // 用lambda创建呼吸动画半周期(0.3↔1.0, 1500ms, InOutSine缓动)
    auto makeFade = [effect](qreal from, qreal to) -> QPropertyAnimation* {
        auto* a = new QPropertyAnimation(effect, "opacity");
        a->setStartValue(from); a->setEndValue(to);
        a->setDuration(1500); a->setEasingCurve(QEasingCurve::InOutSine); return a;
    };
    auto* group = new QSequentialAnimationGroup(this);
    group->addAnimation(makeFade(0.3, 1.0));
    group->addAnimation(makeFade(1.0, 0.3));
    group->setLoopCount(-1);
    group->start(QAbstractAnimation::DeleteWhenStopped);
    m_breathAnim = group;
}

/** @brief 根据状态名更新指示器颜色(connected=绿,connecting=黄,error=红,disconnected=灰) @param state 状态字符串 */
void SerialConfigPanel::updateStatusIndicator(const QString& state)
{
    m_statusIndicator->setProperty("state", state);
    m_statusIndicator->style()->unpolish(m_statusIndicator);
    m_statusIndicator->style()->polish(m_statusIndicator);
}

/** @brief 停止连接状态呼吸动画(连接成功或失败时调用) */
void SerialConfigPanel::stopBreathAnimation()
{
    if (m_breathAnim) { m_breathAnim->stop(); delete m_breathAnim; m_breathAnim = nullptr; }
    if (auto* effect = qobject_cast<QGraphicsOpacityEffect*>(m_statusIndicator->graphicsEffect()))
        effect->setOpacity(1.0);
}

// ---- 端口管理 ----
/** @brief 刷新串口端口列表(枚举系统可用端口，含VID/PID/描述/制造商信息) */
void SerialConfigPanel::refreshPorts()
{
    QString cur = m_portCombo->currentData().toString();
    m_portCombo->clear();
    for (const auto& p : QSerialPortInfo::availablePorts()) {
        QString name = p.portName();
        // 格式: "COM3 - CH340 (VID:1A86 PID:7523)" 或 "COM3"
        QString display = p.description().isEmpty() ? name : QString("%1 - %2").arg(name, p.description());
        if (p.hasVendorIdentifier() || p.hasProductIdentifier()) {
            QStringList ids;
            if (p.hasVendorIdentifier())
                ids << QString("VID:%1").arg(p.vendorIdentifier(), 4, 16, QLatin1Char('0')).toUpper();
            if (p.hasProductIdentifier())
                ids << QString("PID:%1").arg(p.productIdentifier(), 4, 16, QLatin1Char('0')).toUpper();
            display += " (" + ids.join(" ") + ")";
        }
        m_portCombo->addItem(display, name);
        m_portCombo->setItemData(m_portCombo->count() - 1, buildPortTooltip(p), Qt::ToolTipRole);
    }
    if (!cur.isEmpty()) { if (int i = m_portCombo->findData(cur); i >= 0) m_portCombo->setCurrentIndex(i); }
    updateDriverInfo();
    updateConnectButtonState();
}

// ---- 配置读取 ----
/** @brief 返回当前是否已连接 @return true=已连接 */
bool SerialConfigPanel::isConnected() const { return m_connected; }
/** @brief 返回当前选中端口的系统路径(COMn) @return 端口路径字符串 */
QString SerialConfigPanel::currentPortData() const { return m_portCombo->currentData().toString(); }
/** @brief 返回当前选中的波特率 @return 波特率数值 */
int SerialConfigPanel::currentBaudRate() const { return m_baudCombo->currentText().toInt(); }
/** @brief 返回当前选中的数据位索引 @return ComboBox索引 */
int SerialConfigPanel::currentDataBitsIndex() const { return m_dataBitsCombo->currentIndex(); }
/** @brief 返回当前选中的校验位索引 @return ComboBox索引 */
int SerialConfigPanel::currentParityIndex() const { return m_parityCombo->currentIndex(); }
/** @brief 返回当前选中的停止位索引 @return ComboBox索引 */
int SerialConfigPanel::currentStopBitsIndex() const { return m_stopBitsCombo->currentIndex(); }
/** @brief 返回当前选中的流控索引 @return ComboBox索引 */
int SerialConfigPanel::currentFlowControlIndex() const { return m_flowControlCombo->currentIndex(); }
/** @brief 返回DTR信号当前状态 @return true=DTR高电平 */
bool SerialConfigPanel::dtrEnabled() const { return m_dtrState; }
/** @brief 返回RTS信号当前状态 @return true=RTS高电平 */
bool SerialConfigPanel::rtsEnabled() const { return m_rtsState; }

/** @brief 从配置映射恢复串口参数(端口/波特率/数据位/校验/停止位/流控/DTR/RTS/自动重连) @param config 配置映射 */
void SerialConfigPanel::restoreConfig(const QVariantMap& config)
{
    if (config.contains("portName")) {
        int idx = m_portCombo->findData(config["portName"].toString());
        if (idx >= 0) m_portCombo->setCurrentIndex(idx);
    }
    if (config.contains("baudRate"))
        m_baudCombo->setCurrentText(QString::number(config["baudRate"].toInt()));
    // 通用 ComboBox 索引恢复(dataBits/parity/stopBits/flowControl)
    auto setIdx = [this, &config](QComboBox* cb, const QString& key) {
        if (config.contains(key)) { int v = config[key].toInt();
            if (v >= 0 && v < cb->count()) cb->setCurrentIndex(v); }
    };
    setIdx(m_dataBitsCombo, "dataBits"); setIdx(m_parityCombo, "parity");
    setIdx(m_stopBitsCombo, "stopBits"); setIdx(m_flowControlCombo, "flowControl");
    // 恢复 DTR/RTS 信号状态(通用 lambda 避免重复 blockSignals 模式)
    auto restoreSig = [&](const QString& key, bool& state, QPushButton* btn, const char* hi, const char* lo) {
        if (!config.contains(key)) return;
        state = config[key].toBool(); btn->blockSignals(true);
        btn->setChecked(state); btn->setText(state ? tr(hi) : tr(lo));
        btn->blockSignals(false); refreshSignalStyle(btn, state);
    };
    restoreSig("dtr", m_dtrState, m_dtrBtn, "DTR HIGH", "DTR LOW");
    restoreSig("rts", m_rtsState, m_rtsBtn, "RTS HIGH", "RTS LOW");
}

// ---- 内部方法 ----
/** @brief 更新信号线状态LED指示灯
 *  @param pinSignals 当前信号线电平状态
 */
void SerialConfigPanel::updatePinoutLeds(const PinoutSignals& pinSignals)
{
    auto updateLed = [](QLabel* led, bool active) {
        if (!led) return;
        led->setProperty("active", active);
        led->style()->unpolish(led); led->style()->polish(led);
    };
    updateLed(m_ctsLed, pinSignals.cts); updateLed(m_dsrLed, pinSignals.dsr);
    updateLed(m_dcdLed, pinSignals.dcd); updateLed(m_riLed, pinSignals.ri);
}

/** @brief 刷新信号按钮视觉状态，通过QSS property驱动颜色切换 */
void SerialConfigPanel::refreshSignalStyle(QPushButton* btn, bool high)
{
    btn->setProperty("signalState", high ? "high" : "low");
    btn->style()->unpolish(btn);
    btn->style()->polish(btn);
}

/** @brief 检测并显示当前端口的驱动信息(CH340/CP2102/FT232等) */
void SerialConfigPanel::updateDriverInfo()
{
    m_driverInfoLbl->setText(SerialDriverDetector::driverStatusSummary());
}

/** @brief 端口ComboBox选中变化时更新连接按钮启用状态 */
void SerialConfigPanel::onPortComboChanged() { updateConnectButtonState(); }

/** @brief 根据端口选择状态更新连接按钮启用/禁用 */
void SerialConfigPanel::updateConnectButtonState()
{
    if (!m_connected && !m_connecting) {
        bool hasPorts = m_portCombo->count() > 0;
        m_connectBtn->setEnabled(hasPorts);
        m_connectBtn->setText(hasPorts ? tr("连接") : tr("无端口"));
    }
}

/**
 * @brief 构建端口详情tooltip(VID/PID/制造商/序列号/系统路径/推荐波特率)
 * @param info QSerialPortInfo端口信息
 * @return 多行tooltip字符串，包含端口名/描述/制造商/VID/PID/序列号/系统路径/常用波特率
 */
/** @brief 构建端口tooltip(端口名+描述+制造商+VID/PID+驱动芯片) @param info 串口信息 @return HTML格式tooltip */
QString SerialConfigPanel::buildPortTooltip(const QSerialPortInfo& info) const
{
    QStringList details;
    details << tr("端口: %1").arg(info.portName());
    if (!info.description().isEmpty())
        details << tr("描述: %1").arg(info.description());
    if (!info.manufacturer().isEmpty())
        details << tr("制造商: %1").arg(info.manufacturer());
    if (info.hasVendorIdentifier())
        details << tr("VID: %1").arg(info.vendorIdentifier(), 4, 16, QLatin1Char('0')).toUpper();
    if (info.hasProductIdentifier())
        details << tr("PID: %1").arg(info.productIdentifier(), 4, 16, QLatin1Char('0')).toUpper();
    if (!info.serialNumber().isEmpty())
        details << tr("序列号: %1").arg(info.serialNumber());
    // 系统路径: Linux下为/dev/ttyUSB0等，Windows下为\\?\USB#VID_xxxx&PID_xxxx...完整设备路径
    if (!info.systemLocation().isEmpty())
        details << tr("系统路径: %1").arg(info.systemLocation());
    // 常用波特率提示
    details << tr("常用波特率: 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600");
    return details.join("\n");
}

/** @brief 创建自动重连控件布局(复选框+间隔微调框, 范围500~30000ms, 步进500ms, 默认3000ms) */
QHBoxLayout* SerialConfigPanel::createAutoReconnectLayout()
{
    auto* lay = new QHBoxLayout;
    m_autoReconnectCheck = new QCheckBox(tr("自动重连"));
    m_autoReconnectCheck->setObjectName("autoReconnectCheck");
    m_reconnectIntervalSpin = new QSpinBox;
    m_reconnectIntervalSpin->setObjectName("reconnectIntervalSpin");
    m_reconnectIntervalSpin->setRange(500, 30000);
    m_reconnectIntervalSpin->setSingleStep(500);
    m_reconnectIntervalSpin->setValue(3000);
    m_reconnectIntervalSpin->setSuffix("ms");
    m_reconnectIntervalSpin->setEnabled(false);
    lay->addWidget(m_autoReconnectCheck);
    QLabel* intervalLbl = new QLabel(tr("间隔"));
    intervalLbl->setObjectName("reconnectIntervalLabel");
    lay->addWidget(intervalLbl);
    lay->addWidget(m_reconnectIntervalSpin);
    connect(m_autoReconnectCheck, &QCheckBox::toggled, this, [this](bool on) {
        m_reconnectIntervalSpin->setEnabled(on);
        emit autoReconnectToggled(on, m_reconnectIntervalSpin->value());
    });
    connect(m_reconnectIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) {
        if (m_autoReconnectCheck->isChecked()) emit autoReconnectToggled(true, v);
    });
    return lay;
}
