/**
 * @file GraphIsomorphism8.cpp
 * @brief GraphIsomorphism8 实现
 *
 * 实现图同构检测：顶点分类迭代细化与色彩传递至稳定划分。
 */

#include "utils/graph251/GraphIsomorphism8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphIsomorphism8::GraphIsomorphism8(QObject *parent) : QObject(parent) {}
GraphIsomorphism8::~GraphIsomorphism8() = default;

/* ---- Configuration ---- */

void GraphIsomorphism8::setGraph1(const QVector<QVector<int>>& adj) { m_adj1 = adj; }
void GraphIsomorphism8::setGraph2(const QVector<QVector<int>>& adj) { m_adj2 = adj; }
void GraphIsomorphism8::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }

/* ---- Initial coloring by degree ---- */

QVector<int> GraphIsomorphism8::initialColoring(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> colors(n);
    for (int i = 0; i < n; ++i) colors[i] = adj[i].size();
    return relabelColors(colors);
}

/* ---- Neighbor signature ---- */

QVector<int> GraphIsomorphism8::neighborSignature(const QVector<QVector<int>>& adj,
                                                   const QVector<int>& colors, int v) const
{
    QVector<int> sig;
    for (int u : adj[v]) sig.append(colors[u]);
    std::sort(sig.begin(), sig.end());
    return sig;
}

/* ---- Relabel colors to consecutive integers ---- */

QVector<int> GraphIsomorphism8::relabelColors(const QVector<int>& colors) const
{
    QVector<QPair<int, int>> indexed;
    for (int i = 0; i < colors.size(); ++i) indexed.append({colors[i], i});
    std::sort(indexed.begin(), indexed.end());

    QVector<int> result(colors.size());
    int label = 0;
    for (int i = 0; i < indexed.size(); ++i) {
        if (i > 0 && indexed[i].first != indexed[i - 1].first) label++;
        result[indexed[i].second] = label;
    }
    return result;
}

/* ---- Color refinement (one round) ---- */

QVector<int> GraphIsomorphism8::refineColors(const QVector<QVector<int>>& adj,
                                              const QVector<int>& colors) const
{
    int n = adj.size();
    // Build composite signature: (current_color, sorted_neighbor_colors)
    QVector<QPair<int, QVector<int>>> sigs(n);
    for (int v = 0; v < n; ++v) {
        sigs[v].first = colors[v];
        sigs[v].second = neighborSignature(adj, colors, v);
    }

    // Assign new colors based on unique signatures
    QVector<QPair<QPair<int, QVector<int>>, int>> indexed;
    for (int i = 0; i < n; ++i) indexed.append({sigs[i], i});
    std::sort(indexed.begin(), indexed.end());

    QVector<int> newColors(n);
    int label = 0;
    for (int i = 0; i < n; ++i) {
        if (i > 0 && !(indexed[i].first.first == indexed[i - 1].first.first &&
                       indexed[i].first.second == indexed[i - 1].first.second))
            label++;
        newColors[indexed[i].second] = label;
    }
    return newColors;
}

/* ---- Check stability ---- */

bool GraphIsomorphism8::isStable(const QVector<int>& prev, const QVector<int>& curr) const
{
    // Check if color class count is the same
    int prevClasses = countColorClasses(prev);
    int currClasses = countColorClasses(curr);
    return prevClasses == currClasses;
}

/* ---- Count color classes ---- */

int GraphIsomorphism8::countColorClasses(const QVector<int>& colors) const
{
    if (colors.isEmpty()) return 0;
    QVector<int> sorted = colors;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    return sorted.size();
}

/* ---- Verify mapping ---- */

bool GraphIsomorphism8::verifyMapping(const QVector<QVector<int>>& g1,
                                       const QVector<QVector<int>>& g2,
                                       const QVector<int>& map) const
{
    int n = g1.size();
    for (int u = 0; u < n; ++u) {
        for (int v : g1[u]) {
            // Check if (map[u], map[v]) is an edge in g2
            bool found = false;
            for (int w : g2[map[u]]) {
                if (w == map[v]) { found = true; break; }
            }
            if (!found) return false;
        }
    }
    return true;
}

/* ---- Backtracking search ---- */

