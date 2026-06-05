/**
 * @file IndependentSet4.cpp
 * @brief 最大独立集求解器实现
 *
 * 寻找图中互不相邻的最大顶点子集(MIS)。
 * 实现:
 *   - 分支定界精确算法(小规模图)
 *   - 贪心近似算法(大规模图)
 *
 * 分支定界策略: 对每个顶点选择"选入"或"排除"，
 * 上界估计 = 当前独立集大小 + 剩余候选顶点数。
 */

#include "utils/graph93/IndependentSet4.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <numeric>
#include <vector>

/* ──────────────────── 构造/重置 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
IndependentSet4::IndependentSet4(QObject* parent)
    : QObject(parent)
    , m_maxSize(0)
{
}

/** @brief 重置统计数据 */
void IndependentSet4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_maxSize = 0;
    m_solution.clear();
}

/* ──────────────────── 主求解入口 ──────────────────── */

/**
 * @brief 求解最大独立集
 * @param adjacency 邻接表，adjacency[u] = [v1, v2, ...]
 * @return 最大独立集中的顶点编号列表
 *
 * 对小规模图(n<=40)使用分支定界，否则使用贪心近似。
 */
QVector<int> IndependentSet4::solve(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    if (n == 0) {
        emit setComputed(0, 0);
        return {};
    }

    /* 构建邻接集合(加速查找) */
    QVector<QVector<bool>> adjMatrix(n, QVector<bool>(n, false));
    for (int u = 0; u < n; ++u) {
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) {
                adjMatrix[u][v] = true;
            }
        }
    }

    /* 计算每个顶点的度数 */
    QVector<int> degree(n, 0);
    for (int u = 0; u < n; ++u) {
        degree[u] = adjacency[u].size();
    }

    QVector<int> result;

    if (n <= 40) {
        /* 小规模图: 分支定界精确求解 */
        result = branchAndBound(adjMatrix, degree, n);
    } else {
        /* 大规模图: 贪心近似 */
        result = greedyApprox(adjacency);
    }

    m_solution = result;
    m_maxSize = static_cast<int>(result.size());

    /* 更新统计 */
    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalSetsComputed;
    m_stats.totalVertices += n;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSetsComputed);

    emit setComputed(m_maxSize, n);
    return result;
}

/**
 * @brief 获取最大独立集大小
 * @return 最近一次求解的独立集大小
 */
int IndependentSet4::maxSize() const
{
    return m_maxSize;
}

/**
 * @brief 检查给定顶点集合是否为独立集
 * @param vertices 顶点集合
 * @param adjacency 邻接表
 * @return true表示集合内任意两点不相邻
 */
bool IndependentSet4::isIndependent(const QVector<int>& vertices,
                                     const QVector<QVector<int>>& adjacency) const
{
    for (int i = 0; i < vertices.size(); ++i) {
        for (int j = i + 1; j < vertices.size(); ++j) {
            int u = vertices[i];
            int v = vertices[j];
            /* 检查u和v是否相邻 */
            if (u >= 0 && u < adjacency.size()) {
                for (int neighbor : adjacency[u]) {
                    if (neighbor == v) return false;
                }
            }
        }
    }
    return true;
}

/**
 * @brief 贪心近似求解最大独立集
 * @param adjacency 邻接表
 * @return 近似最大独立集中的顶点列表
 *
 * 策略: 每次选度数最小的顶点加入独立集，
 * 然后移除该顶点及其所有邻居。重复直到图为空。
 * 时间复杂度 O(V^2)。
 */
QVector<int> IndependentSet4::greedyApprox(const QVector<QVector<int>>& adjacency)
{
    int n = adjacency.size();
    if (n == 0) return {};

    QVector<bool> available(n, true);
    QVector<int> degree(n, 0);
    for (int u = 0; u < n; ++u) {
        degree[u] = adjacency[u].size();
    }

    QVector<int> independentSet;

    while (true) {
        /* 在可用顶点中找度数最小的 */
        int best = -1;
        int minDeg = std::numeric_limits<int>::max();

        for (int v = 0; v < n; ++v) {
            if (!available[v]) continue;
            /* 重新计算度数(仅计算可用邻居) */
            int availDeg = 0;
            for (int neighbor : adjacency[v]) {
                if (available[neighbor]) ++availDeg;
            }

            if (availDeg < minDeg) {
                minDeg = availDeg;
                best = v;
            }
        }

        if (best < 0) break;

        /* 将该顶点加入独立集 */
        independentSet.append(best);
        available[best] = false;

        /* 移除其所有邻居 */
        for (int neighbor : adjacency[best]) {
            if (neighbor >= 0 && neighbor < n) {
                available[neighbor] = false;
            }
        }
    }

    return independentSet;
}

/* ──────────────────── 分支定界 ──────────────────── */

/**
 * @brief 分支定界精确求解MIS
 * @param adjMatrix 邻接矩阵
 * @param degree 度数数组
 * @param n 顶点数
 * @return 精确最大独立集
 *
 * 按顶点序依次决定"选入"或"排除"。
 * 上界 = 当前独立集大小 + 剩余候选数。
 * 若上界 <= 当前最优则剪枝。
 */
QVector<int> IndependentSet4::branchAndBound(
    const QVector<QVector<bool>>& adjMatrix,
    const QVector<int>& degree, int n)
{
    QVector<int> bestSet;
    int bestSize = 0;

    /* 按度数升序排列(低度数顶点更容易被选入) */
    QVector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return degree[a] < degree[b];
    });

    /* 顶点在排序中的位置 */
    QVector<int> pos(n);
    for (int i = 0; i < n; ++i) {
        pos[order[i]] = i;
    }

    QVector<int> currentSet;
    QVector<bool> excluded(n, false);

    /* 递归分支定界 */
    std::function<void(int)> dfs = [&](int idx) {
        /* 上界检查: 当前大小 + 剩余候选 <= 最优则剪枝 */
        int remaining = 0;
        for (int i = idx; i < n; ++i) {
            if (!excluded[order[i]]) ++remaining;
        }
        if (static_cast<int>(currentSet.size()) + remaining <= bestSize) return;

        /* 找下一个候选顶点 */
        while (idx < n && excluded[order[idx]]) ++idx;
        if (idx >= n) {
            if (static_cast<int>(currentSet.size()) > bestSize) {
                bestSize = static_cast<int>(currentSet.size());
                bestSet = currentSet;
            }
            return;
        }

        int v = order[idx];

        /* 分支1: 选入v */
        currentSet.append(v);
        QVector<int> newlyExcluded;
        for (int i = 0; i < n; ++i) {
            if (!excluded[i] && adjMatrix[v][i] && i != v) {
                excluded[i] = true;
                newlyExcluded.append(i);
            }
        }

        dfs(idx + 1);

        /* 恢复: 撤销排除 */
        for (int u : newlyExcluded) {
            excluded[u] = false;
        }
        currentSet.removeLast();

        /* 分支2: 排除v */
        excluded[v] = true;
        dfs(idx + 1);
        excluded[v] = false;
    };

    dfs(0);
    return bestSet;
}
