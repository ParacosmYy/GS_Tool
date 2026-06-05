/**
 * @file GraphColoring3.cpp
 * @brief 图着色增强实现 — DSATUR/回溯/贪心/色数下界
 */

#include "utils/graph44/GraphColoring3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
GraphColoring3::GraphColoring3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置图结构 @param n 顶点数 @param edges 边列表 */
void GraphColoring3::setGraph(int n, const QVector<QPair<int, int>>& edges)
{
    m_n = qMax(0, n);
    m_adj.assign(m_n, QVector<int>());
    for (const auto& edge : edges) {
        int u = edge.first;
        int v = edge.second;
        if (u >= 0 && u < m_n && v >= 0 && v < m_n && u != v) {
            m_adj[u].append(v);
            m_adj[v].append(u);
        }
    }
}

/** @brief 贪心着色(按度降序) @return 每个顶点的颜色编号 */
QVector<int> GraphColoring3::greedyColor()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> color(m_n, -1);
    if (m_n == 0) return color;

    /* 按度数降序排列顶点 — Welch-Powell启发式 */
    QVector<int> order(m_n);
    for (int i = 0; i < m_n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_adj[a].size() > m_adj[b].size();
    });

    /* 逐顶点贪心着色 */
    for (int idx : order) {
        QVector<bool> used(m_n, false);
        for (int neighbor : m_adj[idx]) {
            if (color[neighbor] >= 0 && color[neighbor] < m_n) {
                used[color[neighbor]] = true;
            }
        }
        int c = 0;
        while (c < m_n && used[c]) ++c;
        color[idx] = c;
    }

    m_stats.totalColorings++;
    m_stats.totalVerticesProcessed += m_n;
    int nc = numColors(color);
    if (m_stats.bestColors == 0 || nc < m_stats.bestColors) {
        m_stats.bestColors = nc;
    }
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalColorings));

    emit coloringComplete(nc);
    return color;
}

/** @brief DSATUR着色(饱和度优先) @return 每个顶点的颜色编号 */
QVector<int> GraphColoring3::dsaturColor()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> color(m_n, -1);
    if (m_n == 0) return color;

    /* 饱和度: 每个顶点邻居使用的不同颜色数 */
    QVector<int> saturation(m_n, 0);
    QVector<QVector<bool>> neighborColors(m_n, QVector<bool>(m_n, false));

    for (int step = 0; step < m_n; ++step) {
        /* 选择饱和度最大的未着色顶点, 平局选度最大的 */
        int best = -1;
        for (int v = 0; v < m_n; ++v) {
            if (color[v] >= 0) continue;
            if (best < 0
                || saturation[v] > saturation[best]
                || (saturation[v] == saturation[best]
                    && m_adj[v].size() > m_adj[best].size())) {
                best = v;
            }
        }
        if (best < 0) break;

        /* 找最小可用颜色 */
        int c = 0;
        while (c < m_n && neighborColors[best][c]) ++c;
        color[best] = c;

        /* 更新邻居的饱和度信息 */
        for (int neighbor : m_adj[best]) {
            if (color[neighbor] < 0 && !neighborColors[neighbor][c]) {
                neighborColors[neighbor][c] = true;
                ++saturation[neighbor];
            }
        }
    }

    m_stats.totalColorings++;
    m_stats.totalVerticesProcessed += m_n;
    int nc = numColors(color);
    if (m_stats.bestColors == 0 || nc < m_stats.bestColors) {
        m_stats.bestColors = nc;
    }
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalColorings));

    emit coloringComplete(nc);
    return color;
}

/** @brief 回溯着色(精确算法) @param maxColors 最大颜色数 @return 每个顶点的颜色编号 */
QVector<int> GraphColoring3::backtrackingColor(int maxColors)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> color(m_n, -1);
    if (m_n == 0 || maxColors <= 0) return color;

    /* 回溯搜索: 逐顶点尝试每种颜色 */
    bool found = false;

    /* 内部递归lambda */
    QVector<int> order(m_n);
    for (int i = 0; i < m_n; ++i) order[i] = i;
    /* 按度降序排列以加速剪枝 */
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_adj[a].size() > m_adj[b].size();
    });

    /* 用栈模拟递归避免栈溢出 */
    QVector<int> colorMap(m_n, -1);
    int depth = 0;

    while (depth >= 0 && !found) {
        int v = order[depth];
        int nextColor = colorMap[v] + 1;

        /* 寻找下一个可用颜色 */
        while (nextColor < maxColors) {
            bool conflict = false;
            for (int nb : m_adj[v]) {
                if (color[nb] == nextColor) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) break;
            ++nextColor;
        }

        if (nextColor < maxColors) {
            color[v] = nextColor;
            colorMap[v] = nextColor;
            ++depth;
            if (depth >= m_n) {
                found = true;
            }
        } else {
            /* 回溯 */
            color[v] = -1;
            colorMap[v] = -1;
            --depth;
        }
    }

    if (!found) {
        color.fill(-1);
    }

    m_stats.totalColorings++;
    m_stats.totalVerticesProcessed += m_n;
    int nc = found ? numColors(color) : maxColors + 1;
    if (m_stats.bestColors == 0 || (found && nc < m_stats.bestColors)) {
        m_stats.bestColors = nc;
    }
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalColorings));

    emit coloringComplete(found ? nc : -1);
    return color;
}

/** @brief 计算色数下界(最大团近似) @return 下界值 */
int GraphColoring3::chromaticLowerBound() const
{
    if (m_n == 0) return 0;

    /* 贪心最大团近似 — 染色数的下界 >= 最大团大小 */
    int maxClique = 1;

    /* 简单近似: 找最大度+1 */
    int maxDeg = 0;
    for (int v = 0; v < m_n; ++v) {
        if (m_adj[v].size() > maxDeg) {
            maxDeg = m_adj[v].size();
        }
    }

    /* Bron-Kerbosch简化的团搜索 */
    for (int start = 0; start < m_n; ++start) {
        QVector<bool> inClique(m_n, false);
        inClique[start] = true;
        int cliqueSize = 1;

        for (int v = 0; v < m_n; ++v) {
            if (inClique[v]) continue;
            bool canAdd = true;
            for (int u = 0; u < m_n; ++u) {
                if (inClique[u] && !m_adj[v].contains(u)) {
                    canAdd = false;
                    break;
                }
            }
            if (canAdd) {
                inClique[v] = true;
                ++cliqueSize;
            }
        }
        maxClique = qMax(maxClique, cliqueSize);
    }

    return qMax(1, maxClique);
}

/** @brief 计算着色方案使用的颜色数 @param coloring 着色方案 @return 颜色数 */
int GraphColoring3::numColors(const QVector<int>& coloring) const
{
    int maxC = -1;
    for (int c : coloring) {
        if (c > maxC) maxC = c;
    }
    return maxC + 1;
}

/** @brief 重置统计 */
void GraphColoring3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
