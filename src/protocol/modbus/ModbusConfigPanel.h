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
};

#endif // MODBUS_CONFIG_PANEL_H
