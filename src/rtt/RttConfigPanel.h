/**
 * @file RttConfigPanel.h
 * @brief RTT 配置面板 — J-Link RTT 连接参数配置 UI
 *
 * 提供设备选择、调试接口、连接速度、通道号等 RTT 参数的配置界面。
 * 用户修改配置后发出 configChanged 信号。
 *
 * 协作关系:
 *   - RttChannelManager: 使用配置创建通道
 *   - JLinkRttConnection: 读取配置建立连接
 */
#ifndef RTTCONFIGPANEL_H
#define RTTCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QVariantMap>

/**
 * @brief RTT 配置面板
 *
 * 包含设备选择、调试接口（JTAG/SWD）、连接速度、通道号等配置项。
 * 修改配置后发出 configChanged() 信号通知外部。
 */
class RttConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit RttConfigPanel(QWidget* parent = nullptr);

    /** @brief 获取当前配置参数 */
    QVariantMap config() const;

signals:
    /** @brief 配置参数变更信号 */
    void configChanged(const QVariantMap& config);

    /** @brief 用户点击连接按钮信号 */
    void connectRequested();

    /** @brief 用户点击断开按钮信号 */
    void disconnectRequested();

private:
    /** @brief 初始化 UI 布局和控件 */
    void setupUI();

    QComboBox* m_deviceCombo;     ///< 设备选择下拉框
    QComboBox* m_interfaceCombo;  ///< 调试接口选择（JTAG/SWD）
    QSpinBox* m_speedSpin;        ///< 连接速度 (kHz)
    QSpinBox* m_channelSpin;      ///< RTT 通道号
};

#endif // RTTCONFIGPANEL_H
