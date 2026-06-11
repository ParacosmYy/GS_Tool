/**
 * @file GraphIsomorphism16.cpp
 * @brief GraphIsomorphism16 实现
 *
 * 实现图同构：Weisfeiler-Leman k维细化与顶点分类实现有界度图认证。
 */

#include "utils/graph307/GraphIsomorphism16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism16::GraphIsomorphism16(QObject *parent)
    : QObject(parent) {}

GraphIsomorphism16::~GraphIsomorphism16() = default;

/* ---- Configuration ---- */

void GraphIsomorphism16::setMaxWLIterations(int iter) { m_maxWLIter = qBound(10, iter, 1000); }
void GraphIsomorphism16::setWLDimension(int k) { m_wlDim = qBound(1, k, 2); }

/* ---- 1-WL refinement ---- */

QVector<int> GraphIsomorphism16::wl1Refine(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> colors(n, 1); // Initial: uniform color (degree-based)
    // Initialize with degree
    for (int i = 0; i < n; ++i)
        colors[i] = adj[i].size();

    QHash<QString, int> colorMap;
    int nextColor = 0;
    // Register initial colors
    for (int i = 0; i < n; ++i) {
        QString key = QString::number(colors[i]);
        if (!colorMap.contains(key))
            colorMap[key] = nextColor++;
        colors[i] = colorMap[key];
    }

    for (int iter = 0; iter < m_maxWLIter; ++iter) {
        QVector<int> newColors(n);
        for (int v = 0; v < n; ++v) {
            // Collect neighbor colors
            QVector<int> neighborColors;
            neighborColors.reserve(adj[v].size());
            for (int u : adj[v])
                neighborColors.append(colors[u]);
            std::sort(neighborColors.begin(), neighborColors.end());
            newColors[v] = compressColors(neighborColors, colorMap, nextColor);
            // Combine with own color
            QString key = QString("%1|%2").arg(colors[v]).arg(newColors[v]);
            if (!colorMap.contains(key))
                colorMap[key] = nextColor++;
            newColors[v] = colorMap[key];
        }
        if (newColors == colors) break;
        colors = newColors;
    }
    return colors;
}

/* ---- 2-WL refinement ---- */

QVector<QVector<int>> GraphIsomorphism16::wl2Refine(
    const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    // Color(i,j) = 1 if edge, 0 if i==j, 2 otherwise
    QVector<QVector<int>> colors(n, QVector<int>(n, 2));
    for (int i = 0; i < n; ++i) {
        colors[i][i] = 0;
        for (int j : adj[i])
            colors[i][j] = 1;
    }

    QHash<QString, int> colorMap;
    int nextColor = 3;

    for (int iter = 0; iter < m_maxWLIter; ++iter) {
        QVector<QVector<int>> newColors(n, QVector<int>(n));
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                // Collect multiset of (colors[i][k], colors[k][j]) for all k
                QVector<QPair<int, int>> multiset;
                multiset.reserve(n);
                for (int k = 0; k < n; ++k)
                    multiset.append({colors[i][k], colors[k][j]});
                std::sort(multiset.begin(), multiset.end());
                QString key = QString("%1").arg(colors[i][j]);
                for (const auto& p : multiset)
                    key += QString("|%1,%2").arg(p.first).arg(p.second);
                if (!colorMap.contains(key))
                    colorMap[key] = nextColor++;
                newColors[i][j] = colorMap[key];
                if (newColors[i][j] != colors[i][j]) changed = true;
            }
        }
        if (!changed) break;
        colors = newColors;
    }
    return colors;
}

/* ---- Color compression ---- */

int GraphIsomorphism16::compressColors(const QVector<int>& colors,
                                          QHash<QString, int>& colorMap,
                                          int& nextColor) const
{
    QString key;
    for (int c : colors)
        key += QString("%1,").arg(c);
    if (!colorMap.contains(key))
        colorMap[key] = nextColor++;
    return colorMap[key];
}

/* ---- Find mapping via backtracking ---- */

