/**
 * @file ScopeChannelManager.h
 * @brief 示波器通道管理器 -- 多通道配置/缩放/单位管理
 *
 * 管理示波器(ScopeWidget)的通道配置，包括通道名称、颜色、Y轴范围、
 * 单位、耦合模式、缩放模式、反相、探头比、偏移等参数。
 * 将通道管理逻辑从 ScopeWidget 渲染代码中分离，实现职责单一化。
 *
 * 协作关系:
 *   - ScopeWidget: 通过 channels()/visibleChannels() 获取通道配置进行渲染
 *   - ScopeConfigPanel(未来): 提供UI管理通道配置
 *   - DataAggregator: 通过 autoScale() 接收数据进行自动缩放
 */
#ifndef SCOPECHANNELMANAGER_H
#define SCOPECHANNELMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QVector>
#include <QColor>
#include <QString>

/**
 * @brief 示波器通道管理器
 *
 * 支持最多8个独立通道的配置管理。每个通道拥有独立的名称、颜色、
 * Y轴范围、物理单位、耦合模式、缩放模式等参数。
 * 提供基于数据的自动缩放功能(10%边距)和JSON格式的配置导入/导出。
 */
class ScopeChannelManager : public QObject {
    Q_OBJECT

public:
    /** @brief 耦合模式枚举，对应硬件示波器的输入耦合方式 */
    enum class CouplingMode {
        DC,   ///< 直流耦合 -- 显示信号直流+交流分量
        AC,   ///< 交流耦合 -- 隔离直流分量，仅显示交流
        GND   ///< 接地耦合 -- 输入接地，显示零基线
    };
    Q_ENUM(CouplingMode)

    /** @brief 缩放模式枚举，控制Y轴范围的调整策略 */
    enum class ScaleMode {
        Auto,   ///< 自动缩放 -- 根据数据范围自动调整
        Manual, ///< 手动缩放 -- 用户指定固定范围
        Fit     ///< 适应缩放 -- 自动缩放但保留一定余量
    };
    Q_ENUM(ScaleMode)

    /**
     * @brief 单个通道的完整配置
     */
    struct ChannelConfig {
        int index = -1;                  ///< 通道索引(由管理器分配，唯─标识)
        QString name;                    ///< 通道显示名称，如 "CH1", "PA0"
        QColor color = QColor("#00FF00");///< 通道波形颜色
        double yMin = -5.0;              ///< Y轴下限(对应单位值)
        double yMax = 5.0;               ///< Y轴上限(对应单位值)
        QString unit = QStringLiteral("V"); ///< 物理单位 ("V"/"A"/"mV"/"mA"/custom)
        CouplingMode coupling = CouplingMode::DC; ///< 输入耦合模式
        ScaleMode scaleMode = ScaleMode::Auto;     ///< 缩放模式
        bool visible = true;             ///< 通道是否可见(参与渲染)
        bool inverted = false;           ///< 波形是否反相(上下翻转)
        double probeRatio = 1.0;         ///< 探头衰减比 (1x/10x/100x)
        double offset = 0.0;             ///< Y轴偏移量(垂直位移)
        int lineWidth = 2;               ///< 波形线宽(像素)
    };

    /**
     * @brief 通道管理器运行统计数据结构体，聚合全部运行期间计数器
     */
    struct Stats {
        quint64 totalChannelAdds = 0;          ///< 累计添加通道次数
        quint64 totalChannelRemoves = 0;       ///< 累计移除通道次数
        quint64 totalRangeChanges = 0;         ///< 累计Y轴范围变更次数
        quint64 totalVisibilityToggles = 0;    ///< 累计可见性切换次数
        quint64 totalColorChanges = 0;         ///< 累计颜色变更次数
        quint64 totalScaleOperations = 0;      ///< 累计缩放操作次数(含autoScale)
        int peakChannelCount = 0;              ///< 历史最高通道数
        int activeChannelCount = 0;            ///< 当前活跃通道数
    };

    /**
     * @brief 构造函数
     * @param maxChannels 最大通道数量上限(默认8)
     * @param parent 父QObject
     */
    explicit ScopeChannelManager(int maxChannels = 8, QObject* parent = nullptr);

    /** @brief 添加一个新通道，返回分配的索引，失败返回-1 @param config 通道配置(无需填index) @return 分配到的通道索引 */
    int addChannel(const ChannelConfig& config);

    /** @brief 移除指定索引的通道 @param index 通道索引 @return 成功返回true */
    bool removeChannel(int index);

    /** @brief 更新指定通道的完整配置 @param index 通道索引 @param config 新配置 @return 成功返回true */
    bool updateChannel(int index, const ChannelConfig& config);

    /** @brief 获取指定通道的配置 @param index 通道索引 @return 通道配置副本 */
    ChannelConfig channel(int index) const;

    /** @brief 获取所有通道配置(按索引排序) @return 通道配置列表 */
    QList<ChannelConfig> channels() const;

    /** @brief 获取当前通道总数 @return 通道数量 */
    int channelCount() const;

    /** @brief 获取最大通道数上限 @return 最大通道数 */
    int maxChannels() const;

    /** @brief 设置通道可见性 @param index 通道索引 @param visible 是否可见 */
    void setChannelVisible(int index, bool visible);

    /** @brief 设置通道颜色 @param index 通道索引 @param color 新颜色 */
    void setChannelColor(int index, const QColor& color);

    /** @brief 设置通道Y轴范围 @param index 通道索引 @param yMin 下限 @param yMax 上限 */
    void setChannelRange(int index, double yMin, double yMax);

    /** @brief 设置通道物理单位 @param index 通道索引 @param unit 单位字符串 */
    void setChannelUnit(int index, const QString& unit);

    /**
     * @brief 根据数据自动缩放指定通道Y轴范围(10%边距)
     * @param index 通道索引
     * @param data 采样数据向量
     */
    void autoScale(int index, const QVector<double>& data);

    /**
     * @brief 对多个通道批量自动缩放
     * @param data 通道索引→采样数据的映射表
     */
    void autoScaleAll(const QMap<int, QVector<double>>& data);

    /** @brief 重置所有通道的Y轴偏移为零 */
    void resetAllOffsets();

    /** @brief 获取所有可见通道的索引列表 @return 可见通道索引列表 */
    QList<int> visibleChannels() const;

    /**
     * @brief 将所有通道配置导出为JSON文件
     * @param filePath 目标文件路径
     * @return 成功返回true
     */
    bool exportConfig(const QString& filePath);

    /**
     * @brief 从JSON文件导入通道配置(追加到现有通道)
     * @param filePath 源文件路径
     * @return 成功返回true
     */
    bool importConfig(const QString& filePath);

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 通道已添加 @param index 新通道索引 */
    void channelAdded(int index);

    /** @brief 通道已移除 @param index 被移除通道的索引 */
    void channelRemoved(int index);

    /** @brief 通道配置已更新 @param index 通道索引 */
    void channelUpdated(int index);

    /** @brief 通道可见性变更 @param index 通道索引 @param visible 新可见状态 */
    void channelVisibilityChanged(int index, bool visible);

    /** @brief 通道Y轴范围变更 @param index 通道索引 @param yMin 新下限 @param yMax 新上限 */
    void channelRangeChanged(int index, double yMin, double yMax);

private:
    QMap<int, ChannelConfig> m_channels;  ///< 通道索引→配置映射表
    int m_maxChannels;                     ///< 最大通道数量上限
    int m_nextIndex;                       ///< 下一个可分配的通道索引(单调递增)
    Stats m_stats;                         ///< 聚合统计结构体
};

#endif // SCOPECHANNELMANAGER_H
