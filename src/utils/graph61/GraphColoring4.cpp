/**
 * @file GraphColoring4.cpp
 * @brief 图着色4 — DSATUR+回溯精确着色 实现
 *
 * 提供三种图着色算法：
 * 1. DSATUR — 基于饱和度启发式的近似着色
 * 2. Greedy — 顺序贪心着色
 * 3. Backtrack — 回溯法搜索精确色数
 */

#include "utils/graph61/GraphColoring4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
GraphColoring4::GraphColoring4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的邻接关系
 * @param n 顶点数量
 * @param edges 边列表，每条边为一对顶点索引
 */
void GraphColoring4::setGraph(int n, const QVector<QPair<int, int>>& edges)
{
    m_n = n;
    m_adj.resize(n);
    for (auto& row : m_adj) {
        row.clear();
    }

    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < n &&
            edge.second >= 0 && edge.second < n) {
            m_adj[edge.first].append(edge.second);
            m_adj[edge.second].append(edge.first);
        }
    }
}

/**
 * @brief DSATUR 算法着色
 *
 * 每次选择饱和度最高（相邻已用颜色种类最多）的未着色顶点，
 * 为其分配最小的可用颜色。这是一个有效的启发式算法。
 *
 * @return 长度为 n 的着色向量，color[i] 为顶点 i 的颜色编号
 */
QVector<int> GraphColoring4::colorDSATUR()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    QVector<int> colors(m_n, -1);
    QVector<int> saturation(m_n, 0); /* 每个顶点的饱和度 */
    QVector<bool> colored(m_n, false);

    for (int step = 0; step < m_n; ++step) {
        /* 选择饱和度最高的未着色顶点（平局选度数最大的） */
        int v = selectDSATUR(colors, saturation);

        /* 找出可用最小颜色 */
        QVector<bool> used(m_n + 1, false);
        for (int neighbor : m_adj[v]) {
            if (colors[neighbor] >= 0) {
                used[colors[neighbor]] = true;
            }
        }
        int c = 0;
        while (used[c]) c++;
        colors[v] = c;

        /* 更新邻居的饱和度 */
        for (int neighbor : m_adj[v]) {
            if (colors[neighbor] < 0) {
                /* 检查邻居是否新增了一种相邻颜色 */
                QVector<bool> neighborColors(m_n + 1, false);
                for (int nn : m_adj[neighbor]) {
                    if (colors[nn] >= 0) {
                        neighborColors[colors[nn]] = true;
                    }
                }
                int sat = 0;
                for (bool b : neighborColors) {
                    if (b) sat++;
                }
                saturation[neighbor] = sat;
            }
        }
    }

    m_chromatic = numColors();
    m_bestColoring = colors;

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalColorings++;
    m_stats.totalVertices += m_n;
    m_stats.chromaticNumber = m_chromatic;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringCompleted(m_chromatic, m_n);
    return colors;
}

/**
 * @brief 贪心顺序着色
 *
 * 按顶点顺序依次为每个顶点分配最小的可用颜色。
 * 时间复杂度 O(V+E)。
 *
 * @return 着色向量
 */
QVector<int> GraphColoring4::colorGreedy()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    QVector<int> colors(m_n, -1);

    for (int v = 0; v < m_n; ++v) {
        QVector<bool> used(m_n + 1, false);
        for (int neighbor : m_adj[v]) {
            if (colors[neighbor] >= 0) {
                used[colors[neighbor]] = true;
            }
        }
        int c = 0;
        while (used[c]) c++;
        colors[v] = c;
    }

    m_chromatic = numColors();
    m_bestColoring = colors;

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalColorings++;
    m_stats.totalVertices += m_n;
    m_stats.chromaticNumber = m_chromatic;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringCompleted(m_chromatic, m_n);
    return colors;
}

/**
 * @brief 回溯法精确着色
 *
 * 从1种颜色开始逐步增加，用回溯法判断是否存在合法着色。
 * 找到最小颜色数即为色数。
 *
 * @return 最优着色向量
 */
