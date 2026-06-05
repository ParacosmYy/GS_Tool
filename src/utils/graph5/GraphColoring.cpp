/**
 * @file GraphColoring.cpp
 * @brief 图顶点着色引擎实现 — 贪心+回溯算法
 */

#include "GraphColoring.h"

#include <QElapsedTimer>
#include <algorithm>
#include <set>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

GraphColoring::GraphColoring(QObject* parent)
    : QObject(parent)
{
}

GraphColoring::~GraphColoring() = default;

// ═══════════════════════════════════════════════════════════
// 图构建
// ═══════════════════════════════════════════════════════════

void GraphColoring::addEdge(int u, int v)
{
    if (u == v) return;  /* 忽略自环 */

    /* 扩展邻接表 */
    int maxV = std::max(u, v) + 1;
    if (maxV > m_adj.size()) {
        m_adj.resize(maxV);
    }

    /* 检查重复边 */
    for (int neighbor : m_adj[u]) {
        if (neighbor == v) return;
    }

    m_adj[u].append(v);
    m_adj[v].append(u);
    m_edges.append({u, v});
}

void GraphColoring::clear()
{
    m_adj.clear();
    m_edges.clear();
}

// ═══════════════════════════════════════════════════════════
// 贪心着色(Welsh-Powell)
// ═══════════════════════════════════════════════════════════

GraphColoring::ColoringResult GraphColoring::colorGreedy()
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_adj.size();
    if (n == 0) {
        emit coloringCompleted(0, true);
        return {{}, 0, true};
    }

    ColoringResult result;
    result.colors.resize(n, -1);

    /* 按度数降序排列顶点 */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) {
        order[i] = i;
    }
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_adj[a].size() > m_adj[b].size();
    });

    int maxColor = 0;

    for (int v : order) {
        /* 收集已使用的相邻颜色 */
        std::set<int> usedColors;
        for (int neighbor : m_adj[v]) {
            if (result.colors[neighbor] >= 0) {
                usedColors.insert(result.colors[neighbor]);
            }
        }

        /* 找最小可用颜色 */
        int color = 0;
        while (usedColors.count(color)) {
            ++color;
        }

        result.colors[v] = color;
        maxColor = std::max(maxColor, color);
    }

    result.colorCount = maxColor + 1;
    result.valid = true;

    /* 更新统计 */
    m_stats.totalColorings++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalColorings;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit coloringCompleted(result.colorCount, true);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 回溯精确着色
// ═══════════════════════════════════════════════════════════

GraphColoring::ColoringResult GraphColoring::colorBacktrack(int maxColors)
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_adj.size();
    ColoringResult result;
    result.colors.resize(n, -1);

    if (n == 0 || maxColors <= 0) {
        result.valid = (n == 0);
        emit coloringCompleted(0, result.valid);
        return result;
    }

    QVector<int> colors(n, -1);

    if (backtrackHelper(colors, 0, maxColors)) {
        result.colors = colors;
        result.colorCount = 0;
        for (int c : colors) {
            result.colorCount = std::max(result.colorCount, c + 1);
        }
        result.valid = true;
    } else {
        result.colorCount = maxColors;
        result.valid = false;
    }

    /* 更新统计 */
    m_stats.totalColorings++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalColorings;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit coloringCompleted(result.colorCount, result.valid);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

int GraphColoring::vertexCount() const
{
    return m_adj.size();
}

int GraphColoring::edgeCount() const
{
    return m_edges.size();
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

GraphColoring::Stats GraphColoring::stats() const
{
    return m_stats;
}

void GraphColoring::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

bool GraphColoring::isSafe(int v, int c, const QVector<int>& colors) const
{
    for (int neighbor : m_adj[v]) {
        if (colors[neighbor] == c) {
            return false;
        }
    }
    return true;
}

bool GraphColoring::backtrackHelper(QVector<int>& colors, int vertex,
                                     int maxColors)
{
    const int n = m_adj.size();

    /* 所有顶点都已着色 */
    if (vertex >= n) {
        return true;
    }

    /* 尝试每种颜色 */
    for (int c = 0; c < maxColors; ++c) {
        if (isSafe(vertex, c, colors)) {
            colors[vertex] = c;

            if (backtrackHelper(colors, vertex + 1, maxColors)) {
                return true;
            }

            colors[vertex] = -1;  /* 回溯 */
        }
    }

    return false;  /* 无解 */
}
