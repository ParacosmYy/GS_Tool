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

#include "chart/ChannelConfig.h"

// 图表数据模型 -- 管理多通道数据缓冲区、滑动窗口、降采样
// 职责: 接收帧数据 -> 按ChannelConfig分发 -> 维护滑动窗口 -> 通知视图更新
// 不负责: 图表渲染（由ChartWidget负责）
// 数据层: 不依赖任何表现层类
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
    int totalPointsReceived() const;    // 总共接收的数据点数
    int currentFrameIndex() const;      // 当前帧索引（X轴计数器）

    // ---- 操作 ----

    // 清除所有通道数据
    void clear();

signals:
    // 通道数据更新通知（视图据此刷新渲染）
    // updatedChannels: 本次数据更新的通道名列表
    void dataUpdated(const QStringList& updatedChannels);

    // 通道配置变更通知（增删通道时发出）
    void channelsChanged();

    // 全部数据已清除
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
    int m_frameIndex = 0;                       // 全局帧计数器（X轴）
    int m_totalPoints = 0;

    // 刷新合并
    QTimer* m_refreshTimer;
    int m_refreshInterval = 0;                  // 0=立即刷新
    QStringList m_pendingUpdates;               // 待刷新的通道名
};

#endif // CHARTMODEL_H
