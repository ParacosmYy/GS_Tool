/**
 * @file ChartModel.h
 * @brief 图表数据模型 — 滑动窗口+降采样的实时波形数据管理
 *
 * 管理多通道波形数据的存储、滑动窗口截取和降采样显示。
 * 支持暂停/继续、缩放、通道动态增减。
 * 数据层：不依赖表现层，通过信号通知ChartWidget更新。
 */
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

/**
 * @brief 图表数据模型 — 管理多通道波形数据的滑动窗口截取和降采样显示
 *
 * 接收帧解析结果，按ChannelConfig分发到各通道缓冲区，
 * 维护滑动窗口和降采样计数器，通过信号通知ChartWidget刷新渲染。
 * 属于数据层，不依赖任何表现层组件。
 *
 * 协作关系:
 *   - ChartWidget: 监听 dataUpdated/channelsChanged 信号进行渲染
 *   - ChannelConfigSet: 提供通道配置（名称、数据源映射、降采样比率）
 *   - FrameParser: 通过 onFrameParsed 槽接收帧数据
 */
class ChartModel : public QObject {
    Q_OBJECT

public:
    explicit ChartModel(QObject* parent = nullptr);

    // ---- 配置 ----

    // 设置通道配置集合（会重建内部缓冲区）
    void setChannelConfigSet(const ChannelConfigSet& configSet);

    // 获取当前通道配置集合
    const ChannelConfigSet& channelConfigSet() const;

    // 设置滑动窗口大小（显示的最大数据点数）
    void setWindowSize(int points);

    // 获取滑动窗口大小
    int windowSize() const;

    // 设置刷新间隔（毫秒），数据到来后按此间隔合并刷新，减少渲染频率
    // 0表示每次数据到来都立即刷新
    void setRefreshInterval(int ms);

    // 获取刷新间隔
    int refreshInterval() const;

    // ---- 数据查询 ----

    // 获取指定通道的当前可见数据点（滑动窗口内的点）
    QVector<QPointF> channelData(const QString& displayName) const;

    // 获取所有启用通道的数据（用于批量渲染）
    // 返回: QMap<displayName, QVector<QPointF>>
    QMap<QString, QVector<QPointF>> allChannelData() const;

    // 获取指定通道的Y值范围
    // 返回: pair<min, max>，无数据时返回 <0, 0>
    QPair<double, double> channelYRange(const QString& displayName) const;

    // 获取所有通道的全局Y值范围（用于自动Y轴）
    QPair<double, double> globalYRange() const;

    // 获取当前通道名称列表
    QStringList channelNames() const;

    // 获取当前X轴范围（样本计数范围）
    QPair<double, double> xRange() const;

    // 获取统计信息
    qint64 totalPointsReceived() const;    // 总共接收的数据点数（qint64防溢出）
    qint64 currentFrameIndex() const;      // 当前帧索引（X轴计数器）

    // ---- 统计信息 ----

    /**
     * @brief 获取跨所有通道添加的数据点总数
     * @return 数据点总数（跨所有通道累计）
     */
    quint64 totalDataPoints() const;

    /**
     * @brief 获取历史创建的通道总数
     * @return 通道创建计数（累计，包含已移除的）
     */
    quint64 channelsCreated() const;

    /**
     * @brief 获取历史移除的通道总数
     * @return 通道移除计数（累计）
     */
    quint64 channelsRemoved() const;

    /**
     * @brief 获取峰值数据速率（数据点/秒）
     * @return 峰值速率（双精度浮点，单位: 点/秒）
     */
    double peakDataRate() const;

    /**
     * @brief 重置所有图表统计计数器为初始值
     *
     * 将 totalDataPoints、channelsCreated、channelsRemoved、peakDataRate
     * 全部清零，并重置速率计算时间戳。不影响通道数据和配置。
     */
    void resetChartStatistics();

    // ---- 操作 ----

    /**
     * @brief 添加单个通道到配置集并更新统计计数器
     * @param config 通道配置
     *
     * 向 ChannelConfigSet 追加通道、创建对应缓冲区，
     * 并递增 m_channelsCreated 计数器。
     */
    void addChannel(const ChannelConfig& config);

    /**
     * @brief 移除指定通道并更新统计计数器
     * @param displayName 通道显示名称
     *
     * 从 ChannelConfigSet 和内部缓冲区中移除通道，
     * 并递增 m_channelsRemoved 计数器。
     */
    void removeChannel(const QString& displayName);

    /**
     * @brief 向指定通道添加一个数据点（公开接口，含统计更新）
     * @param displayName 通道显示名称
     * @param value 数据值
     *
     * 递增 m_totalDataPoints，更新峰值数据速率，
     * 并委托内部 appendPoint() 完成降采样和滑动窗口裁剪。
     */
    void addDataPoint(const QString& displayName, double value);

    // 清除所有通道数据
    void clear();

signals:
    /** @brief 通道数据更新通知（视图据此刷新渲染） @param updatedChannels 本次数据更新的通道名列表 */
    void dataUpdated(const QStringList& updatedChannels);

    /** @brief 通道配置变更通知（增删通道时发出） */
    void channelsChanged();

    /** @brief 全部数据已清除 */
    void dataCleared();

public slots:
    // 接收帧解析结果，按ChannelConfig分发到各通道
    // 连接: FrameParser::frameParsed -> ChartModel::onFrameParsed
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

private slots:
    // 定时刷新（用于合并高频数据更新，减少信号发射频率）
    void onRefreshTick();

private:
    // 单个通道的内部数据缓冲区
    struct ChannelBuffer {
        QVector<QPointF> points;        // 滑动窗口内的数据点
        int sampleCounter = 0;          // 降采样计数器（每到达sampleDivisor次才记录一个点）
    };

    // 从配置中重建通道缓冲区
    void rebuildBuffers();

    // 向指定通道追加一个数据点（应用降采样和滑动窗口）
    void appendPoint(const QString& displayName, double value, int sampleDivisor);

    // 立即发射待更新的通道数据信号
    void flushPendingUpdates();

    ChannelConfigSet m_configSet;
    QMap<QString, ChannelBuffer> m_buffers;     // displayName -> buffer
    int m_windowSize = 200;
    qint64 m_frameIndex = 0;                      ///< 全局帧计数器（X轴），qint64 防止长时间运行后溢出
    qint64 m_totalPoints = 0;                        ///< 总数据点计数（qint64防溢出）

    // 统计计数器
    quint64 m_totalDataPoints = 0;               ///< 跨所有通道添加的数据点总数
    quint64 m_channelsCreated = 0;               ///< 历史创建的通道总数（累计）
    quint64 m_channelsRemoved = 0;               ///< 历史移除的通道总数（累计）
    double m_peakDataRate = 0.0;                 ///< 峰值数据速率（数据点/秒）
    qint64 m_dataRateTimestamp = 0;              ///< 速率计算用的上次时间戳（毫秒纪元）
    quint64 m_dataRatePointCount = 0;            ///< 速率计算窗口内的数据点累计

    // 刷新合并
    QTimer* m_refreshTimer;
    int m_refreshInterval = 0;                  // 0=立即刷新
    QStringList m_pendingUpdates;               // 待刷新的通道名
};

#endif // CHARTMODEL_H
