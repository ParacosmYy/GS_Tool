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
// 数据查询接口
// ============================================================================

/** @brief 获取指定通道的完整数据点序列 @param displayName 通道显示名 */
QVector<QPointF> ChartModel::channelData(const QString& displayName) const
{
    auto it = m_buffers.constFind(displayName);
    if (it == m_buffers.constEnd()) {
        return QVector<QPointF>();
    }
    return it.value().points;
}

/** @brief 获取所有启用通道的数据 @return 通道名→数据点序列的映射 */
QMap<QString, QVector<QPointF>> ChartModel::allChannelData() const
{
    QMap<QString, QVector<QPointF>> result;
    for (auto it = m_buffers.constBegin(); it != m_buffers.constEnd(); ++it) {
        result[it.key()] = it.value().points;
    }
    return result;
}

/** @brief 获取指定通道的Y值范围 @param displayName 通道显示名 @return <最小值,最大值>，无数据返回<0,0> */
QPair<double, double> ChartModel::channelYRange(const QString& displayName) const
{
    auto it = m_buffers.constFind(displayName);
    if (it == m_buffers.constEnd() || it.value().points.isEmpty()) {
        return qMakePair(0.0, 0.0);
    }

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    for (const QPointF& pt : it.value().points) {
        if (!qIsNaN(pt.y()) && !qIsInf(pt.y())) {
            if (pt.y() < yMin) yMin = pt.y();
            if (pt.y() > yMax) yMax = pt.y();
        }
    }

    if (yMin > yMax) {
        return qMakePair(0.0, 0.0);
    }
    return qMakePair(yMin, yMax);
}

/** @brief 获取所有通道的全局Y值范围 @return <全局最小值,全局最大值>，无数据返回<0,0> */
QPair<double, double> ChartModel::globalYRange() const
{
    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();
    bool hasValidData = false;

    for (auto it = m_buffers.constBegin(); it != m_buffers.constEnd(); ++it) {
        for (const QPointF& pt : it.value().points) {
            if (!qIsNaN(pt.y()) && !qIsInf(pt.y())) {
                if (pt.y() < globalMin) globalMin = pt.y();
                if (pt.y() > globalMax) globalMax = pt.y();
                hasValidData = true;
            }
        }
    }

    if (!hasValidData) {
        return qMakePair(0.0, 0.0);
    }
    return qMakePair(globalMin, globalMax);
}

/** @brief 获取所有已注册通道的显示名列表 */
QStringList ChartModel::channelNames() const
{
    return m_buffers.keys();
}

/** @brief 获取当前X轴范围(基于帧索引和窗口大小) @return <起始值,结束值> */
QPair<double, double> ChartModel::xRange() const
{
    if (m_frameIndex <= 0) {
        return qMakePair(0.0, 10.0);
    }
    if (m_frameIndex >= m_windowSize) {
        return qMakePair(static_cast<double>(m_frameIndex - m_windowSize),
                         static_cast<double>(m_frameIndex));
    }
    return qMakePair(0.0, static_cast<double>(m_frameIndex) + 10.0);
}

/** @brief 返回累计接收的数据点总数 */
qint64 ChartModel::totalPointsReceived() const
{
    return m_totalPoints;
}

/** @brief 获取当前帧索引(X轴计数器) @return 帧索引值 */
qint64 ChartModel::currentFrameIndex() const
{
    return m_frameIndex;
}

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 返回跨所有通道添加的数据点总数 */
quint64 ChartModel::totalDataPoints() const
{
    return m_totalDataPoints;
}

/** @brief 返回历史创建的通道总数（累计） */
quint64 ChartModel::channelsCreated() const
{
    return m_channelsCreated;
}

/** @brief 返回历史移除的通道总数（累计） */
quint64 ChartModel::channelsRemoved() const
{
    return m_channelsRemoved;
}

/** @brief 返回当前活跃通道数（实时） */
quint64 ChartModel::totalChannelsActive() const
{
    return static_cast<quint64>(m_buffers.size());
}

/** @brief 返回单通道内出现过的最大数据点数（峰值） */
quint64 ChartModel::maxDataPointsInChannel() const
{
    return m_maxDataPointsInChannel;
}

/** @brief 返回峰值数据速率（数据点/秒） */
double ChartModel::peakDataRate() const
{
    return m_peakDataRate;
}

/** @brief 重置所有图表统计计数器为初始值
 *
 * 将所有统计计数器清零，重置速率计算时间戳和窗口计数。
 * 不影响通道数据、配置和帧索引。
 */
