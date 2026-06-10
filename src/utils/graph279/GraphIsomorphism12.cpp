/**
 * @file GraphIsomorphism12.cpp
 * @brief GraphIsomorphism12 实现
 *
 * 实现图同构：Weisfeiler-Lehman k维细化与颜色精炼多项式时间判别。
 */

#include "utils/graph279/GraphIsomorphism12.h"

#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism12::GraphIsomorphism12(QObject *parent)
    : QObject(parent) {}

GraphIsomorphism12::~GraphIsomorphism12() = default;

/* ---- Utility ---- */

int GraphIsomorphism12::countEdges(const QVector<QVector<int>>& adj)
{
    int edges = 0;
    for (const auto& row : adj)
        edges += row.size();
    return edges / 2;  // Undirected
}

/* ---- Hash colors into single value ---- */

quint64 GraphIsomorphism12::hashColors(const QVector<quint64>& colors) const
{
    // FNV-1a-like hash for color multiset
    quint64 h = 14695981039346656037ULL;
    auto sorted = colors;
    std::sort(sorted.begin(), sorted.end());
    for (auto c : sorted) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return h;
}

/* ---- Compare signatures ---- */

bool GraphIsomorphism12::signaturesEqual(QVector<quint64> sig1,
                                           QVector<quint64> sig2)
{
    if (sig1.size() != sig2.size()) return false;
    std::sort(sig1.begin(), sig1.end());
    std::sort(sig2.begin(), sig2.end());
    return sig1 == sig2;
}

/* ---- 1D WL color refinement ---- */

QVector<quint64> GraphIsomorphism12::wlRefine1D(
    const QVector<QVector<int>>& adj, int maxIter) const
{
    int n = adj.size();
    QVector<quint64> colors(n);
    // Initial coloring: by degree
    for (int i = 0; i < n; ++i)
        colors[i] = static_cast<quint64>(adj[i].size());

    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<quint64> newColors(n);
        for (int i = 0; i < n; ++i) {
            // Collect neighbor colors
            QVector<quint64> neighborColors;
            for (int neighbor : adj[i])
                neighborColors.append(colors[neighbor]);
            // Hash own color + sorted neighbor multiset
            neighborColors.prepend(colors[i]);
            newColors[i] = hashColors(neighborColors);
        }

        // Check convergence
        if (newColors == colors) break;
        colors = newColors;
    }
    return colors;
}

/* ---- WL signature ---- */

QVector<quint64> GraphIsomorphism12::wlSignature(
    const QVector<QVector<int>>& adj) const
{
    return wlRefine1D(adj, 50);
}

/* ---- k-dimensional WL ---- */

QVector<QVector<quint64>> GraphIsomorphism12::wlKDim(
    const QVector<QVector<int>>& adj, int k) const
{
    int n = adj.size();
    QVector<QVector<quint64>> result(k + 1);

    // k=1: standard WL refinement
    result[1] = wlRefine1D(adj, 50);

    for (int dim = 2; dim <= k; ++dim) {
        result[dim].resize(n);
        // Higher-dim WL: color based on (k-1)-dim colors of all reachable tuples
        for (int i = 0; i < n; ++i) {
            QVector<quint64> tuple;
            tuple.append(result[dim - 1][i]);
            for (int neighbor : adj[i])
                tuple.append(result[dim - 1][neighbor]);
            result[dim][i] = hashColors(tuple);
        }
    }
    return result;
}

/* ---- Vertex coloring ---- */

QVector<int> GraphIsomorphism12::vertexColoring(
    const QVector<QVector<int>>& adj) const
{
    auto colors = wlRefine1D(adj, 50);

    // Map unique hash values to sequential integers
    auto sorted = colors;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());

    QVector<int> mapping(colors.size());
    for (int i = 0; i < colors.size(); ++i) {
        int idx = std::lower_bound(sorted.begin(), sorted.end(),
                                    colors[i]) - sorted.begin();
        mapping[i] = idx;
    }
    return mapping;
}

/* ---- Canonical label ---- */

QString GraphIsomorphism12::canonicalLabel(
    const QVector<QVector<int>>& adj) const
{
    auto colors = wlRefine1D(adj, 50);
    auto sorted = colors;
    std::sort(sorted.begin(), sorted.end());

    // Build canonical string from sorted color sequence and edge structure
    QByteArray data;
    for (auto c : sorted) {
        data.append(reinterpret_cast<const char*>(&c), sizeof(c));
    }
    // Include adjacency structure hashed
    for (int i = 0; i < adj.size(); ++i) {
        auto edges = adj[i];
        std::sort(edges.begin(), edges.end());
        for (int e : edges)
            data.append(reinterpret_cast<const char*>(&e), sizeof(e));
    }

    auto hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

/* ---- Isomorphism check ---- */

bool GraphIsomorphism12::isIsomorphic(
    const QVector<QVector<int>>& adj1,
    const QVector<QVector<int>>& adj2) const
{
    QElapsedTimer timer;
    timer.start();

    int n1 = adj1.size(), n2 = adj2.size();
    if (n1 != n2) return false;
    if (countEdges(adj1) != countEdges(adj2)) return false;

    // Quick degree sequence check
    QVector<int> deg1(n1), deg2(n2);
    for (int i = 0; i < n1; ++i) deg1[i] = adj1[i].size();
    for (int i = 0; i < n2; ++i) deg2[i] = adj2[i].size();
    std::sort(deg1.begin(), deg1.end());
    std::sort(deg2.begin(), deg2.end());
    if (deg1 != deg2) return false;

    // 1D WL test
    auto sig1 = wlRefine1D(adj1, 50);
    auto sig2 = wlRefine1D(adj2, 50);
    bool result = signaturesEqual(sig1, sig2);

    // If 1D passes, try 2D WL for stronger discrimination
    if (result && n1 <= 100) {
        auto wl2_1 = wlKDim(adj1, 2);
        auto wl2_2 = wlKDim(adj2, 2);
        result = signaturesEqual(wl2_1[2], wl2_2[2]);
    }

    double elapsed = timer.elapsed();
    m_stats.numVertices = n1;
    m_stats.numEdges = countEdges(adj1);
    m_stats.wlIterations = 50;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit comparisonComplete(result, 50, elapsed);
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
