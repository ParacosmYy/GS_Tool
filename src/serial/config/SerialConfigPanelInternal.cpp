/**
 * @file SerialConfigPanelInternal.cpp
 * @brief 串口配置面板 - 内部辅助方法实现
 *
 * 从 SerialConfigPanel.cpp 拆分而来，包含:
 *   - updatePinoutLeds():     信号线状态LED指示灯更新
 *   - refreshSignalStyle():   信号按钮视觉状态刷新(QSS property驱动)
 *   - updateDriverInfo():     驱动检测信息显示(CH340/CP2102/FT232等)
 *   - onPortComboChanged():   端口选择变化处理(统计+按钮状态更新)
 *   - updateConnectButtonState(): 连接按钮启用/禁用状态管理
 *   - resetStats():           操作统计计数器重置
 */

#include "serial/config/SerialConfigPanel.h"
#include "serial/port/SerialDriverDetector.h"

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
