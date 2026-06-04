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
    explicit ChartModel(QObject* parent = nullptr); ///< 构造
    // ---- 配置 ----
    void setChannelConfigSet(const ChannelConfigSet& configSet); ///< 设置通道配置集(重建缓冲区)
    const ChannelConfigSet& channelConfigSet() const; ///< 获取当前通道配置集
    void setWindowSize(int points);          ///< 设置滑动窗口大小
    int windowSize() const;                  ///< 获取滑动窗口大小
    void setRefreshInterval(int ms);         ///< 设置刷新间隔(ms, 0=每次数据立即刷新)
    int refreshInterval() const;             ///< 获取刷新间隔

    // ---- 数据查询 ----
    QVector<QPointF> channelData(const QString& displayName) const; ///< 指定通道可见数据点
    QMap<QString, QVector<QPointF>> allChannelData() const; ///< 所有启用通道数据
    QPair<double, double> channelYRange(const QString& displayName) const; ///< 指定通道Y值范围
    QPair<double, double> globalYRange() const; ///< 全局Y值范围
    QStringList channelNames() const;        ///< 通道名称列表
    QPair<double, double> xRange() const;    ///< X轴范围
    qint64 totalPointsReceived() const;      ///< 总接收数据点数
    qint64 currentFrameIndex() const;        ///< 当前帧索引(X轴计数器)

    // ---- 统计信息 ----
    quint64 totalDataPoints() const;         ///< 跨所有通道的数据点总数
    quint64 channelsCreated() const;         ///< 历史创建通道总数(累计)
    quint64 channelsRemoved() const;         ///< 历史移除通道总数(累计)
    quint64 totalChannelsActive() const;     ///< 当前活跃通道数(实时)
    quint64 maxDataPointsInChannel() const;  ///< 单通道最大数据点数(峰值)
    double peakDataRate() const;             ///< 峰值数据速率(点/秒)
    void resetChartStatistics();             ///< 重置图表统计(不影响通道数据和配置)
    void resetStats();                       ///< 别名，调用resetChartStatistics

    // ---- 操作 ----
    void addChannel(const ChannelConfig& config); ///< 添加通道(更新统计)
    void removeChannel(const QString& displayName); ///< 移除通道(更新统计)
    void addDataPoint(const QString& displayName, double value); ///< 添加数据点(含统计)
    void clear();                            ///< 清除所有通道数据

signals:
    void dataUpdated(const QStringList& updatedChannels); ///< 通道数据更新通知
    void channelsChanged();                  ///< 通道配置变更通知
    void dataCleared();                      ///< 全部数据已清除

public slots:
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame); ///< 接收帧解析结果

private slots:
    void onRefreshTick();                    ///< 定时刷新(合并高频更新)

private:
    struct ChannelBuffer { QVector<QPointF> points; int sampleCounter = 0; }; ///< 单通道内部缓冲区
    void rebuildBuffers();                   ///< 重建所有通道缓冲区
    void appendPoint(const QString& displayName, double value, int sampleDivisor); ///< 追加数据点(含降采样)
    void flushPendingUpdates();              ///< 打包发射dataUpdated并清空待刷新列表

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

    // 刷新合并
    QTimer* m_refreshTimer;                     ///< 定时刷新定时器（合并高频更新）
    int m_refreshInterval = 0;                  ///< 刷新间隔（毫秒），0=立即刷新
    QStringList m_pendingUpdates;               ///< 待刷新的通道名列表
};

#endif // CHARTMODEL_H
