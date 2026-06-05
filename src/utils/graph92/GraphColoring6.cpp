/**
 * @file GraphColoring6.cpp
 * @brief 图顶点着色算法实现
 *
 * 提供三种着色策略:
 *   - greedy:  按顶点序贪心着色，O(V+E)
 *   - dsatur:  DSATUR启发式，优先着色饱和度最高的顶点
 *   - backtrack: 回溯法 + 约束传播，精确求解色数(小规模图)
 */

#include "utils/graph92/GraphColoring6.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <numeric>
#include <queue>
#include <vector>

/* ──────────────────── 构造/重置 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
GraphColoring6::GraphColoring6(QObject* parent)
    : QObject(parent)
{
}

/** @brief 重置统计数据 */
void GraphColoring6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_numColors = 0;
}

/* ──────────────────── 配置 ──────────────────── */

/**
 * @brief 设置着色算法
 * @param algorithm 算法名: "greedy", "dsatur", "backtrack"
 */
void GraphColoring6::setAlgorithm(const QString& algorithm)
{
    if (algorithm == "greedy" || algorithm == "dsatur" || algorithm == "backtrack") {
        m_algorithm = algorithm;
    }
}

/* ──────────────────── 主求解入口 ──────────────────── */

/**
 * @brief 对图执行顶点着色
 * @param adjacency 邻接表，adjacency[u] = [v1, v2, ...]
 * @return 每个顶点的颜色编号(从0开始)
 *
 * 根据配置选择对应算法，返回着色方案。
 */
QVector<int> GraphColoring6::color(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    if (n == 0) {
        emit coloringCompleted(0);
        return {};
    }

    QVector<int> result;

    if (m_algorithm == "greedy") {
        result = greedyColor(adjacency, n);
    } else if (m_algorithm == "backtrack") {
        result = backtrackColor(adjacency, n);
    } else {
        /* 默认使用 dsatur */
        result = dsaturColor(adjacency, n);
    }

    /* 计算使用的颜色数 */
    int maxC = 0;
    for (int c : result) {
        if (c > maxC) maxC = c;
    }
    m_numColors = maxC + 1;

    /* 更新统计 */
    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalGraphsColored;
    m_stats.totalColorsUsed += m_numColors;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalGraphsColored);

    emit coloringCompleted(m_numColors);
    return result;
}

/**
 * @brief 获取着色使用的颜色数
 * @return 最近一次着色使用的颜色数
 */
int GraphColoring6::numColors() const
{
    return m_numColors;
}

/**
 * @brief 验证着色合法性
 * @param coloring 着色方案
 * @param adjacency 邻接表
 * @return true表示没有相邻顶点使用相同颜色
 */
bool GraphColoring6::isValid(const QVector<int>& coloring,
                              const QVector<QVector<int>>& adjacency) const
{
    int n = adjacency.size();
    for (int u = 0; u < n; ++u) {
        if (u >= coloring.size()) return false;
        for (int v : adjacency[u]) {
            if (v < coloring.size() && u < v) {
                if (coloring[u] == coloring[v]) return false;
            }
        }
    }
    return true;
}

/* ──────────────────── 贪心着色 ──────────────────── */

/**
 * @brief 贪心着色算法
 * @param adj 邻接表
 * @param n 顶点数
 * @return 着色方案
 *
 * 按度数降序排列顶点，依次分配最小可用颜色。
 * 时间复杂度 O(V*logV + E)。
 */
QVector<int> GraphColoring6::greedyColor(
    const QVector<QVector<int>>& adj, int n)
{
    /* 按度数降序排列顶点(Welsh-Powell策略) */
    QVector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return adj[a].size() > adj[b].size();
    });

    QVector<int> colors(n, -1);

    for (int v : order) {
        /* 收集邻居已使用的颜色 */
        QVector<bool> used(n + 1, false);
        for (int neighbor : adj[v]) {
            if (neighbor < n && colors[neighbor] >= 0) {
                if (colors[neighbor] < used.size()) {
                    used[colors[neighbor]] = true;
                }
            }
        }

        /* 分配最小可用颜色 */
        int c = 0;
        while (c < used.size() && used[c]) ++c;
        colors[v] = c;
    }

    return colors;
}

/* ──────────────────── DSATUR 着色 ──────────────────── */

/**
 * @brief DSATUR启发式着色
 * @param adj 邻接表
 * @param n 顶点数
 * @return 着色方案
 *
 * 每步选择饱和度(邻居中不同颜色数)最高的未着色顶点，
 * 若平局则选度数最大的。贪心分配最小可用颜色。
 */
