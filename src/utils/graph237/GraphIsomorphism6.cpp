/**
 * @file GraphIsomorphism6.cpp
 * @brief GraphIsomorphism6 实现
 *
 * 实现图同构：Weisfeiler-Leman k维精细化、k可区分性测试。
 */

#include "utils/graph237/GraphIsomorphism6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphIsomorphism6::GraphIsomorphism6(QObject *parent) : QObject(parent) {}
GraphIsomorphism6::~GraphIsomorphism6() = default;

/* ---- Configuration ---- */

void GraphIsomorphism6::setDimension(int k) { m_k = qMax(1, k); }

/* ---- WL refinement step ---- */

QVector<int> GraphIsomorphism6::wlRefine(const QVector<QVector<int>>& adj,
                                            const QVector<int>& colors) const
{
    int n = adj.size();
    QVector<int> newColors(n);

    QHash<QString, int> colorMap;
    int nextColor = 0;

    for (int v = 0; v < n; ++v) {
        // Collect neighbor colors and sort
        QVector<int> neighborColors;
        for (int u = 0; u < n; ++u) {
            if (adj[v][u] != 0)
                neighborColors.append(colors[u]);
        }
        std::sort(neighborColors.begin(), neighborColors.end());

        // Create composite label: own color + sorted neighbor colors
        QString key = QString::number(colors[v]);
        for (int c : neighborColors)
            key += "_" + QString::number(c);

        if (!colorMap.contains(key))
            colorMap[key] = nextColor++;
        newColors[v] = colorMap[key];
    }
    return newColors;
}

/* ---- Compress colors ---- */

int GraphIsomorphism6::compressColors(const QVector<int>& sortedColors,
                                         const QHash<QString, int>& colorMap,
                                         int& nextColor) const
{
    QString key;
    for (int c : sortedColors) key += QString::number(c) + "_";
    if (!colorMap.contains(key))
        const_cast<QHash<QString, int>&>(colorMap)[key] = nextColor++;
    return colorMap[key];
}

/* ---- Check stability ---- */

bool GraphIsomorphism6::isStable(const QVector<int>& prev,
                                    const QVector<int>& curr) const
{
    return prev == curr;
}

/* ---- WL coloring ---- */

QVector<int> GraphIsomorphism6::wlColoring(const QVector<QVector<int>>& adj,
                                              int maxIter) const
{
    int n = adj.size();
    if (n == 0) return {};

    // Initial coloring: degree-based
    QVector<int> colors(n);
    for (int v = 0; v < n; ++v) {
        int deg = 0;
        for (int u = 0; u < n; ++u) deg += (adj[v][u] != 0) ? 1 : 0;
        colors[v] = deg;
    }

    // Refine until stable
    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<int> newColors = wlRefine(adj, colors);
        if (isStable(colors, newColors)) break;
        colors = newColors;
    }
    return colors;
}

/* ---- k-WL coloring ---- */

QVector<int> GraphIsomorphism6::kWLColoring(const QVector<QVector<int>>& adj,
                                               int k, int maxIter) const
{
    if (k <= 1) return wlColoring(adj, maxIter);

    int n = adj.size();
    // k-WL: color k-tuples of vertices
    // Simplified: run 1-WL on k-powers of the graph
    QVector<int> colors = wlColoring(adj, maxIter);

    // Higher-order refinement using k-subgraph patterns
    for (int order = 2; order <= k; ++order) {
        QVector<int> newColors(n);
        QHash<QString, int> colorMap;
        int nextColor = 0;

        for (int v = 0; v < n; ++v) {
            // Collect colors of order-hop neighbors
            QVector<int> pattern;
            pattern.append(colors[v]);
            for (int u = 0; u < n; ++u) {
                if (adj[v][u] != 0) {
                    pattern.append(colors[u]);
                    // Second-order: neighbors of neighbors
                    if (order >= 2) {
                        for (int w = 0; w < n; ++w) {
                            if (adj[u][w] != 0 && w != v)
                                pattern.append(colors[w]);
                        }
                    }
                }
            }
            std::sort(pattern.begin(), pattern.end());

            QString key;
            for (int c : pattern) key += QString::number(c) + "_";
            if (!colorMap.contains(key))
                colorMap[key] = nextColor++;
            newColors[v] = colorMap[key];
        }
        colors = newColors;
    }
    return colors;
}

