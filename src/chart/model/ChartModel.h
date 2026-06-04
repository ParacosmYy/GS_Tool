/** @file ChartModel.h @brief 图表数据模型 -- 滑动窗口+降采样的实时波形数据管理。支持暂停/继续/缩放/通道动态增减。数据层: 不依赖表现层 */
#ifndef CHARTMODEL_H
#define CHARTMODEL_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPointF>
#include <QVariantMap>
#include <QTimer>
#include <QStringList>
#include <QPair>
#include <QDateTime>

#include "chart/model/ChannelConfig.h"

/** @brief 图表数据模型。接收帧解析结果，按ChannelConfig分发到各通道缓冲区，维护滑动窗口和降采样。数据层，不依赖表现层。协作: ChartWidget/ChannelConfigSet/FrameParser */
class ChartModel : public QObject {
    Q_OBJECT

public:
    /** @brief 构造图表数据模型 @param parent 父对象 */
    explicit ChartModel(QObject* parent = nullptr);
    // ---- 配置 ----
    /** @brief 设置通道配置集(重建缓冲区) @param configSet 通道配置集 */
    void setChannelConfigSet(const ChannelConfigSet& configSet);
    /** @brief 获取当前通道配置集 @return 配置集常引用 */
    const ChannelConfigSet& channelConfigSet() const;
    /** @brief 设置滑动窗口大小 @param points 最大可见数据点数 */
    void setWindowSize(int points);
    /** @brief 获取滑动窗口大小 @return 最大可见数据点数 */
    int windowSize() const;
    /** @brief 设置刷新间隔 @param ms 毫秒数，0=每次数据立即刷新 */
    void setRefreshInterval(int ms);
    /** @brief 获取刷新间隔 @return 毫秒数 */
    int refreshInterval() const;

    // ---- 数据查询 ----
    /** @brief 获取指定通道的可见数据点 @param displayName 通道名称 @return 数据点向量 */
    QVector<QPointF> channelData(const QString& displayName) const;
    /** @brief 获取所有启用通道的数据 @return 通道名到数据点集的映射 */
    QMap<QString, QVector<QPointF>> allChannelData() const;
    /** @brief 获取指定通道的Y值范围 @param displayName 通道名称 @return (min, max)对 */
    QPair<double, double> channelYRange(const QString& displayName) const;
    /** @brief 获取全局Y值范围(所有通道) @return (min, max)对 */
    QPair<double, double> globalYRange() const;
    /** @brief 获取所有通道名称列表 @return 名称列表 */
    QStringList channelNames() const;
    /** @brief 获取当前X轴范围 @return (min, max)对 */
    QPair<double, double> xRange() const;
    /** @brief 获取总接收数据点数 @return 累计点数 */
    qint64 totalPointsReceived() const;
    /** @brief 获取当前帧索引(X轴计数器) @return 帧索引 */
    qint64 currentFrameIndex() const;

    // ---- 统计信息 ----
    /** @brief 获取跨所有通道的数据点总数 @return 数据点总数 */
    quint64 totalDataPoints() const;
    /** @brief 获取历史创建通道总数(累计) @return 创建总数 */
    quint64 channelsCreated() const;
    /** @brief 获取历史移除通道总数(累计) @return 移除总数 */
    quint64 channelsRemoved() const;
    /** @brief 获取当前活跃通道数(实时) @return 活跃通道数 */
    quint64 totalChannelsActive() const;
    /** @brief 获取单通道最大数据点数(峰值) @return 峰值数据点数 */
    quint64 maxDataPointsInChannel() const;
    /** @brief 获取峰值数据速率 @return 数据点/秒 */
    double peakDataRate() const;
    /** @brief 重置图表统计(不影响通道数据和配置) */
    void resetChartStatistics();
    /** @brief 别名，调用resetChartStatistics */
    void resetStats();

