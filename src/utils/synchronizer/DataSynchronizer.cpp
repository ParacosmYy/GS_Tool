/**
 * @file DataSynchronizer.cpp
 * @brief 数据同步引擎实现 -- 多数据流时间戳对齐
 *
 * 维护每流的 timestamp→value 有序映射，pushData 时以最新时间戳
 * 为基准尝试跨流对齐。对齐策略: 在 maxDrift 窗口内找最近值，
 * 通过线性插值获得精确值。超出窗口的数据点触发 driftExceeded。
 */

#include "utils/synchronizer/DataSynchronizer.h"

#include <QDateTime>
#include <algorithm>

// ---- 构造 / 析构 ----

/** @brief 构造数据同步引擎 @param parent 父对象 */
DataSynchronizer::DataSynchronizer(QObject *parent)
    : QObject(parent)
{
}

/** @brief 析构函数 */
DataSynchronizer::~DataSynchronizer() = default;

// ---- 流管理 ----

/** @brief 注册一个新数据流(若已存在则跳过) @param name 流名称 */
void DataSynchronizer::addStream(const QString &name)
{
    if (!m_streams.contains(name)) {
        m_streams.insert(name, QMap<qint64, double>());
    }
}

/** @brief 移除指定数据流及其缓冲数据 @param name 流名称 */
void DataSynchronizer::removeStream(const QString &name)
{
    m_streams.remove(name);
}

/** @brief 查询所有已注册流名称 @return 流名称列表 */
QStringList DataSynchronizer::streams() const
{
    return m_streams.keys();
}

// ---- 数据输入 ----

/** @brief 向指定流推入一条带时间戳数据，并尝试跨流对齐 @param stream 流名称 @param timestamp 时间戳(ms) @param value 数据值 */
void DataSynchronizer::pushData(const QString &stream, qint64 timestamp, double value)
{
    auto it = m_streams.find(stream);
    if (it == m_streams.end()) {
        return;
    }
    it->insert(timestamp, value);
    tryAlign(timestamp);
    pruneBuffers();
}

// ---- 数据查询 ----

/** @brief 查询指定时间范围内的已对齐同步点 @param fromMs 起始时间戳(含) @param toMs 结束时间戳(含) @return 范围内的同步点列表 */
QVector<DataSynchronizer::SyncPoint> DataSynchronizer::getSyncedData(qint64 fromMs, qint64 toMs) const
{
    QVector<SyncPoint> result;
    for (const auto &sp : m_syncedPoints) {
        if (sp.timestamp >= fromMs && sp.timestamp <= toMs) {
            result.append(sp);
        }
    }
    return result;
}

// ---- 配置 ----

/** @brief 设置最大允许漂移(ms)，最小1ms @param ms 最大漂移毫秒数 */
void DataSynchronizer::setMaxDrift(double ms)
{
    m_maxDriftMs = qMax(1.0, ms);
}

/** @brief 获取当前最大漂移设置 @return 最大漂移(ms) */
double DataSynchronizer::maxDrift() const
{
    return m_maxDriftMs;
}

// ---- 统计 ----

/** @brief 获取运行时统计快照 @return Stats 结构体 */
DataSynchronizer::Stats DataSynchronizer::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void DataSynchronizer::resetStatistics()
{
    m_stats = Stats();
    m_driftSum = 0.0;
}

/** @brief 清空所有流的缓冲数据和已对齐点(保留流注册) */
void DataSynchronizer::clearBuffers()
{
    for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
        it->clear();
    }
    m_syncedPoints.clear();
}

// ---- 私有方法 ----

