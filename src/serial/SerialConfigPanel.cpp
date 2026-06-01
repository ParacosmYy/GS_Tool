/**
 * @file SerialConfigPanel.cpp
 * @brief 串口配置面板实现 - 串口参数配置、连接控制和状态指示
 *
 * 面板布局: 端口选择 → 参数配置 → 控制信号 → 驱动检测 → 连接按钮+状态指示器
 * 状态指示器通过QSS的statusIndicator[state="xxx"]控制圆点颜色。
 *
 * UI构建方法已拆分至 SerialConfigPanelUI.cpp
 */

#include "serial/SerialConfigPanel.h"
#include "core/Constants.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QTimer>

#include "serial/SerialDriverDetector.h"
#include "core/AnimatedButton.h"

// ---- 构造 ----

/** @brief 构造串口配置面板(端口/波特率/数据位/校验/停止位/流控/DTR/RTS) @param parent 父控件 */
SerialConfigPanel::SerialConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    refreshPorts();
    updateDriverInfo();
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
    QTimer::singleShot(Timers::kConnectFailedDisplayMs, this, [this]() {
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
        a->setDuration(Animations::kConnPulseMs); a->setEasingCurve(QEasingCurve::InOutSine); return a;
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
    // DeleteWhenStopped会在stop()后自动deleteLater()，不再手动delete防止双重释放
    if (m_breathAnim) { m_breathAnim->stop(); m_breathAnim = nullptr; }
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
