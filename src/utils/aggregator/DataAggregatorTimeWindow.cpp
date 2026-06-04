/**
 * @file DataAggregatorTimeWindow.cpp
 * @brief 数据聚合器 — 时间窗口聚合方法实现
 *
 * 从 DataAggregator.cpp 拆分而来，包含时间窗口聚合的启用/禁用/
 * 统计查询/历史管理方法。核心计算逻辑(processTimeWindow)在
 * DataAggregatorCompute.cpp 中。
 *
 * @see DataAggregator.cpp — 滑动窗口聚合与实时滚动聚合
 * @see DataAggregatorCompute.cpp — 私有计算方法
 */

#include "utils/aggregator/DataAggregator.h"

// ---- 时间窗口聚合 ----

/** @brief 为数据源启用时间窗口聚合，按自然时间对齐窗口边界 @param name 数据源名称(必须已通过addSource添加) @param intervalSec 时间窗口间隔(秒) */
void DataAggregator::enableTimeWindow(const QString &name, int intervalSec) {
    if (!m_sources.contains(name) || intervalSec <= 0) return;
    ++m_totalTimeWindowEnables;
    TimeWindowData twd;
    twd.intervalSec = intervalSec;
    twd.currentWindowStart = 0;
    m_timeWindows[name] = twd;
}

/** @brief 禁用指定数据源的时间窗口聚合 @param name 数据源名称 */
void DataAggregator::disableTimeWindow(const QString &name) {
    if (m_timeWindows.contains(name)) {
        ++m_totalTimeWindowDisables;
        m_timeWindows.remove(name);
    }
}

/** @brief 获取指定数据源最近一个已完成时间窗口的统计结果 @param name 数据源名称 @return 统计结果；若不存在或无已完成窗口，返回count=0的结构体 */
DataAggregator::WindowStats DataAggregator::lastWindowStats(const QString &name) const {
    auto it = m_timeWindows.constFind(name);
    if (it != m_timeWindows.constEnd()) return it->lastCompleted;
    return WindowStats();
}

/** @brief 获取指定数据源所有已完成时间窗口的统计结果列表(时间升序) @param name 数据源名称 @return 统计结果列表 */
QList<DataAggregator::WindowStats> DataAggregator::windowStatsHistory(const QString &name) const {
    auto it = m_timeWindows.constFind(name);
    if (it != m_timeWindows.constEnd()) return it->history;
    return {};
}

// currentWindowStats — 已拆分至 DataAggregatorCompute.cpp(依赖computeStats)

/** @brief 设置时间窗口历史记录最大保留数量 @param max 最大保留数量 */
void DataAggregator::setMaxHistoryWindows(int max) {
    m_maxHistoryWindows = qMax(1, max);
}

/** @brief 获取时间窗口历史记录最大保留数量 @return 最大保留数量 */
int DataAggregator::maxHistoryWindows() const {
    return m_maxHistoryWindows;
}