/** @brief 尝试以 referenceTs 为基准进行跨流对齐 @param referenceTs 基准时间戳(ms) */
void DataSynchronizer::tryAlign(qint64 referenceTs)
{
    ++m_stats.totalAlignments;

    SyncPoint sp;
    sp.timestamp = referenceTs;
    bool allOk = true;

    for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ++it) {
        const QString &name = it.key();
        bool ok = false;
        double val = interpolateNear(name, referenceTs, ok);
        if (!ok) {
            allOk = false;

            // 检查该流是否有数据但漂移超限
            const auto &dataMap = it.value();
            if (!dataMap.isEmpty()) {
                auto upper = dataMap.lowerBound(referenceTs);
                double drift = 0.0;
                if (upper != dataMap.constEnd()) {
                    drift = qAbs(upper.key() - referenceTs);
                } else if (upper != dataMap.constBegin()) {
                    --upper;
                    drift = qAbs(upper.key() - referenceTs);
                }
                if (drift > m_maxDriftMs) {
                    ++m_stats.droppedPoints;
                    emit driftExceeded(name, drift);
                }
            }
            break;
        }

        // 检查实际漂移并更新统计
        const auto &dataMap = it.value();
        auto upper = dataMap.lowerBound(referenceTs);
        double drift = 0.0;
        if (upper != dataMap.constEnd()) {
            drift = qAbs(upper.key() - referenceTs);
        } else if (upper != dataMap.constBegin()) {
            --upper;
            drift = qAbs(upper.key() - referenceTs);
        }

        if (drift > m_stats.maxDriftMs) {
            m_stats.maxDriftMs = drift;
        }
        m_driftSum += drift;
        sp.values[name] = val;
    }

    if (allOk && !sp.values.isEmpty()) {
        ++m_stats.totalSyncPoints;
        m_stats.avgDriftMs = m_driftSum / m_stats.totalAlignments;

        m_syncedPoints.append(sp);
        if (m_syncedPoints.size() > m_maxSyncedPoints) {
            m_syncedPoints.removeFirst();
        }
        emit syncPointReady(sp);
    }
}

/** @brief 在指定流中对 targetTs 做线性插值 @param stream 流名称 @param targetTs 目标时间戳 @param ok 是否找到有效值 @return 插值结果 */
double DataSynchronizer::interpolateNear(const QString &stream, qint64 targetTs, bool &ok) const
{
    ok = false;
    auto streamIt = m_streams.constFind(stream);
    if (streamIt == m_streams.constEnd() || streamIt->isEmpty()) {
        return 0.0;
    }

    const auto &dataMap = streamIt.value();

    // 找到第一个 >= targetTs 的迭代器
    auto upper = dataMap.lowerBound(targetTs);

    // 情况1: 精确命中
    if (upper != dataMap.constEnd() && upper.key() == targetTs) {
        ok = true;
        return upper.value();
    }

    // 情况2: targetTs 在所有数据之前
    if (upper == dataMap.constBegin()) {
        if (qAbs(upper.key() - targetTs) <= m_maxDriftMs) {
            ok = true;
            return upper.value();
        }
        return 0.0;
    }

    // 情况3: targetTs 在所有数据之后
    if (upper == dataMap.constEnd()) {
        auto last = dataMap.constEnd();
        --last;
        if (qAbs(last.key() - targetTs) <= m_maxDriftMs) {
            ok = true;
            return last.value();
        }
        return 0.0;
    }

    // 情况4: targetTs 在两个数据点之间 -- 线性插值
    auto lower = upper;
    --lower;
    qint64 t0 = lower.key();
    qint64 t1 = upper.key();
    double v0 = lower.value();
    double v1 = upper.value();

    // 检查最近点的漂移是否在窗口内
    double drift = qMin(qAbs(t0 - targetTs), qAbs(t1 - targetTs));
    if (drift > m_maxDriftMs) {
        return 0.0;
    }

    ok = true;
    if (t1 == t0) {
        return v0;
    }
    double ratio = static_cast<double>(targetTs - t0) / static_cast<double>(t1 - t0);
    return v0 + ratio * (v1 - v0);
}

/** @brief 修剪各流中过旧的缓冲数据，保留最近 maxDrift*4 范围内的数据 */
void DataSynchronizer::pruneBuffers()
{
    // 找到所有流中最近的时间戳
    qint64 latestTs = 0;
    for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ++it) {
        if (!it->isEmpty()) {
            qint64 ts = it->lastKey();
            if (ts > latestTs) {
                latestTs = ts;
            }
        }
    }

    if (latestTs == 0) {
        return;
    }

    qint64 cutoff = latestTs - static_cast<qint64>(m_maxDriftMs * 4);
    for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
        auto &dataMap = *it;
        auto removeEnd = dataMap.lowerBound(cutoff);
        // 保留 cutoff 处的一个点用于插值
        if (removeEnd != dataMap.begin()) {
            --removeEnd;
        }
        if (removeEnd != dataMap.begin()) {
            dataMap.erase(dataMap.begin(), removeEnd);
        }
    }
}