QVector<int> GraphColoring6::dsaturColor(
    const QVector<QVector<int>>& adj, int n)
{
    QVector<int> colors(n, -1);
    QVector<int> saturation(n, 0);
    QVector<bool> colored(n, false);

    for (int step = 0; step < n; ++step) {
        /* 选择饱和度最高的未着色顶点 */
        int best = -1;
        int bestSat = -1;
        int bestDeg = -1;

        for (int v = 0; v < n; ++v) {
            if (colored[v]) continue;
            int deg = static_cast<int>(adj[v].size());
            if (saturation[v] > bestSat ||
                (saturation[v] == bestSat && deg > bestDeg)) {
                bestSat = saturation[v];
                bestDeg = deg;
                best = v;
            }
        }

        if (best < 0) break;

        /* 贪心分配颜色 */
        QVector<bool> used(n + 1, false);
        for (int neighbor : adj[best]) {
            if (neighbor < n && colored[neighbor] && colors[neighbor] >= 0) {
                if (colors[neighbor] < static_cast<int>(used.size())) {
                    used[colors[neighbor]] = true;
                }
            }
        }

        int c = 0;
        while (c < static_cast<int>(used.size()) && used[c]) ++c;
        colors[best] = c;
        colored[best] = true;

        /* 更新邻居的饱和度 */
        for (int neighbor : adj[best]) {
            if (neighbor < n && !colored[neighbor]) {
                /* 检查邻居的邻居中是否已有颜色c */
                bool alreadyHasColor = false;
                for (int nn : adj[neighbor]) {
                    if (nn < n && colored[nn] && colors[nn] == c && nn != best) {
                        alreadyHasColor = true;
                        break;
                    }
                }
                if (!alreadyHasColor) {
                    ++saturation[neighbor];
                }
            }
        }
    }

    return colors;
}

/* ──────────────────── 回溯着色 ──────────────────── */

/**
 * @brief 回溯法精确着色
 * @param adj 邻接表
 * @param n 顶点数
 * @return 着色方案
 *
 * 先用DSATUR获得上界，再从上界向下二分尝试。
 * 对每个候选颜色数k，使用回溯+前向检查验证可行性。
 * 限制最大搜索节点数以避免超时。
 */
QVector<int> GraphColoring6::backtrackColor(
    const QVector<QVector<int>>& adj, int n)
{
    /* 先用DSATUR得到上界 */
    QVector<int> upper = dsaturColor(adj, n);
    int k = 0;
    for (int c : upper) {
        if (c > k) k = c;
    }
    ++k; /* 颜色数 = 最大颜色编号 + 1 */

    /* 尝试用更少颜色(从k-1往下搜索，最多尝试5轮) */
    QVector<int> best = upper;
    int bestK = k;
    const int maxRetries = qMin(k - 1, 5);

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        int targetK = k - 1 - attempt;
        if (targetK < 1) break;

        QVector<int> colors(n, -1);
        m_backtrackNodes = 0;
        const int nodeLimit = 500000;

        if (backtrackHelper(adj, n, 0, targetK, colors, nodeLimit)) {
            best = colors;
            bestK = targetK;
        }
    }

    return best;
}

/**
 * @brief 回溯递归核心
 * @param adj 邻接表
 * @param n 顶点数
 * @param vertex 当前处理的顶点
 * @param maxColors 允许的最大颜色数
 * @param colors 当前着色方案
 * @param nodeLimit 搜索节点上限
 * @return true表示找到合法着色
 */
bool GraphColoring6::backtrackHelper(
    const QVector<QVector<int>>& adj, int n,
    int vertex, int maxColors,
    QVector<int>& colors, int nodeLimit)
{
    if (vertex >= n) return true;
    ++m_backtrackNodes;
    if (m_backtrackNodes > nodeLimit) return false;

    /* 尝试每种颜色 */
    for (int c = 0; c < maxColors; ++c) {
        /* 前向检查: 检查与已着色邻居是否冲突 */
        bool conflict = false;
        for (int neighbor : adj[vertex]) {
            if (neighbor < vertex && colors[neighbor] == c) {
                conflict = true;
                break;
            }
        }

        if (!conflict) {
            colors[vertex] = c;

            /* 前向检查: 验证未着色邻居是否仍有可用颜色 */
            bool feasible = true;
            for (int neighbor : adj[vertex]) {
                if (neighbor > vertex) {
                    /* 检查该邻居是否还有可用颜色 */
                    bool hasAvail = false;
                    for (int cc = 0; cc < maxColors; ++cc) {
                        bool adjConflict = false;
                        for (int nn : adj[neighbor]) {
                            if (nn < n && colors[nn] == cc) {
                                adjConflict = true;
                                break;
                            }
                        }
                        if (!adjConflict) {
                            hasAvail = true;
                            break;
                        }
                    }
                    if (!hasAvail) {
                        feasible = false;
                        break;
                    }
                }
            }

            if (feasible && backtrackHelper(adj, n, vertex + 1, maxColors, colors, nodeLimit)) {
                return true;
            }

            colors[vertex] = -1;
        }
    }

    return false;
}
