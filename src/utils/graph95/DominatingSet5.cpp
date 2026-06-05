#include "DominatingSet5.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化支配集求解器
 * @param parent 父QObject对象指针
 */
DominatingSet5::DominatingSet5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 贪心近似求解最小支配集
 *
 * 每轮迭代选择能覆盖最多未支配节点的顶点加入支配集，
 * 直到所有顶点都被支配（自身在支配集中或有邻居在支配集中）。
 * 贪心近似比为O(ln n)，对大多数实际图接近最优。
 *
 * @param adjacency 邻接表表示的图，adjacency[i]为顶点i的邻居列表
 * @return 支配集中顶点的索引列表
 */
QVector<int> DominatingSet5::greedyApprox(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacency.size();
    if (n == 0) return {};

    QVector<bool> dominated(n, false);  ///< 标记顶点是否已被支配
    QVector<int> dominatingSet;          ///< 支配集结果

    /// 重复直到所有顶点都被支配
    while (true) {
        /// 检查是否所有顶点都已被支配
        bool allDominated = true;
        for (int i = 0; i < n; ++i) {
            if (!dominated[i]) { allDominated = false; break; }
        }
        if (allDominated) break;

        /// 贪心选择：找到能新支配最多顶点的候选
        int bestVertex = -1;
        int bestCoverage = 0;

        for (int v = 0; v < n; ++v) {
            if (dominated[v]) continue;  ///< 跳过已支配的顶点（可选但不影响正确性）

            /// 计算选择v能新支配的顶点数
            int coverage = dominated[v] ? 0 : 1;  ///< v自身
            for (int neighbor : adjacency[v]) {
                if (neighbor >= 0 && neighbor < n && !dominated[neighbor]) {
                    ++coverage;
                }
            }

            if (coverage > bestCoverage) {
                bestCoverage = coverage;
                bestVertex = v;
            }
        }

        /// 加入支配集并标记支配状态
        if (bestVertex >= 0) {
            dominatingSet.append(bestVertex);
            dominated[bestVertex] = true;
            for (int neighbor : adjacency[bestVertex]) {
                if (neighbor >= 0 && neighbor < n) {
                    dominated[neighbor] = true;
                }
            }
        } else {
            break;  ///< 无法继续扩展（不应发生）
        }
    }

    /// 更新统计信息
    m_stats.totalSetsComputed++;
    m_stats.totalGraphsProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSetsComputed;

    emit setComputed(dominatingSet.size(), false);
    return dominatingSet;
}

/**
 * @brief 验证集合是否为图的有效支配集
 *
 * 检查给定集合是否满足支配集条件：
 * 图中每个顶点要么属于该集合，要么至少有一个邻居属于该集合。
 *
 * @param adjacency 邻接表
 * @param set 待验证的顶点集合
 * @return true如果是有效支配集，false否则
 */
bool DominatingSet5::validate(const QVector<QVector<int>>& adjacency,
                               const QVector<int>& set) const
{
    const int n = adjacency.size();
    QVector<bool> inSet(n, false);

    /// 标记支配集中的顶点
    for (int v : set) {
        if (v >= 0 && v < n) inSet[v] = true;
    }

    /// 检查每个顶点是否被支配
    for (int v = 0; v < n; ++v) {
        if (inSet[v]) continue;  ///< 自身在支配集中

        /// 检查是否有邻居在支配集中
        bool hasDominatingNeighbor = false;
        for (int neighbor : adjacency[v]) {
            if (neighbor >= 0 && neighbor < n && inSet[neighbor]) {
                hasDominatingNeighbor = true;
                break;
            }
        }
        if (!hasDominatingNeighbor) return false;
    }

    return true;
}

/**
 * @brief 获取当前统计数据
 * @return 包含计算次数、图处理数和平均耗时的Stats结构
 */
DominatingSet5::Stats DominatingSet5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void DominatingSet5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
