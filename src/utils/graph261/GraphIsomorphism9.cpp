/**
 * @file GraphIsomorphism9.cpp
 * @brief GraphIsomorphism9 实现
 *
 * 实现图同构：Weisfeiler-Lehman k维细化与哈希规范标号。
 */

#include "utils/graph261/GraphIsomorphism9.h"

#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism9::GraphIsomorphism9(QObject *parent) : QObject(parent) {}
GraphIsomorphism9::~GraphIsomorphism9() = default;

/* ---- Configuration ---- */

void GraphIsomorphism9::setMaxIterations(int iters)
{
    m_maxIter = qMax(1, iters);
}

/* ---- Count edges ---- */

int GraphIsomorphism9::countEdges(const AdjList& graph) const
{
    int edges = 0;
    for (const auto& neighbors : graph)
        edges += neighbors.size();
    return edges / 2;  // Undirected: each edge counted twice
}

/* ---- Degree sequence ---- */

QVector<int> GraphIsomorphism9::degreeSequence(const AdjList& graph) const
{
    QVector<int> degs;
    degs.reserve(graph.size());
    for (const auto& neighbors : graph)
        degs.append(neighbors.size());
    std::sort(degs.begin(), degs.end(), std::greater<int>());
    return degs;
}

/* ---- FNV-1a hash ---- */

quint64 GraphIsomorphism9::fnv1a(quint64 hash, const void* data, int len)
{
    const auto* bytes = static_cast<const unsigned char*>(data);
    constexpr quint64 prime = 0x00000100000001B3ULL;
    for (int i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= prime;
    }
    return hash;
}

/* ---- Hash a sorted color vector into a single color ---- */

quint64 GraphIsomorphism9::hashColors(
    const QVector<quint64>& sortedNeighborColors) const
{
    quint64 h = 0xcbf29ce484222325ULL;  // FNV offset basis
    for (quint64 c : sortedNeighborColors)
        h = fnv1a(h, &c, sizeof(c));
    return h;
}

/* ---- WL refinement step ---- */

QVector<quint64> GraphIsomorphism9::wlRefine(
    const AdjList& graph, const QVector<quint64>& colors) const
{
    int n = graph.size();
    QVector<quint64> newColors(n);
    for (int v = 0; v < n; ++v) {
        // Collect neighbor colors into sorted multiset
        QVector<quint64> neighborColors;
        neighborColors.reserve(graph[v].size() + 1);
        neighborColors.append(colors[v]);  // Include own color
        for (int u : graph[v])
            neighborColors.append(colors[u]);
        std::sort(neighborColors.begin(), neighborColors.end());
        newColors[v] = hashColors(neighborColors);
    }
    return newColors;
}

/* ---- Compute canonical label ---- */

QString GraphIsomorphism9::canonicalLabel(const AdjList& graph)
{
    int n = graph.size();
    if (n == 0) return "empty";

    // Initialize colors from degree
    QVector<quint64> colors(n);
    for (int v = 0; v < n; ++v)
        colors[v] = static_cast<quint64>(graph[v].size()) + 1;

    // Iterative WL refinement until stable
    for (int iter = 0; iter < m_maxIter; ++iter) {
        auto newColors = wlRefine(graph, colors);
        // Check convergence
        bool changed = false;
        for (int v = 0; v < n; ++v) {
            if (newColors[v] != colors[v]) { changed = true; break; }
        }
        colors = newColors;
        if (!changed) break;
    }

    // Build canonical string from sorted color histogram
    QVector<quint64> sorted = colors;
    std::sort(sorted.begin(), sorted.end());
    QString label;
    for (int i = 0; i < sorted.size(); ++i) {
        if (i > 0) label += '_';
        label += QString::number(sorted[i], 16);
    }
    return label;
}

/* ---- Check isomorphism ---- */

bool GraphIsomorphism9::isIsomorphic(const AdjList& g1, const AdjList& g2)
{
    QElapsedTimer timer;
    timer.start();

    // Quick checks: vertex and edge counts must match
    if (g1.size() != g2.size()) {
        m_stats.lastResult = false;
        m_stats.totalOps++;
        emit isomorphismChecked(false, 0, timer.elapsed());
        return false;
    }

    int e1 = countEdges(g1), e2 = countEdges(g2);
    if (e1 != e2) {
        m_stats.lastResult = false;
        m_stats.totalOps++;
        emit isomorphismChecked(false, 0, timer.elapsed());
        return false;
    }

    // Degree sequence must match
    auto d1 = degreeSequence(g1);
    auto d2 = degreeSequence(g2);
    if (d1 != d2) {
        m_stats.lastResult = false;
        m_stats.totalOps++;
        emit isomorphismChecked(false, 0, timer.elapsed());
        return false;
    }

    // WL refinement for both graphs
    int n = g1.size();
    QVector<quint64> c1(n), c2(n);
    for (int v = 0; v < n; ++v) {
        c1[v] = static_cast<quint64>(g1[v].size()) + 1;
        c2[v] = static_cast<quint64>(g2[v].size()) + 1;
    }

    int iter = 0;
    for (; iter < m_maxIter; ++iter) {
        auto nc1 = wlRefine(g1, c1);
        auto nc2 = wlRefine(g2, c2);

        // Check if color histograms differ -> not isomorphic
        auto sh1 = nc1, sh2 = nc2;
        std::sort(sh1.begin(), sh1.end());
        std::sort(sh2.begin(), sh2.end());
        if (sh1 != sh2) {
            m_stats.numVertices = n;
            m_stats.numEdges = e1;
            m_stats.wlIterations = iter + 1;
            m_stats.lastResult = false;
            m_stats.totalOps++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
            emit isomorphismChecked(false, iter + 1, timer.elapsed());
            return false;
        }

        c1 = nc1;
        c2 = nc2;
    }

    m_lastColors = c1;
    m_stats.numVertices = n;
    m_stats.numEdges = e1;
    m_stats.wlIterations = iter;
    m_stats.lastResult = true;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit isomorphismChecked(true, iter, timer.elapsed());
    return true;
}

/* ---- Accessors ---- */

QVector<quint64> GraphIsomorphism9::lastColoring() const
{
    return m_lastColors;
}

/* ---- Reset ---- */

void GraphIsomorphism9::resetStatistics()
{
    m_lastColors.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
