/**
 * @file CountingMultiSet.cpp
 * @brief 计数多重集合实现 — 频率统计/Top-K/Jaccard相似度
 */

#include "utils/multiset/CountingMultiSet.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
CountingMultiSet::CountingMultiSet(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/** @brief 插入元素 @param value 元素 @param count 插入次数 */
void CountingMultiSet::insert(int value, int count)
{
    QElapsedTimer timer;
    timer.start();

    if (count <= 0) return;

    int oldCount = m_counts.value(value, 0);
    m_counts[value] = oldCount + count;

    m_stats.totalInserts += count;
    m_stats.uniqueElements = m_counts.size();
    int total = 0;
    for (auto it = m_counts.constBegin(); it != m_counts.constEnd(); ++it)
        total += it.value();
    m_stats.totalCount = total;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalQueries)
        : 0.0;

    emit elementInserted(value, m_counts[value]);
}

/** @brief 删除元素 @param value 元素 @param count 删除次数 @return 实际删除数 */
int CountingMultiSet::remove(int value, int count)
{
    QElapsedTimer timer;
    timer.start();

    if (count <= 0) return 0;

    int current = m_counts.value(value, 0);
    if (current == 0) return 0;

    int removed = std::min(count, current);
    int newCount = current - removed;

    if (newCount > 0) {
        m_counts[value] = newCount;
    } else {
        m_counts.remove(value);
    }

    m_stats.totalRemoves += removed;
    m_stats.uniqueElements = m_counts.size();
    int total = 0;
    for (auto it = m_counts.constBegin(); it != m_counts.constEnd(); ++it)
        total += it.value();
    m_stats.totalCount = total;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalQueries)
        : 0.0;

    emit elementRemoved(value, newCount);
    return removed;
}

/** @brief 查询元素频率 @param value 元素 @return 频率 */
int CountingMultiSet::count(int value)
{
    m_stats.totalQueries++;
    return m_counts.value(value, 0);
}

/** @brief 是否包含元素 @param value 元素 @return 是否存在 */
bool CountingMultiSet::contains(int value)
{
    m_stats.totalQueries++;
    return m_counts.contains(value);
}

/** @brief Top-K频繁元素 @param k 返回数量 @return (元素,频率)列表 */
QVector<QPair<int, int>> CountingMultiSet::topK(int k) const
{
    QVector<QPair<int, int>> items;
    items.reserve(m_counts.size());
    for (auto it = m_counts.constBegin(); it != m_counts.constEnd(); ++it) {
        items.append({it.key(), it.value()});
    }

    std::sort(items.begin(), items.end(),
              [](const QPair<int, int>& a, const QPair<int, int>& b) {
                  return a.second > b.second;
              });

    if (items.size() > k) items.resize(k);
    return items;
}

/** @brief Jaccard相似度 @param other 另一个多重集合 @return 相似度 */
double CountingMultiSet::jaccardSimilarity(const CountingMultiSet& other) const
{
    if (m_counts.isEmpty() && other.m_counts.isEmpty()) return 1.0;
    if (m_counts.isEmpty() || other.m_counts.isEmpty()) return 0.0;

    /* 计算交集大小(取最小频率)和并集大小(取最大频率) */
    int intersection = 0;
    int unionSum = 0;

    /* 遍历自身元素 */
    for (auto it = m_counts.constBegin(); it != m_counts.constEnd(); ++it) {
        int otherCount = other.m_counts.value(it.key(), 0);
        intersection += std::min(it.value(), otherCount);
        unionSum += std::max(it.value(), otherCount);
    }

    /* 遍历对方独有元素 */
    for (auto it = other.m_counts.constBegin(); it != other.m_counts.constEnd(); ++it) {
        if (!m_counts.contains(it.key())) {
            unionSum += it.value();
        }
    }

    return (unionSum > 0) ? static_cast<double>(intersection) / unionSum : 0.0;
}

/** @brief 清空集合 */
void CountingMultiSet::clear()
{
    m_counts.clear();
    m_stats.uniqueElements = 0;
    m_stats.totalCount = 0;
}

/** @brief 当前不同元素数 @return 唯一元素数 */
int CountingMultiSet::size() const { return m_counts.size(); }

/** @brief 元素总数(含重复) @return 总计数 */
int CountingMultiSet::totalCount() const
{
    int total = 0;
    for (auto it = m_counts.constBegin(); it != m_counts.constEnd(); ++it)
        total += it.value();
    return total;
}

/** @brief 重置统计 */
void CountingMultiSet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
