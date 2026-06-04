/**
 * @file PerformanceMonitorLatency.cpp
 * @brief 性能监视器延迟查询与清除方法实现
 *
 * 从 PerformanceMonitor.cpp 拆分而来，包含模块延迟的
 * 平均/最大/最小查询、标签列表和清除方法。
 */

#include "utils/perf/PerformanceMonitor.h"
#include <limits>

/**
 * @brief 获取指定模块的平均延迟
 * @param tag 模块标签
 * @return 平均延迟（微秒），无数据返回 0
 */
double PerformanceMonitor::avgLatency(const QString& tag) const
{
    auto it = m_latencyMap.constFind(tag);
    if (it == m_latencyMap.constEnd() || it->isEmpty()) {
        return 0.0;
    }
    quint64 sum = 0;
    for (quint64 v : *it) {
        sum += v;
    }
    return static_cast<double>(sum) / it->size();
}

/**
 * @brief 获取指定模块的最大延迟
 * @param tag 模块标签
 * @return 最大延迟（微秒），无数据返回 0
 */
quint64 PerformanceMonitor::maxLatency(const QString& tag) const
{
    auto it = m_latencyMap.constFind(tag);
    if (it == m_latencyMap.constEnd() || it->isEmpty()) {
        return 0;
    }
    quint64 maxVal = 0;
    for (quint64 v : *it) {
        if (v > maxVal) maxVal = v;
    }
    return maxVal;
}

/**
 * @brief 获取指定模块的最小延迟
 * @param tag 模块标签
 * @return 最小延迟（微秒），无数据返回 0
 */
quint64 PerformanceMonitor::minLatency(const QString& tag) const
{
    auto it = m_latencyMap.constFind(tag);
    if (it == m_latencyMap.constEnd() || it->isEmpty()) {
        return 0;
    }
    quint64 minVal = std::numeric_limits<quint64>::max();
    for (quint64 v : *it) {
        if (v < minVal) minVal = v;
    }
    return minVal;
}

/**
 * @brief 获取所有已记录延迟的模块标签
 * @return 标签列表
 */
QStringList PerformanceMonitor::latencyTags() const
{
    return m_latencyMap.keys();
}

/**
 * @brief 清除指定模块的延迟记录
 * @param tag 模块标签
 */
void PerformanceMonitor::clearLatency(const QString& tag)
{
    m_latencyMap.remove(tag);
}

/**
 * @brief 清除所有延迟记录
 */
void PerformanceMonitor::clearAllLatency()
{
    m_latencyMap.clear();
}