    /** @brief 获取数据更新信号发射总次数 @return 更新信号次数 */
    quint64 totalDataUpdates() const;
    /** @brief 获取数据清空操作总次数 @return 清空次数 */
    quint64 totalClears() const;
    /** @brief 获取配置变更总次数(通道配置集更换) @return 配置变更次数 */
    quint64 totalConfigChanges() const;
    /** @brief 获取窗口大小变更总次数 @return 窗口大小变更次数 */
    quint64 totalWindowResizes() const;

    // ---- 操作 ----
    /** @brief 添加通道(更新统计) @param config 通道配置 */
    void addChannel(const ChannelConfig& config);
    /** @brief 移除通道(更新统计) @param displayName 通道名称 */
    void removeChannel(const QString& displayName);
    /** @brief 添加数据点(含统计) @param displayName 通道名称 @param value 数据值 */
    void addDataPoint(const QString& displayName, double value);
    /** @brief 清除所有通道数据 */
    void clear();

signals:
    /** @brief 通道数据更新通知 @param updatedChannels 已更新的通道列表 */
    void dataUpdated(const QStringList& updatedChannels);
    /** @brief 通道配置变更通知 */
    void channelsChanged();
    /** @brief 全部数据已清除 */
    void dataCleared();

public slots:
    /** @brief 接收帧解析结果 @param fields 解析后的字段映射 @param rawFrame 原始帧字节 */
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

private slots:
    /** @brief 定时刷新(合并高频更新) */
    void onRefreshTick();

private:
    /** @brief 单通道内部缓冲区 */
    struct ChannelBuffer { QVector<QPointF> points; int sampleCounter = 0; };
    /** @brief 重建所有通道缓冲区 */
    void rebuildBuffers();
    /** @brief 追加数据点(含降采样) @param displayName 通道名称 @param value 数据值 @param sampleDivisor 降采样比率 */
    void appendPoint(const QString& displayName, double value, int sampleDivisor);
    /** @brief 打包发射dataUpdated并清空待刷新列表 */
    void flushPendingUpdates();

    ChannelConfigSet m_configSet;               ///< 当前通道配置集合
    QMap<QString, ChannelBuffer> m_buffers;     ///< 通道名 → 数据缓冲区映射
    int m_windowSize = 200;                     ///< 滑动窗口大小（最大可见数据点数）
    qint64 m_frameIndex = 0;                    ///< 全局帧计数器（X轴）
    qint64 m_totalPoints = 0;                   ///< 总数据点计数（qint64防溢出）

    // 统计计数器
    quint64 m_totalDataPoints = 0;          ///< 跨所有通道添加的数据点总数
    quint64 m_channelsCreated = 0;          ///< 历史创建的通道总数（累计）
    quint64 m_channelsRemoved = 0;          ///< 历史移除的通道总数（累计）
    quint64 m_totalChannelsActive = 0;      ///< 当前活跃通道数（实时）
    quint64 m_maxDataPointsInChannel = 0;   ///< 单通道内出现过的最大数据点数
    double m_peakDataRate = 0.0;            ///< 峰值数据速率（数据点/秒）
    qint64 m_dataRateTimestamp = 0;         ///< 速率计算用的上次时间戳（毫秒纪元）
    quint64 m_dataRatePointCount = 0;       ///< 速率计算窗口内的数据点累计
    quint64 m_totalDataUpdates = 0;         ///< 数据更新信号发射总次数
    quint64 m_totalClears = 0;             ///< 数据清空操作总次数
    quint64 m_totalConfigChanges = 0;      ///< 配置变更总次数(通道配置集更换)
    quint64 m_totalWindowResizes = 0;      ///< 窗口大小变更总次数

    // 刷新合并
    QTimer* m_refreshTimer;                     ///< 定时刷新定时器（合并高频更新）
    int m_refreshInterval = 0;                  ///< 刷新间隔（毫秒），0=立即刷新
    QStringList m_pendingUpdates;               ///< 待刷新的通道名列表
};

#endif // CHARTMODEL_H