QVector<int> GraphColoring4::colorBacktrack()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    /* 先用DSATUR估计上界 */
    QVector<int> dsaturResult = colorDSATUR();
    int upperBound = 0;
    for (int c : dsaturResult) {
        upperBound = qMax(upperBound, c + 1);
    }

    /* 从1到上界逐一尝试 */
    for (int maxC = 1; maxC <= upperBound; ++maxC) {
        QVector<int> colors(m_n, -1);
        if (backtrackTry(colors, 0, maxC)) {
            m_chromatic = maxC;
            m_bestColoring = colors;

            /* 统计更新 */
            qint64 elapsed = timer.elapsed();
            m_timeSum += elapsed;
            m_stats.totalColorings++;
            m_stats.totalVertices += m_n;
            m_stats.chromaticNumber = m_chromatic;
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

            emit coloringCompleted(m_chromatic, m_n);
            return colors;
        }
    }

    /* 不应到达此处 */
    m_bestColoring = dsaturResult;
    m_chromatic = upperBound;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalColorings++;
    m_stats.totalVertices += m_n;
    m_stats.chromaticNumber = m_chromatic;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringCompleted(m_chromatic, m_n);
    return dsaturResult;
}

/**
 * @brief 验证着色方案是否合法
 * @param coloring 待验证的着色向量
 * @return 是否为合法着色（相邻顶点颜色不同）
 */
bool GraphColoring4::isValidColoring(const QVector<int>& coloring) const
{
    if (coloring.size() != m_n) return false;

    for (int v = 0; v < m_n; ++v) {
        for (int neighbor : m_adj[v]) {
            if (coloring[v] == coloring[neighbor]) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief 返回当前着色使用的颜色数
 * @return 颜色数
 */
int GraphColoring4::numColors() const
{
    if (m_bestColoring.isEmpty()) return 0;
    int maxC = 0;
    for (int c : m_bestColoring) {
        maxC = qMax(maxC, c + 1);
    }
    return maxC;
}

/**
 * @brief 重置统计信息
 */
void GraphColoring4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief DSATUR 顶点选择策略
 *
 * 选择饱和度最高的未着色顶点。
 * 平局时选择度数最大的顶点。
 *
 * @param colors 当前着色状态
 * @param saturation 饱和度向量
 * @return 选中的顶点索引
 */
int GraphColoring4::selectDSATUR(const QVector<int>& colors,
                                   const QVector<int>& saturation) const
{
    int bestV = -1;
    int bestSat = -1;
    int bestDeg = -1;

    for (int v = 0; v < m_n; ++v) {
        if (colors[v] >= 0) continue;
        int sat = saturation[v];
        int deg = m_adj[v].size();
        if (sat > bestSat || (sat == bestSat && deg > bestDeg)) {
            bestSat = sat;
            bestDeg = deg;
            bestV = v;
        }
    }
    return bestV;
}

/**
 * @brief 回溯法尝试用 maxColors 种颜色着色
 * @param colors 着色状态
 * @param vertex 当前处理的顶点索引
 * @param maxColors 允许的最大颜色数
 * @return 是否成功找到合法着色
 */
bool GraphColoring4::backtrackTry(QVector<int>& colors, int vertex, int maxColors)
{
    if (vertex >= m_n) {
        return true; /* 所有顶点已着色 */
    }

    for (int c = 0; c < maxColors; ++c) {
        /* 检查颜色 c 是否与邻居冲突 */
        bool conflict = false;
        for (int neighbor : m_adj[vertex]) {
            if (colors[neighbor] == c) {
                conflict = true;
                break;
            }
        }

        if (!conflict) {
            colors[vertex] = c;
            if (backtrackTry(colors, vertex + 1, maxColors)) {
                return true;
            }
            colors[vertex] = -1; /* 回溯 */
        }
    }
    return false;
}