QVector<int> GraphIsomorphism16::findMapping(
    const QVector<QVector<int>>& adj1,
    const QVector<QVector<int>>& adj2,
    const QVector<int>& color1,
    const QVector<int>& color2) const
{
    int n = adj1.size();
    if (adj2.size() != n) return {};

    // Group vertices by color
    QHash<int, QVector<int>> groups2;
    for (int i = 0; i < n; ++i)
        groups2[color2[i]].append(i);

    QVector<int> mapping(n, -1);
    QVector<bool> used(n, false);

    // Simple backtracking with color pruning
    std::function<bool(int)> solve = [&](int v) -> bool {
        if (v == n) return true;
        int targetColor = color1[v];
        if (!groups2.contains(targetColor)) return false;
        for (int candidate : groups2[targetColor]) {
            if (used[candidate]) continue;
            // Check local consistency: neighbors of v must map to neighbors of candidate
            bool consistent = true;
            for (int u : adj1[v]) {
                if (u < v && mapping[u] >= 0) {
                    bool found = false;
                    for (int w : adj2[candidate]) {
                        if (w == mapping[u]) { found = true; break; }
                    }
                    if (!found) { consistent = false; break; }
                }
            }
            if (consistent) {
                mapping[v] = candidate;
                used[candidate] = true;
                if (solve(v + 1)) return true;
                mapping[v] = -1;
                used[candidate] = false;
            }
        }
        return false;
    };

    solve(0);
    return mapping;
}

/* ---- Verify mapping ---- */

bool GraphIsomorphism16::verifyMapping(const QVector<QVector<int>>& adj1,
                                          const QVector<QVector<int>>& adj2,
                                          const QVector<int>& mapping) const
{
    int n = adj1.size();
    for (int i = 0; i < n; ++i) {
        if (mapping[i] < 0) return false;
        for (int j : adj1[i]) {
            int mi = mapping[i], mj = mapping[j];
            bool found = false;
            for (int w : adj2[mi]) {
                if (w == mj) { found = true; break; }
            }
            if (!found) return false;
        }
    }
    return true;
}

/* ---- Compute WL coloring ---- */

QVector<int> GraphIsomorphism16::computeWLColoring(
    const QVector<QVector<int>>& adj) const
{
    if (m_wlDim == 1) {
        return wl1Refine(adj);
    }
    // For 2-WL, project diagonal as vertex colors
    auto mat = wl2Refine(adj);
    int n = adj.size();
    QVector<int> colors(n);
    for (int i = 0; i < n; ++i)
        colors[i] = mat[i][i];
    return colors;
}

/* ---- Canonical hash ---- */

quint64 GraphIsomorphism16::canonicalHash(const QVector<QVector<int>>& adj) const
{
    auto colors = computeWLColoring(adj);
    QVector<int> sorted = colors;
    std::sort(sorted.begin(), sorted.end());
    quint64 h = 0;
    for (int c : sorted)
        h = h * 31 + static_cast<quint64>(c);
    return h;
}

/* ---- Test isomorphism ---- */

GraphIsomorphism16::IsoResult GraphIsomorphism16::testIsomorphism(
    const QVector<QVector<int>>& adj1,
    const QVector<QVector<int>>& adj2) const
{
    QElapsedTimer timer;
    timer.start();

    IsoResult result;
    int n1 = adj1.size();
    int n2 = adj2.size();
    result.numVertices = n1;

    // Quick check: different vertex/edge counts
    if (n1 != n2) {
        result.isIsomorphic = false;
        result.isCertified = true;
        double elapsed = timer.elapsed();
        m_stats.nonIsoCount++;
        m_stats.totalTests++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;
        emit testDone(n1, n2, false, elapsed);
        return result;
    }

    // WL coloring comparison
    auto color1 = wl1Refine(adj1);
    auto color2 = wl1Refine(adj2);

    // Sort color histograms
    QVector<int> hist1 = color1, hist2 = color2;
    std::sort(hist1.begin(), hist1.end());
    std::sort(hist2.begin(), hist2.end());

    if (hist1 != hist2) {
        // WL certifies non-isomorphism
        result.isIsomorphic = false;
        result.isCertified = true;
        double elapsed = timer.elapsed();
        m_stats.nonIsoCount++;
        m_stats.totalTests++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;
        emit testDone(n1, n2, false, elapsed);
        return result;
    }

    // Colors match; try to find explicit mapping
    auto mapping = findMapping(adj1, adj2, color1, color2);
    if (!mapping.isEmpty() && verifyMapping(adj1, adj2, mapping)) {
        result.isIsomorphic = true;
        result.mapping = mapping;
    } else {
        result.isIsomorphic = false;
    }
    result.wlIterations = m_maxWLIter;

    double elapsed = timer.elapsed();
    if (result.isIsomorphic) m_stats.isoCount++; else m_stats.nonIsoCount++;
    m_stats.totalTests++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testDone(n1, n2, result.isIsomorphic, elapsed);
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
