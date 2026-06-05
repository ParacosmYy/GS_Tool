/**
 * @file GraphColoring.cpp
 * @brief 图着色实现
 */

#include "GraphColoring.h"
#include <QElapsedTimer>
#include <algorithm>
#include <set>

GraphColoring::GraphColoring(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<int> GraphColoring::greedy(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    QVector<int> colors(n, -1);

    for (int v = 0; v < n; ++v) {
        QSet<int> usedColors;
        for (int u : adjacency[v]) {
            if (u >= 0 && u < n && colors[u] >= 0)
                usedColors.insert(colors[u]);
        }

        int c = 0;
        while (usedColors.contains(c)) c++;
        colors[v] = c;
    }

    m_stats.totalColored++;
    m_stats.totalColors += colorCount(colors);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColored;

    emit coloringCompleted(n, colorCount(colors));
    return colors;
}

QVector<int> GraphColoring::dsatur(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    QVector<int> colors(n, -1);
    QVector<int> saturation(n, 0);
    QVector<QSet<int>> neighborColors(n);

    /* 计算度数 */
    QVector<int> degree(n);
    for (int i = 0; i < n; ++i)
        degree[i] = adjacency[i].size();

    for (int step = 0; step < n; ++step) {
        /* 选择饱和度最高的未着色顶点(平局选度最高的) */
        int bestV = -1;
        for (int v = 0; v < n; ++v) {
            if (colors[v] >= 0) continue;
            if (bestV < 0 ||
                saturation[v] > saturation[bestV] ||
                (saturation[v] == saturation[bestV] && degree[v] > degree[bestV])) {
                bestV = v;
            }
        }

        if (bestV < 0) break;

        /* 找最小可用颜色 */
        int c = 0;
        while (neighborColors[bestV].contains(c)) c++;
        colors[bestV] = c;

        /* 更新邻居 */
        for (int u : adjacency[bestV]) {
            if (u >= 0 && u < n && colors[u] < 0) {
                if (!neighborColors[u].contains(c)) {
                    neighborColors[u].insert(c);
                    saturation[u]++;
                }
            }
        }
    }

    m_stats.totalColored++;
    m_stats.totalColors += colorCount(colors);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColored;

    emit coloringCompleted(n, colorCount(colors));
    return colors;
}

QVector<int> GraphColoring::welshPowell(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    QVector<int> colors(n, -1);

    /* 按度降序排列 */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return adjacency[a].size() > adjacency[b].size();
    });

    int currentColor = 0;
    for (int v : order) {
        if (colors[v] >= 0) continue;
        colors[v] = currentColor;

        /* 尝试给其他顶点分配同色 */
        for (int u : order) {
            if (colors[u] >= 0) continue;

            bool conflict = false;
            for (int w : adjacency[u]) {
                if (w >= 0 && w < n && colors[w] == currentColor) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) colors[u] = currentColor;
        }
        currentColor++;
    }

    m_stats.totalColored++;
    m_stats.totalColors += colorCount(colors);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColored;

    emit coloringCompleted(n, colorCount(colors));
    return colors;
}

bool GraphColoring::isValid(const QVector<QVector<int>>& adjacency,
                              const QVector<int>& colors)
{
    int n = adjacency.size();
    for (int v = 0; v < n; ++v) {
        for (int u : adjacency[v]) {
            if (u >= 0 && u < n && colors[v] == colors[u])
                return false;
        }
    }
    return true;
}

int GraphColoring::colorCount(const QVector<int>& colors)
{
    int maxC = -1;
    for (int c : colors) maxC = qMax(maxC, c);
    return maxC + 1;
}

GraphColoring::Stats GraphColoring::stats() const { return m_stats; }

void GraphColoring::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
