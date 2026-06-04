/**
 * @file SerialConfigPanel.cpp
 * @brief 串口配置面板实现 - 串口参数配置、连接控制和状态指示
 *
 * 面板布局: 端口选择 → 参数配置 → 控制信号 → 驱动检测 → 连接按钮+状态指示器
 * 状态指示器通过QSS的statusIndicator[state="xxx"]控制圆点颜色。
 *
 * UI构建方法已拆分至 SerialConfigPanelUI.cpp
 */

#include "serial/config/SerialConfigPanel.h"
#include "shared/AnimationConstants.h"
#include "shared/TimerConstants.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QTimer>

#include "serial/port/SerialDriverDetector.h"
#include "core/widgets/AnimatedButton.h"

// ---- 构造 ----

/** @brief 构造串口配置面板(端口/波特率/数据位/校验/停止位/流控/DTR/RTS) @param parent 父控件 */
SerialConfigPanel::SerialConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("serialConfigPanel");

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

    // 安全停止旧动画: 使用stop()触发DeleteWhenStopped自动清理，避免直接delete与DeleteWhenStopped冲突
    if (m_breathAnim) {
        m_breathAnim->stop();   // stop()触发DeleteWhenStopped -> deleteLater()
        m_breathAnim = nullptr;
    }

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
    ++m_totalRefreshPorts;
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

// 配置读取/恢复方法见 SerialConfigPanelConfig.cpp

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
void SerialConfigPanel::onPortComboChanged()
{
    // Statistics: count port switches when user selects a different port
    if (!m_portCombo->currentData().toString().isEmpty()) {
        m_totalPortSwitches++;
    }
    updateConnectButtonState();
}

/** @brief 根据端口选择状态更新连接按钮启用/禁用 */
void SerialConfigPanel::updateConnectButtonState()
{
    if (!m_connected && !m_connecting) {
        bool hasPorts = m_portCombo->count() > 0;
        m_connectBtn->setEnabled(hasPorts);
        m_connectBtn->setText(hasPorts ? tr("连接") : tr("无端口"));
    }
}

// buildPortTooltip见 SerialConfigPanelConfig.cpp

/** @brief 重置所有操作统计计数器(配置变更/波特率变更/端口切换/流控切换/刷新/连接尝试归零) */
void SerialConfigPanel::resetStats()
{
    m_totalConfigChanges = 0;
    m_totalBaudChanges = 0;
    m_totalPortSwitches = 0;
    m_totalFlowControlToggles = 0;
    m_totalRefreshPorts = 0;
    m_totalConnectAttempts = 0;
}