bool GraphIsomorphism8::findMapping(const QVector<QVector<int>>& g1,
                                     const QVector<QVector<int>>& g2,
                                     const QVector<int>& c1, const QVector<int>& c2,
                                     QVector<int>& map, QVector<bool>& used, int depth) const
{
    int n = g1.size();
    if (depth == n) return verifyMapping(g1, g2, map);

    int v = depth;
    // Try all vertices in g2 with matching color
    for (int u = 0; u < n; ++u) {
        if (used[u]) continue;
        if (c1[v] != c2[u]) continue;
        if (g1[v].size() != g2[u].size()) continue;

        map[v] = u;
        used[u] = true;

        // Prune: check partial consistency
        bool consistent = true;
        for (int w : g1[v]) {
            if (w < depth) {
                bool found = false;
                for (int x : g2[u]) { if (x == map[w]) { found = true; break; } }
                if (!found) { consistent = false; break; }
            }
        }

        if (consistent && findMapping(g1, g2, c1, c2, map, used, depth + 1))
            return true;

        used[u] = false;
        map[v] = -1;
    }
    return false;
}

/* ---- Canonical coloring ---- */

QVector<int> GraphIsomorphism8::canonicalColoring(const QVector<QVector<int>>& adj) const
{
    QVector<int> colors = initialColoring(adj);
    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<int> newColors = refineColors(adj, colors);
        if (isStable(colors, newColors)) break;
        colors = newColors;
    }
    return relabelColors(colors);
}

/* ---- Check isomorphism ---- */

GraphIsomorphism8::IsoResult GraphIsomorphism8::check() const
{
    QElapsedTimer timer;
    timer.start();

    IsoResult result;
    int n1 = m_adj1.size(), n2 = m_adj2.size();

    if (n1 != n2) {
        m_stats.totalOps++;
        emit checkCompleted(false, 0, timer.elapsed());
        return result;
    }

    int n = n1;
    if (n == 0) {
        result.isomorphic = true;
        m_stats.totalOps++;
        return result;
    }

    // Quick degree check
    QVector<int> deg1(n), deg2(n);
    for (int i = 0; i < n; ++i) { deg1[i] = m_adj1[i].size(); deg2[i] = m_adj2[i].size(); }
    std::sort(deg1.begin(), deg1.end());
    std::sort(deg2.begin(), deg2.end());
    if (deg1 != deg2) {
        m_stats.totalOps++;
        emit checkCompleted(false, 0, timer.elapsed());
        return result;
    }

    // Color refinement on both graphs
    QVector<int> c1 = initialColoring(m_adj1);
    QVector<int> c2 = initialColoring(m_adj2);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<int> newC1 = refineColors(m_adj1, c1);
        QVector<int> newC2 = refineColors(m_adj2, c2);
        bool stable1 = isStable(c1, newC1);
        bool stable2 = isStable(c2, newC2);
        c1 = newC1; c2 = newC2;
        emit refinementIteration(iter + 1, countColorClasses(c1));
        if (stable1 && stable2) break;
    }

    // Compare color class distributions
    QVector<int> dist1 = c1, dist2 = c2;
    std::sort(dist1.begin(), dist1.end());
    std::sort(dist2.begin(), dist2.end());
    if (dist1 != dist2) {
        m_stats.totalOps++;
        emit checkCompleted(false, 0, timer.elapsed());
        return result;
    }

    // Backtracking search for exact mapping
    QVector<int> map(n, -1);
    QVector<bool> used(n, false);
    if (findMapping(m_adj1, m_adj2, c1, c2, map, used, 0)) {
        result.isomorphic = true;
        result.mapping = map;
    }

    result.iterations = 0;
    result.numColorClasses = countColorClasses(c1);

    m_stats.numVertices1 = n1;
    m_stats.numVertices2 = n2;
    m_stats.numEdges1 = 0;
    m_stats.numEdges2 = 0;
    for (int i = 0; i < n; ++i) { m_stats.numEdges1 += m_adj1[i].size(); m_stats.numEdges2 += m_adj2[i].size(); }
    m_stats.totalIterations = result.iterations;
    m_stats.totalOps++;

    emit checkCompleted(result.isomorphic, result.iterations, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism8::resetStatistics()
{
    m_adj1.clear(); m_adj2.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