/* ---- Histogram compatibility ---- */

bool GraphIsomorphism6::histogramCompatible(const QVector<int>& c1,
                                              const QVector<int>& c2) const
{
    if (c1.size() != c2.size()) return false;
    QVector<int> h1 = c1, h2 = c2;
    std::sort(h1.begin(), h1.end());
    std::sort(h2.begin(), h2.end());
    return h1 == h2;
}

/* ---- Find mapping ---- */

QVector<int> GraphIsomorphism6::findMapping(const QVector<QVector<int>>& adj1,
                                               const QVector<QVector<int>>& adj2,
                                               const QVector<int>& c1,
                                               const QVector<int>& c2) const
{
    int n = c1.size();
    QVector<int> mapping(n, -1);
    QVector<bool> used(n, false);

    // For each vertex in graph1, find a matching vertex in graph2
    for (int v = 0; v < n; ++v) {
        for (int u = 0; u < n; ++u) {
            if (!used[u] && c1[v] == c2[u]) {
                // Verify local neighborhood structure
                bool valid = true;
                int deg1 = 0, deg2 = 0;
                for (int w = 0; w < n; ++w) {
                    deg1 += (adj1[v][w] != 0) ? 1 : 0;
                    deg2 += (adj2[u][w] != 0) ? 1 : 0;
                }
                if (deg1 == deg2) {
                    mapping[v] = u;
                    used[u] = true;
                    break;
                }
            }
        }
    }
    return mapping;
}

/* ---- Test isomorphism ---- */

GraphIsomorphism6::IsoResult GraphIsomorphism6::test(
    const QVector<QVector<int>>& adj1,
    const QVector<QVector<int>>& adj2)
{
    QElapsedTimer timer;
    timer.start();

    IsoResult result;
    int n1 = adj1.size(), n2 = adj2.size();
    m_stats.numVertices1 = n1;
    m_stats.numVertices2 = n2;

    // Quick rejection: different vertex/edge counts
    if (n1 != n2) {
        result.definitelyNot = true;
        result.isomorphic = false;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit testCompleted(false, 0, timer.elapsed());
        return result;
    }

    int edges1 = 0, edges2 = 0;
    for (int i = 0; i < n1; ++i)
        for (int j = 0; j < adj1[i].size(); ++j)
            edges1 += (adj1[i][j] != 0) ? 1 : 0;
    for (int i = 0; i < n2; ++i)
        for (int j = 0; j < adj2[i].size(); ++j)
            edges2 += (adj2[i][j] != 0) ? 1 : 0;
    m_stats.numEdges1 = edges1;
    m_stats.numEdges2 = edges2;

    if (edges1 != edges2) {
        result.definitelyNot = true;
        result.isomorphic = false;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit testCompleted(false, 0, timer.elapsed());
        return result;
    }

    // Compute k-WL coloring for both graphs
    int maxIter = 100;
    result.coloring1 = kWLColoring(adj1, m_k, maxIter);
    result.coloring2 = kWLColoring(adj2, m_k, maxIter);

    // Check histogram compatibility
    if (!histogramCompatible(result.coloring1, result.coloring2)) {
        result.definitelyNot = true;
        result.isomorphic = false;
    } else {
        // WL test passed — likely isomorphic
        result.isomorphic = true;
        result.mapping = findMapping(adj1, adj2,
                                      result.coloring1, result.coloring2);
    }

    result.kDimension = m_k;
    result.refinementIterations = maxIter;
    m_stats.kDimension = m_k;
    m_stats.totalIterations += maxIter;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit testCompleted(result.isomorphic, maxIter, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_k = 1;
}
