#include "GraphColoring10.h"
#include <QElapsedTimer>
#include <QSet>
#include <algorithm>
#include <QtMath>

/**
 * @brief 构造函数，初始化图着色引擎v10
 * @param parent 父对象指针
 */
GraphColoring10::GraphColoring10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GraphColoring10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 贪心法图着色
 *
 * 按顶点度数降序排列，依次分配最小可用颜色。
 * 时间复杂度O(V^2)。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 各顶点分配的颜色编号
 */
QVector<int> GraphColoring10::greedyColoring(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> colors(n, -1);

    if (n == 0) {
        emit coloringCompleted(0);
        return colors;
    }

    /* 按度数降序排列 */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;

    QVector<int> degree(n, 0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < qMin(n, adjacencyMatrix[i].size()); ++j) {
            if (adjacencyMatrix[i][j] != 0) degree[i]++;
        }
    }

    std::sort(order.begin(), order.end(), [&degree](int a, int b) {
        return degree[a] > degree[b];
    });

    int maxColor = 0;
    for (int idx = 0; idx < n; ++idx) {
        int v = order[idx];
        QVector<bool> used(n + 1, false);

        for (int u = 0; u < n; ++u) {
            if (u < adjacencyMatrix[v].size() && adjacencyMatrix[v][u] != 0 && colors[u] >= 0) {
                if (colors[u] < used.size()) used[colors[u]] = true;
            }
        }

        int c = 0;
        while (c < used.size() && used[c]) c++;
        colors[v] = c;
        if (c + 1 > maxColor) maxColor = c + 1;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalColoringOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColoringOps;

    emit coloringCompleted(maxColor);
    return colors;
}

/**
 * @brief DSATUR算法着色（饱和度优先）
 *
 * 每次选择饱和度（相邻已用颜色种类数）最高的未着色顶点，
 * 如果饱和度相同则选择度数最大的。
 * 通常比简单贪心得到更少的颜色数。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 各顶点分配的颜色编号
 */
QVector<int> GraphColoring10::dsaturColoring(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> colors(n, -1);

    if (n == 0) {
        emit coloringCompleted(0);
        return colors;
    }

    /* 每个顶点的相邻颜色集合 */
    QVector<QSet<int>> neighborColors(n);
    QVector<int> degree(n, 0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < qMin(n, adjacencyMatrix[i].size()); ++j) {
            if (adjacencyMatrix[i][j] != 0) degree[i]++;
        }
    }

    int maxColor = 0;
    for (int step = 0; step < n; ++step) {
        /* 选择饱和度最高（且度数最大）的未着色顶点 */
        int best = -1;
        int bestSat = -1;
        int bestDeg = -1;

        for (int v = 0; v < n; ++v) {
            if (colors[v] >= 0) continue;
            int sat = neighborColors[v].size();
            if (sat > bestSat || (sat == bestSat && degree[v] > bestDeg)) {
                bestSat = sat;
                bestDeg = degree[v];
                best = v;
            }
        }

        if (best < 0) break;

        /* 分配最小可用颜色 */
        int c = 0;
        while (neighborColors[best].contains(c)) c++;
        colors[best] = c;
        if (c + 1 > maxColor) maxColor = c + 1;

        /* 更新邻居的饱和度 */
        for (int u = 0; u < n; ++u) {
            if (u < adjacencyMatrix[best].size() && adjacencyMatrix[best][u] != 0) {
                neighborColors[u].insert(c);
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalColoringOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColoringOps;

    emit coloringCompleted(maxColor);
    return colors;
}

/**
 * @brief 回溯法求精确色数
 *
 * 从1种颜色开始尝试，逐步增加直到找到合法着色。
 * 时间复杂度最坏为O(k^V)，适合小规模图。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @param maxColors 最大颜色数上限
 * @return 着色方案，无解则返回空
 */
QVector<int> GraphColoring10::exactColoring(const QVector<QVector<int>>& adjacencyMatrix, int maxColors)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    if (n == 0 || maxColors <= 0) {
        emit coloringCompleted(0);
        return {};
    }

    QVector<int> colors(n, -1);
    bool found = false;

    std::function<bool(int)> backtrack = [&](int vertex) -> bool {
        if (vertex >= n) { found = true; return true; }
        for (int c = 0; c < maxColors; ++c) {
            bool safe = true;
            for (int u = 0; u < n && safe; ++u) {
                if (u < adjacencyMatrix[vertex].size() &&
                    adjacencyMatrix[vertex][u] != 0 && colors[u] == c) {
                    safe = false;
                }
            }
            if (!safe) continue;
            colors[vertex] = c;
            if (backtrack(vertex + 1)) return true;
            colors[vertex] = -1;
        }
        return false;
    };

    backtrack(0);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalColoringOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColoringOps;

    if (found) {
        int maxC = 0;
        for (int c : colors) { if (c + 1 > maxC) maxC = c + 1; }
        emit coloringCompleted(maxC);
        return colors;
    }

    emit coloringCompleted(0);
    return {};
}

/**
 * @brief 计算色数下界（团数估计）
 *
 * 使用贪心策略估计最大团的大小，作为色数的下界。
 * 色数 >= 最大团大小。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 色数下界值
 */
int GraphColoring10::computeChromaticLowerBound(const QVector<QVector<int>>& adjacencyMatrix)
{
    const int n = adjacencyMatrix.size();
    if (n == 0) return 0;

    /* 贪心最大团估计 */
    int maxClique = 1;

    for (int v = 0; v < n; ++v) {
        QVector<int> candidates;
        for (int u = 0; u < qMin(n, adjacencyMatrix[v].size()); ++u) {
            if (adjacencyMatrix[v][u] != 0 && u != v) candidates.append(u);
        }

        int cliqueSize = 1;
        QVector<int> inClique = {v};

        for (int c : candidates) {
            bool allAdj = true;
            for (int member : inClique) {
                if (c >= adjacencyMatrix[member].size() || adjacencyMatrix[member][c] == 0) {
                    allAdj = false;
                    break;
                }
            }
            if (allAdj) {
                inClique.append(c);
                cliqueSize++;
            }
        }
        if (cliqueSize > maxClique) maxClique = cliqueSize;
    }

    return maxClique;
}
