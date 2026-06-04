/**
 * @file ModbusConfigPanel.h
 * @brief Modbus配置面板 — 配置Modbus通信参数
 *
 * 提供模式选择（RTU/ASCII/TCP）、从站地址、超时时间等参数配置。
 */
#ifndef MODBUS_CONFIG_PANEL_H
#define MODBUS_CONFIG_PANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QSettings>
#include <QVariantMap>

/**
 * @brief Modbus配置面板控件
 * 配置Modbus RTU/ASCII/TCP模式的通信参数。
 */
class ModbusConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造Modbus配置面板 @param parent 父控件 */
    explicit ModbusConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 获取当前配置参数
     * @return 包含 mode, slaveAddress, timeout 的参数映射
     */
    QVariantMap config() const;

    /** @brief 获取累计配置变更次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }

    /** @brief 获取累计扫描请求次数 */
    quint64 totalScanRequests() const { return m_totalScanRequests; }

    /** @brief 获取累计传输模式切换次数(RTU/ASCII/TCP) */
    quint64 totalModeSwitches() const { return m_totalModeSwitches; }

    /** @brief 获取累计从站地址变更次数 */
    quint64 totalSlaveAddressChanges() const { return m_totalSlaveAddressChanges; }

    /** @brief 获取累计超时时间变更次数 */
    quint64 totalTimeoutChanges() const { return m_totalTimeoutChanges; }

    /** @brief 获取累计配置保存次数 */
    quint64 totalConfigSaves() const { return m_totalConfigSaves; }

    /** @brief 获取累计配置加载次数 */
    quint64 totalConfigLoads() const { return m_totalConfigLoads; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

    /**
     * @brief 保存配置到QSettings
     * @param settings QSettings对象
     */
    void saveSettings(QSettings& settings) const;

    /**
     * @brief 从QSettings加载配置
     * @param settings QSettings对象
     */
    void loadSettings(QSettings& settings);

private:
    QComboBox* m_modeCombo    = nullptr; ///< 模式选择 (RTU/ASCII/TCP)
    QSpinBox*  m_slaveSpin    = nullptr; ///< 从站地址 (1-247)
    QSpinBox*  m_timeoutSpin  = nullptr; ///< 超时时间 (ms)

    // ---- 统计计数器 ----
    quint64 m_totalConfigChanges = 0;       ///< 累计配置变更次数
    quint64 m_totalScanRequests = 0;        ///< 累计扫描请求次数
    quint64 m_totalModeSwitches = 0;        ///< 累计传输模式切换次数
    quint64 m_totalSlaveAddressChanges = 0; ///< 累计从站地址变更次数
    quint64 m_totalTimeoutChanges = 0;      ///< 累计超时时间变更次数
    quint64 m_totalConfigSaves = 0;         ///< 累计配置保存次数
    quint64 m_totalConfigLoads = 0;         ///< 累计配置加载次数
};

#endif // MODBUS_CONFIG_PANEL_H
