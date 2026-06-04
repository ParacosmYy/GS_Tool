/**
 * @file ChartModel.cpp
 * @brief 波形数据模型实现 - 滑动窗口 + 降采样的实时数据管理
 *
 * 核心机制:
 *   - 滑动窗口: 固定容量，旧数据自动丢弃
 *   - 降采样: 数据量超过显示像素时自动降采样，保留极值
 *   - 实时速率: 通过定时采样计算每秒数据点数
 */

#include "chart/model/ChartModel.h"

#include <QVariantMap>
#include <QTimer>
#include <QtMath>
#include <algorithm>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造函数，初始化刷新定时器 @param parent 父对象 */
ChartModel::ChartModel(QObject* parent)
    : QObject(parent)
    , m_refreshTimer(new QTimer(this))
{
    m_refreshTimer->setObjectName(QStringLiteral("chartModelRefreshTimer"));
    m_refreshTimer->setSingleShot(true);
    connect(m_refreshTimer, &QTimer::timeout,
            this, &ChartModel::onRefreshTick);
}

// ============================================================================
// 配置接口
// ============================================================================

/** @brief 设置通道配置集(重建缓冲区并发射channelsChanged) @param configSet 通道配置集 */
void ChartModel::setChannelConfigSet(const ChannelConfigSet& configSet)
{
    ++m_totalConfigChanges;  ///< 统计: 配置变更次数递增
    m_configSet = configSet;
    rebuildBuffers();

    /* 更新当前活跃通道数统计 */
    m_totalChannelsActive = static_cast<quint64>(m_buffers.size());

    emit channelsChanged();
}

/** @brief 返回当前通道配置集(只读引用) */
const ChannelConfigSet& ChartModel::channelConfigSet() const
{
    return m_configSet;
}

/** @brief 设置滑动窗口大小(超过时裁剪旧数据) @param points 窗口点数(最小1) */
void ChartModel::setWindowSize(int points)
{
    if (points < 1) points = 1;
    if (points != m_windowSize) {
        ++m_totalWindowResizes;  ///< 统计: 窗口大小变更次数递增
    }
    m_windowSize = points;

    for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it) {
        ChannelBuffer& buf = it.value();
        while (buf.points.size() > m_windowSize) {
            buf.points.removeFirst();
        }
    }
}

/** @brief 返回滑动窗口大小 */
int ChartModel::windowSize() const
{
    return m_windowSize;
}

/** @brief 设置刷新间隔(0=立即刷新，>0=批量刷新减少信号频率) @param ms 刷新间隔毫秒数 */
void ChartModel::setRefreshInterval(int ms)
{
    if (ms < 0) ms = 0;
    m_refreshInterval = ms;

    if (m_refreshInterval == 0) {
        m_refreshTimer->stop();
        flushPendingUpdates();
    }
}

/** @brief 返回刷新间隔 */
int ChartModel::refreshInterval() const
{
    return m_refreshInterval;
}

// ============================================================================
// 数据查询和统计接口见 ChartModelStats.cpp
// ============================================================================

// ============================================================================
// 操作接口
// ============================================================================

/** @brief 添加单个通道到配置集并更新统计计数器 @param config 通道配置 */
void ChartModel::addChannel(const ChannelConfig& config)
{
    if (m_buffers.contains(config.displayName)) {
        m_buffers.remove(config.displayName);
    }

    m_configSet.addChannel(config);

    ChannelBuffer buf;
    buf.sampleCounter = 0;
    buf.points.reserve(m_windowSize);
    m_buffers.insert(config.displayName, buf);

    m_channelsCreated++;
    m_totalChannelsActive = static_cast<quint64>(m_buffers.size());

    emit channelsChanged();
}

/** @brief 移除指定通道并更新统计计数器 @param displayName 通道显示名称 */
void ChartModel::removeChannel(const QString& displayName)
{
    m_configSet.removeChannel(displayName);
    m_buffers.remove(displayName);

    m_channelsRemoved++;
    m_totalChannelsActive = static_cast<quint64>(m_buffers.size());
    m_pendingUpdates.removeAll(displayName);

    emit channelsChanged();
}

/** @brief 向指定通道添加一个数据点（公开接口，含统计更新）
 * @param displayName 通道显示名称
 * @param value 数据值
 *
 * 递增 m_totalDataPoints，计算并更新峰值数据速率 m_peakDataRate，
 * 然后委托内部 appendPoint() 完成降采样和滑动窗口裁剪。
 */
void ChartModel::addDataPoint(const QString& displayName, double value)
{
    m_totalDataPoints++;

    /* 更新峰值数据速率 */
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_dataRateTimestamp == 0) {
        m_dataRateTimestamp = now;
        m_dataRatePointCount = 1;
    } else {
        m_dataRatePointCount++;
        qint64 elapsedMs = now - m_dataRateTimestamp;
        if (elapsedMs >= 500) {
            double rate = static_cast<double>(m_dataRatePointCount)
                          / (elapsedMs / 1000.0);
            if (rate > m_peakDataRate) {
                m_peakDataRate = rate;
            }
            m_dataRateTimestamp = now;
            m_dataRatePointCount = 0;
        }
    }

    appendPoint(displayName, value, 1);
}

/** @brief 清除所有通道数据、帧索引和待刷新队列，发射dataCleared */
void ChartModel::clear()
{
    ++m_totalClears;  ///< 统计: 数据清空次数递增
    m_frameIndex = 0;
    m_totalPoints = 0;
    m_pendingUpdates.clear();
    m_refreshTimer->stop();

    for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it) {
        it.value().points.clear();
        it.value().sampleCounter = 0;
    }

    emit dataCleared();
}

// 槽函数(onFrameParsed/onRefreshTick)和内部方法(rebuildBuffers/appendPoint/flushPendingUpdates)
// 见 ChartModelSlots.cpp

// 数据查询接口见 ChartModelStats.cpp
