/**
 * @file SerialConfigPanelConfig.cpp
 * @brief 串口配置面板 — 配置读取、恢复和端口详情tooltip实现
 *
 * 从 SerialConfigPanel.cpp 拆分而来，包含配置项getter、restoreConfig、
 * buildPortTooltip等方法。
 */

#include "serial/config/SerialConfigPanel.h"

#include <QSerialPortInfo>

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

/** @brief 构建端口详情tooltip(端口名+描述+制造商+VID/PID+序列号+系统路径+常用波特率) @param info 串口信息 @return 多行tooltip字符串 */
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
