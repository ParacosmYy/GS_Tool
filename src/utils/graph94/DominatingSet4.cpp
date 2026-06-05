/**
 * @file DominatingSet4.cpp
 * @brief 最小支配集求解器实现
 *
 * 寻找图的最小支配集: 每个顶点要么在集合中，要么与集合中
 * 某个顶点相邻。
 *
 * 实现:
 *   - 贪心近似算法: 每次选支配最多未支配顶点的顶点
 *   - 精确验证: 检查给定集合是否为支配集
 *   - 未支配顶点查询
 *
 * 贪心近似比: O(ln n) 近似保证。
 */

#include "utils/graph94/DominatingSet4.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <numeric>
#include <vector>

/* ──────────────────── 构造/重置 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
DominatingSet4::DominatingSet4(QObject* parent)
    : QObject(parent)
    , m_setSize(0)
{
}

/** @brief 重置统计数据 */
void DominatingSet4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_setSize = 0;
    m_solution.clear();
}

/* ──────────────────── 主求解入口 ──────────────────── */

/**
 * @brief 使用贪心近似求解最小支配集
 * @param adjacency 邻接表，adjacency[u] = [v1, v2, ...]
 * @return 支配集中的顶点编号列表
 *
 * 贪心策略: 每轮选择能支配最多未支配顶点的顶点。
 * 包含自身和其所有邻居中尚未被支配的顶点数作为收益。
 * 重复直到所有顶点被支配。
 */
QVector<int> DominatingSet4::greedySolve(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    if (n == 0) {
        emit setComputed(0, 0);
        return {};
    }

    /* 构建邻接集合(包含自身) */
    QVector<QVector<int>> closedNeighbors(n);
    for (int u = 0; u < n; ++u) {
        closedNeighbors[u].append(u);
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n && v != u) {
                closedNeighbors[u].append(v);
            }
        }
    }

    QVector<bool> dominated(n, false);
    QVector<int> dominatingSet;

    while (true) {
        /* 统计尚未被支配的顶点数 */
        int undominatedCount = 0;
        for (int v = 0; v < n; ++v) {
            if (!dominated[v]) ++undominatedCount;
        }
        if (undominatedCount == 0) break;

        /* 选择支配最多未支配顶点的顶点 */
        int best = -1;
        int bestGain = 0;

        for (int u = 0; u < n; ++u) {
            int gain = 0;
            for (int v : closedNeighbors[u]) {
                if (!dominated[v]) ++gain;
            }
            if (gain > bestGain) {
                bestGain = gain;
                best = u;
            }
        }

        if (best < 0 || bestGain == 0) break;

        /* 将选中的顶点加入支配集 */
        dominatingSet.append(best);

        /* 标记其支配的所有顶点 */
        for (int v : closedNeighbors[best]) {
            dominated[v] = true;
        }
    }

    /* 局部优化: 尝试移除冗余顶点 */
    dominatingSet = removeRedundant(dominatingSet, adjacency, n);

    m_solution = dominatingSet;
    m_setSize = static_cast<int>(dominatingSet.size());

    /* 更新统计 */
    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalSetsComputed;
    m_stats.totalVertices += n;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSetsComputed);

    emit setComputed(m_setSize, n);
    return dominatingSet;
}

/**
 * @brief 验证给定集合是否为支配集
 * @param set 候选支配集
 * @param adjacency 邻接表
 * @return true表示集合支配图中所有顶点
 */
bool DominatingSet4::isDominating(const QVector<int>& set,
                                   const QVector<QVector<int>>& adjacency) const
{
    int n = adjacency.size();
    if (n == 0) return true;

    QVector<bool> dominated(n, false);

    /* 标记集合中顶点及其邻居为已支配 */
    for (int u : set) {
        if (u < 0 || u >= n) continue;
        dominated[u] = true;
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) {
                dominated[v] = true;
            }
        }
    }

    /* 检查是否所有顶点都被支配 */
    for (int v = 0; v < n; ++v) {
        if (!dominated[v]) return false;
    }
    return true;
}

/**
 * @brief 获取支配集大小
 * @return 最近一次求解的支配集大小
 */
int DominatingSet4::setSize() const
{
    return m_setSize;
}

/**
 * @brief 获取未被支配的顶点列表
 * @param set 候选集合
 * @param adjacency 邻接表
 * @return 未被集合支配的顶点列表
 */
QVector<int> DominatingSet4::undominated(const QVector<int>& set,
                                          const QVector<QVector<int>>& adjacency) const
{
    int n = adjacency.size();
    QVector<bool> dominated(n, false);

    for (int u : set) {
        if (u < 0 || u >= n) continue;
        dominated[u] = true;
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) {
                dominated[v] = true;
            }
        }
    }

    QVector<int> result;
    for (int v = 0; v < n; ++v) {
        if (!dominated[v]) result.append(v);
    }
    return result;
}

/* ──────────────────── 私有方法 ──────────────────── */

/**
 * @brief 移除支配集中的冗余顶点
 * @param set 初始支配集
 * @param adjacency 邻接表
 * @param n 顶点数
 * @return 去冗余后的支配集
 *
 * 对支配集中每个顶点，检查移除后剩余集合是否仍为支配集。
 * 若仍为支配集则移除该顶点。
 */
QVector<int> DominatingSet4::removeRedundant(
    const QVector<int>& set,
    const QVector<QVector<int>>& adjacency, int n)
{
    QVector<int> result = set;

    /* 计算每个顶点被支配的次数(被多少个支配集中的顶点覆盖) */
    QVector<int> coverCount(n, 0);
    for (int u : result) {
        if (u < 0 || u >= n) continue;
        ++coverCount[u];
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) ++coverCount[v];
        }
    }

    /* 尝试移除每个顶点 */
    for (int i = result.size() - 1; i >= 0; --i) {
        int u = result[i];
        if (u < 0 || u >= n) continue;

        /* 检查移除u后是否所有顶点仍被支配 */
        bool canRemove = true;

        /* u自身的覆盖需要由邻居提供 */
        if (coverCount[u] <= 1) {
            /* u只被自己覆盖，移除后需要检查邻居是否覆盖 */
            canRemove = false;
            for (int v : adjacency[u]) {
                if (v >= 0 && v < n && coverCount[v] > 0) {
                    /* 邻居v被u覆盖，移除u后v仍需被覆盖 */
                    bool stillCovered = false;
                    for (int d : result) {
                        if (d == u) continue;
                        if (d == v) { stillCovered = true; break; }
                        for (int nn : adjacency[d]) {
                            if (nn == v) { stillCovered = true; break; }
                        }
                        if (stillCovered) break;
                    }
                    if (!stillCovered) { canRemove = false; break; }
                }
            }
        }

        /* 额外检查: u的私有邻居(仅被u覆盖的顶点) */
        if (canRemove) {
            /* u自身是否仍被覆盖 */
            bool selfCovered = false;
            for (int d : result) {
                if (d == u) continue;
                if (d == u) { selfCovered = true; break; }
                for (int nn : adjacency[d]) {
                    if (nn == u) { selfCovered = true; break; }
                }
                if (selfCovered) break;
            }
            if (!selfCovered && coverCount[u] <= 1) canRemove = false;
        }

        if (canRemove) {
            /* 执行移除: 更新覆盖计数 */
            --coverCount[u];
            for (int v : adjacency[u]) {
                if (v >= 0 && v < n) --coverCount[v];
            }
            result.removeAt(i);
        }
    }

    return result;
}
