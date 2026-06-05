/**
 * @file DataAggregatorCompute.cpp
 * @brief 数据聚合器 — 计算引擎实现(滑动窗口聚合 + 时间窗口统计 + 辅助方法)
 *
 * 本文件从 DataAggregator.cpp 拆分而来，集中管理所有数据计算逻辑:
 *   - 滑动窗口聚合运算(Sum/Average/Min/Max/Count/First/Last)
 *   - 时间窗口边界检测与统计计算
 *   - 静态统计工具函数(min/max/avg/count/sum)
 *   - 窗口对齐与滚动聚合定时回调
 */

#include "utils/aggregator/DataAggregator.h"
#include <QtMath>
#include <algorithm>

// ---- 私有计算方法 ----

/** @brief 对指定数据源执行滑动窗口聚合计算 @param source 数据源名称 */
void DataAggregator::computeAggregate(const QString &source) {
    ++m_totalComputeCalls;
    auto it = m_sources.find(source);
    if (it == m_sources.end() || it->values.isEmpty()) return;
    const auto &vals = it->values;
    const int count = vals.size(); ///< 缓存size()避免重复调用
    switch (it->func) {
    case Sum:     { double s=0; for (auto v:vals) s+=v; it->result=s; break; }
    case Average: { double s=0; for (auto v:vals) s+=v; it->result=s/static_cast<double>(count); break; }
    case Min:     { double m=vals[0]; for (auto v:vals) if (v<m) m=v; it->result=m; break; }
    case Max:     { double m=vals[0]; for (auto v:vals) if (v>m) m=v; it->result=m; break; }
    case Count:   it->result=count; break;
    case First:   it->result=vals.first(); break;
    case Last:    it->result=vals.last(); break;
    }
}

/** @brief 将一个新值纳入时间窗口统计，自动检测窗口边界并关闭过期窗口 @param source 数据源名称 @param value 新数据值 @param timestampMs 当前时间戳(Epoch毫秒) */
void DataAggregator::processTimeWindow(const QString &source, double value, qint64 timestampMs) {
    auto it = m_timeWindows.find(source);
    if (it == m_timeWindows.end()) return;

    qint64 windowStart = alignToWindow(timestampMs, it->intervalSec);

    // 首次收到数据或窗口已切换
    if (it->currentWindowStart == 0 || windowStart != it->currentWindowStart) {
        // 关闭上一个窗口(如果有数据)
        if (it->currentWindowStart != 0 && !it->currentValues.isEmpty()) {
            WindowStats completed = computeStats(it->currentValues);
            completed.windowStartMs = it->currentWindowStart;
            completed.windowEndMs = it->currentWindowStart
                                    + static_cast<qint64>(it->intervalSec) * 1000;
            it->lastCompleted = completed;
            it->history.append(completed);

            // 限制历史记录数量
            while (it->history.size() > m_maxHistoryWindows) {
                it->history.removeFirst();
            }
            emit windowClosed(source, completed);
            ++m_totalWindowsCompleted;
        }

        // 开启新窗口
        it->currentWindowStart = windowStart;
        it->currentValues.clear();
    }

    it->currentValues.append(value);
}

/** @brief 计算指定列表值的完整统计(min/max/avg/count/sum) @param values 值列表 @return 统计结果 */
DataAggregator::WindowStats DataAggregator::computeStats(const QList<double> &values) {
    WindowStats stats;
    if (values.isEmpty()) return stats;

    stats.count = values.size();
    double minVal = values.first();
    double maxVal = values.first();
    double sumVal = 0.0;
    for (double v : values) {
        if (v < minVal) minVal = v;
        if (v > maxVal) maxVal = v;
        sumVal += v;
    }
    stats.min = minVal;
    stats.max = maxVal;
    stats.sum = sumVal;
    stats.avg = sumVal / stats.count;
    return stats;
}

/** @brief 计算指定时间戳所属的时间窗口起始时间(自然对齐) @param timestampMs 时间戳(Epoch毫秒) @param intervalSec 窗口间隔(秒) @return 窗口起始时间(Epoch毫秒) */
qint64 DataAggregator::alignToWindow(qint64 timestampMs, int intervalSec) {
    if (intervalSec <= 0) return timestampMs;
    qint64 intervalMs = static_cast<qint64>(intervalSec) * 1000;
    return (timestampMs / intervalMs) * intervalMs;
}

/** @brief 定时触发滚动聚合，发射所有数据源的最新聚合结果 */
void DataAggregator::onRollingTimeout() {
    if (m_sources.isEmpty()) return;
    ++m_totalRollingEmits;
    emit rollingAggregation(allResults());
}

// ---- 依赖计算方法的公共接口 ----

/** @brief 获取指定数据源当前正在进行的(尚未关闭的)时间窗口统计(实时更新) @param name 数据源名称 @return 当前窗口的实时统计 */
DataAggregator::WindowStats DataAggregator::currentWindowStats(const QString &name) const {
    auto it = m_timeWindows.constFind(name);
    if (it == m_timeWindows.constEnd() || it->currentValues.isEmpty())
        return WindowStats();

    WindowStats stats = computeStats(it->currentValues);
    stats.windowStartMs = it->currentWindowStart;
    stats.windowEndMs = it->currentWindowStart + static_cast<qint64>(it->intervalSec) * 1000;
    return stats;
}
