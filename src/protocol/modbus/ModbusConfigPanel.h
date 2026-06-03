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
    quint64 m_totalConfigChanges = 0;  ///< 累计配置变更次数
    quint64 m_totalScanRequests = 0;   ///< 累计扫描请求次数
};

#endif // MODBUS_CONFIG_PANEL_H
