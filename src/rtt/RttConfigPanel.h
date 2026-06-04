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
#include <QSettings>
#include <QVariantMap>

/**
 * @brief RTT 配置面板
 *
 * 包含设备选择、调试接口（JTAG/SWD/cJTAG）、连接速度、通道号等配置项。
 * 修改配置后发出 configChanged() 信号通知外部。
 * 连接/断开按钮分别发出 connectRequested/disconnectRequested 信号。
 */
class RttConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父控件 */
    explicit RttConfigPanel(QWidget* parent = nullptr);

    /** @brief 获取当前配置参数 @return QVariantMap 包含device/interface/speed/channel字段 */
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

signals:
    /** @brief 配置参数变更信号 */
    void configChanged(const QVariantMap& config);

    /** @brief 用户点击连接按钮信号 */
    void connectRequested();

    /** @brief 用户点击断开按钮信号 */
    void disconnectRequested();

public:
    // ---- 统计接口 ----

    /** @brief RTT配置面板运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalConfigChanges = 0;       ///< 累计配置变更次数
        quint64 totalConnectRequests = 0;     ///< 累计连接请求次数
        quint64 totalDisconnectRequests = 0;  ///< 累计断开请求次数
        quint64 totalSettingsSaves = 0;       ///< 累计配置保存次数
        quint64 totalSettingsLoads = 0;       ///< 累计配置加载次数
        quint64 totalChannelCountChanges = 0; ///< 累计通道号变更次数
        quint64 totalSpeedChanges = 0;        ///< 累计速度变更次数
    };

    /** @brief 获取累计配置变更次数 */
    quint64 totalConfigChanges() const;

    /** @brief 获取累计连接请求次数 */
    quint64 totalConnectRequests() const;

    /** @brief 获取累计断开请求次数 */
    quint64 totalDisconnectRequests() const;

    /** @brief 获取累计配置保存次数 @return 保存次数 */
    quint64 totalSettingsSaves() const { return m_stats.totalSettingsSaves; }

    /** @brief 获取累计配置加载次数 @return 加载次数 */
    quint64 totalSettingsLoads() const { return m_stats.totalSettingsLoads; }

    /** @brief 获取累计通道号变更次数 @return 变更总次数 */
    quint64 totalChannelCountChanges() const { return m_stats.totalChannelCountChanges; }

    /** @brief 获取累计速度变更次数 @return 变更总次数 */
    quint64 totalSpeedChanges() const { return m_stats.totalSpeedChanges; }

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器归零 */
    void resetConfigStatistics();

private:
    /** @brief 初始化 UI 布局和控件 */
    void setupUI();

    /** @brief 表单值变化时的统一处理槽 */
    void onFormValueChanged();

    QComboBox* m_deviceCombo;     ///< 设备选择下拉框
    QComboBox* m_interfaceCombo;  ///< 调试接口选择（JTAG/SWD/cJTAG）
    QSpinBox* m_speedSpin;        ///< 连接速度 (kHz)
    QSpinBox* m_channelSpin;      ///< RTT 通道号

    // ---- 统计计数器 ----
    mutable Stats m_stats;    ///< 聚合统计结构体(mutable因const方法需修改)
};

#endif // RTTCONFIGPANEL_H