void ChartModel::resetChartStatistics()
{
    m_totalDataPoints = 0;
    m_channelsCreated = 0;
    m_channelsRemoved = 0;
    m_totalChannelsActive = 0;
    m_maxDataPointsInChannel = 0;
    m_peakDataRate = 0.0;
    m_dataRateTimestamp = 0;
    m_dataRatePointCount = 0;
}

/** @brief 重置所有扩展统计计数器为初始值（别名接口） */
void ChartModel::resetStats()
{
    resetChartStatistics();
}

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

// ============================================================================
// 槽函数 -- 帧数据接收
// ============================================================================

/** @brief 帧解析回调：计算各通道值、降采样、追加缓冲区
 * @param fields 解析后的字段映射
 * @param rawFrame 原始帧数据(未使用)
 */
void ChartModel::onFrameParsed(const QVariantMap& fields,
                               const QByteArray& rawFrame)
{
    Q_UNUSED(rawFrame);

    if (m_configSet.channels().isEmpty()) {
        return;
    }

    QMap<QString, double> computedValues = m_configSet.computeAll(fields);
    m_frameIndex++;

    for (auto it = computedValues.constBegin();
         it != computedValues.constEnd(); ++it) {
        const QString& channelName = it.key();
        double value = it.value();

        if (qIsNaN(value)) {
            auto bufIt = m_buffers.find(channelName);
            if (bufIt != m_buffers.end()) {
                bufIt.value().sampleCounter++;
            }
            continue;
        }

        const ChannelConfig* cfg = m_configSet.findChannel(channelName);
        int sampleDivisor = (cfg != nullptr) ? cfg->sampleDivisor : 1;
        if (sampleDivisor < 1) sampleDivisor = 1;

        appendPoint(channelName, value, sampleDivisor);
        m_totalPoints++;
    }

    if (m_refreshInterval == 0) {
        flushPendingUpdates();
    } else {
        if (!m_refreshTimer->isActive()) {
            m_refreshTimer->start(m_refreshInterval);
        }
    }
}

// ============================================================================
// 槽函数 -- 定时刷新
// ============================================================================

/** @brief 刷新定时器回调：刷出所有挂起的数据更新 */
void ChartModel::onRefreshTick()
{
    flushPendingUpdates();
}

// ============================================================================
// 内部方法
// ============================================================================

/** @brief 重建通道缓冲区(清除所有数据并为启用的通道预分配空间) */
void ChartModel::rebuildBuffers()
{
    m_buffers.clear();

    const QVector<ChannelConfig>& channels = m_configSet.channels();
    for (const ChannelConfig& cfg : channels) {
        if (cfg.enabled && !cfg.displayName.isEmpty()) {
            ChannelBuffer buf;
            buf.sampleCounter = 0;
            buf.points.reserve(m_windowSize);
            m_buffers.insert(cfg.displayName, buf);
        }
    }
}

/** @brief 向指定通道追加数据点(含降采样和滑动窗口裁剪)
 * @param displayName 通道名称
 * @param value 数据值
 * @param sampleDivisor 降采样因子
 */
void ChartModel::appendPoint(const QString& displayName, double value,
                             int sampleDivisor)
{
    auto it = m_buffers.find(displayName);
    if (it == m_buffers.end()) {
        return;
    }

    ChannelBuffer& buf = it.value();

    buf.sampleCounter++;
    if (buf.sampleCounter % sampleDivisor != 0) {
        return;
    }

    buf.points.append(QPointF(m_frameIndex, value));

    /* 滑动窗口裁剪 */
    while (buf.points.size() > m_windowSize) {
        buf.points.removeFirst();
    }

    /* 更新单通道最大数据点数统计 */
    quint64 currentSize = static_cast<quint64>(buf.points.size());
    if (currentSize > m_maxDataPointsInChannel) {
        m_maxDataPointsInChannel = currentSize;
    }

    if (!m_pendingUpdates.contains(displayName)) {
        m_pendingUpdates.append(displayName);
    }
}

/** @brief 刷出所有挂起的通道更新，发射dataUpdated信号通知ChartWidget重绘 */
void ChartModel::flushPendingUpdates()
{
    if (m_pendingUpdates.isEmpty()) {
        return;
    }

    QStringList updates = m_pendingUpdates;
    m_pendingUpdates.clear();
    emit dataUpdated(updates);
}
