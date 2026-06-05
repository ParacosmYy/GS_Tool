#include "GraphColoring9.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化图着色引擎
 * @param parent 父对象指针
 */
GraphColoring9::GraphColoring9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GraphColoring9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_colorCount = 0;
}

/**
 * @brief 对图执行贪心着色（Welsh-Powell策略）
 *
 * 按顶点度数降序排列，依次为每个顶点分配最小可用颜色。
 * 该算法为近似算法，不保证最优色数，但时间复杂度为O(V^2)。
 *
 * @param adjacencyMatrix 邻接矩阵表示
 * @return 每个顶点的颜色编号
 */
QVector<int> GraphColoring9::greedyColoring(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> colors(n, -1);

    if (n == 0) {
        m_colorCount = 0;
        emit coloringCompleted(0);
        return colors;
    }

    /* 按度数降序排列顶点索引 */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;

    QVector<int> degree(n, 0);
    for (int i = 0; i < n; ++i) {
        if (adjacencyMatrix[i].size() != n) {
            emit coloringCompleted(0);
            return QVector<int>(n, -1);
        }
        for (int j = 0; j < n; ++j) {
            if (adjacencyMatrix[i][j] != 0) degree[i]++;
        }
    }

    std::sort(order.begin(), order.end(), [&degree](int a, int b) {
        return degree[a] > degree[b];
    });

    /* 贪心着色：为每个顶点分配最小可用颜色 */
    int maxColor = 0;
    for (int idx = 0; idx < n; ++idx) {
        int v = order[idx];
        QVector<bool> used(n + 1, false);

        for (int u = 0; u < n; ++u) {
            if (adjacencyMatrix[v][u] != 0 && colors[u] >= 0) {
                if (colors[u] < used.size()) used[colors[u]] = true;
            }
        }

        int c = 0;
        while (c < used.size() && used[c]) c++;
        colors[v] = c;
        if (c + 1 > maxColor) maxColor = c + 1;
    }

    m_colorCount = maxColor;
    m_stats.chromaticNumber = maxColor;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalColoringRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColoringRuns;

    emit coloringCompleted(maxColor);
    return colors;
}

/**
 * @brief 回溯法求解最优着色
 *
 * 尝试用不超过maxColors种颜色对图进行合法着色。
 * 回溯搜索所有可能分配，找到第一个合法方案即返回。
 * 时间复杂度最坏为O(maxColors^V)，适合小规模图。
 *
 * @param adjacencyMatrix 邻接矩阵表示
 * @param maxColors 最大允许颜色数
 * @return 最优着色方案，空向量表示无解
 */
QVector<int> GraphColoring9::optimalColoring(const QVector<QVector<int>>& adjacencyMatrix, int maxColors)
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

    /* 回溯辅助函数 */
    std::function<bool(int)> backtrack = [&](int vertex) -> bool {
        if (vertex >= n) {
            found = true;
            return true;
        }

        for (int c = 0; c < maxColors; ++c) {
            bool safe = true;
            for (int u = 0; u < n && safe; ++u) {
                if (adjacencyMatrix[vertex][u] != 0 && colors[u] == c) {
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
    m_stats.totalColoringRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColoringRuns;

    if (found) {
        int maxC = 0;
        for (int c : colors) { if (c + 1 > maxC) maxC = c + 1; }
        m_colorCount = maxC;
        m_stats.chromaticNumber = maxC;
        emit coloringCompleted(maxC);
        return colors;
    }

    emit coloringCompleted(0);
    return {};
}

/**
 * @brief 验证着色方案是否合法
 *
 * 检查任意两个相邻顶点是否分配了不同颜色。
 *
 * @param adjacencyMatrix 邻接矩阵
 * @param colors 着色方案
 * @return 是否合法
 */
bool GraphColoring9::validateColoring(const QVector<QVector<int>>& adjacencyMatrix,
                                       const QVector<int>& colors) const
{
    const int n = adjacencyMatrix.size();
    if (n == 0 || colors.size() != n) return false;

    for (int i = 0; i < n; ++i) {
        if (adjacencyMatrix[i].size() != n) return false;
        for (int j = i + 1; j < n; ++j) {
            if (adjacencyMatrix[i][j] != 0 && colors[i] == colors[j]) {
                return false;
            }
        }
    }
    return true;
}
