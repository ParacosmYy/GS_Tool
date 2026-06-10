/**
 * @file GraphIsomorphism11.cpp
 * @brief GraphIsomorphism11 实现
 *
 * 实现图同构：度序列指纹与排序邻接编码紧致规范形式。
 */

#include "utils/graph275/GraphIsomorphism11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism11::GraphIsomorphism11(QObject *parent)
    : QObject(parent) {}
GraphIsomorphism11::~GraphIsomorphism11() = default;

/* ---- Degree helpers ---- */

QVector<int> GraphIsomorphism11::computeDegrees(const Graph& g) const
{
    QVector<int> deg(g.numVertices, 0);
    for (int v = 0; v < g.numVertices; ++v)
        deg[v] = g.adjacency[v].size();
    return deg;
}

QVector<int> GraphIsomorphism11::degreeFingerprint(const Graph& g) const
{
    QVector<int> deg = computeDegrees(g);
    std::sort(deg.begin(), deg.end());
    return deg;
}

int GraphIsomorphism11::countEdges(const Graph& g) const
{
    int edges = 0;
    for (int v = 0; v < g.numVertices; ++v)
        edges += g.adjacency[v].size();
    return g.directed ? edges : edges / 2;
}

/* ---- Hashing ---- */

quint64 GraphIsomorphism11::hashRow(const QVector<int>& neighbors, int vertex, int n) const
{
    quint64 h = static_cast<quint64>(vertex) * 1000003ULL;
    for (int nb : neighbors)
        h = h * 31ULL + static_cast<quint64>(nb);
    return h;
}

/* ---- Sorted adjacency encoding ---- */

QVector<quint64> GraphIsomorphism11::sortedAdjacencyEncoding(
    const Graph& g, const QVector<int>& vertexOrder) const
{
    QVector<quint64> encoding;
    encoding.reserve(vertexOrder.size());
    for (int v : vertexOrder) {
        QVector<int> neighbors = g.adjacency[v];
        std::sort(neighbors.begin(), neighbors.end());
        encoding.append(hashRow(neighbors, v, g.numVertices));
    }
    std::sort(encoding.begin(), encoding.end());
    return encoding;
}

/* ---- Canonical form ---- */

QVector<quint64> GraphIsomorphism11::canonicalForm(const Graph& g) const
{
    // Use degree-sorted vertex order for canonical encoding
    QVector<int> order(g.numVertices);
    for (int i = 0; i < g.numVertices; ++i) order[i] = i;
    QVector<int> deg = computeDegrees(g);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return deg[a] < deg[b];
    });
    return sortedAdjacencyEncoding(g, order);
}

/* ---- Partition by degree ---- */

QVector<QVector<int>> GraphIsomorphism11::partitionByDegree(const Graph& g) const
{
    QVector<int> deg = computeDegrees(g);
    QMap<int, QVector<int>> partitions;
    for (int v = 0; v < g.numVertices; ++v)
        partitions[deg[v]].append(v);

    QVector<QVector<int>> result;
    for (auto it = partitions.begin(); it != partitions.end(); ++it)
        result.append(it.value());
    return result;
}

/* ---- Backtracking isomorphism search ---- */

bool GraphIsomorphism11::backtrackIso(const Graph& g1, const Graph& g2,
    QVector<int>& mapping, QVector<bool>& used,
    const QVector<QVector<int>>& part1,
    const QVector<QVector<int>>& part2) const
{
    int depth = 0;
    for (const auto& p : part1)
        for (int v : p)
            if (mapping[v] < 0) depth++;
            // Actually find unmapped vertices

    // Find next unmapped vertex in g1
    int nextV = -1;
    int partIdx = -1;
    for (int p = 0; p < part1.size() && nextV < 0; ++p) {
        for (int v : part1[p]) {
            if (mapping[v] < 0) {
                nextV = v;
                partIdx = p;
                break;
            }
        }
    }
    if (nextV < 0) return true;  // All mapped

    // Try each vertex in corresponding partition of g2
    const QVector<int>& candidates = part2.value(partIdx, QVector<int>());
    for (int u : candidates) {
        if (used[u]) continue;

        // Check adjacency consistency
        bool consistent = true;
        for (int nb : g1.adjacency[nextV]) {
            if (mapping[nb] >= 0) {
                // Check if mapping[nb] is adjacent to u in g2
                bool found = false;
                for (int nb2 : g2.adjacency[u]) {
                    if (nb2 == mapping[nb]) { found = true; break; }
                }
                if (!found) { consistent = false; break; }
            }
        }
        if (!consistent) continue;

        mapping[nextV] = u;
        used[u] = true;
        if (backtrackIso(g1, g2, mapping, used, part1, part2))
            return true;
        mapping[nextV] = -1;
        used[u] = false;
    }
    return false;
}

/* ---- Isomorphic check ---- */

bool GraphIsomorphism11::isIsomorphic(const Graph& g1, const Graph& g2) const
{
    QElapsedTimer timer;
    timer.start();

    // Quick rejects
    if (g1.numVertices != g2.numVertices) return false;
    if (g1.directed != g2.directed) return false;
    if (countEdges(g1) != countEdges(g2)) return false;

    // Degree fingerprint comparison
    QVector<int> fp1 = degreeFingerprint(g1);
    QVector<int> fp2 = degreeFingerprint(g2);
    if (fp1 != fp2) return false;

    // Canonical form comparison
    QVector<quint64> cf1 = canonicalForm(g1);
    QVector<quint64> cf2 = canonicalForm(g2);
    if (cf1 == cf2) {
        double elapsed = timer.elapsed();
        m_stats.numGraphsCompared++;
        m_stats.numIsomorphicPairs++;
        m_stats.numVertices = g1.numVertices;
        m_stats.totalOps++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit comparisonCompleted(g1.numVertices, true, elapsed);
        return true;
    }

    // Full backtracking search
    int n = g1.numVertices;
    QVector<int> mapping(n, -1);
    QVector<bool> used(n, false);
    auto part1 = partitionByDegree(g1);
    auto part2 = partitionByDegree(g2);

    bool result = backtrackIso(g1, g2, mapping, used, part1, part2);

    double elapsed = timer.elapsed();
    m_stats.numGraphsCompared++;
    m_stats.numVertices = g1.numVertices;
    if (result) m_stats.numIsomorphicPairs++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit comparisonCompleted(g1.numVertices, result, elapsed);
    return result;
}

/* ---- Find mapping ---- */

QVector<int> GraphIsomorphism11::findMapping(const Graph& g1, const Graph& g2) const
{
    if (g1.numVertices != g2.numVertices) return {};

    int n = g1.numVertices;
    QVector<int> mapping(n, -1);
    QVector<bool> used(n, false);
    auto part1 = partitionByDegree(g1);
    auto part2 = partitionByDegree(g2);

    if (backtrackIso(g1, g2, mapping, used, part1, part2))
        return mapping;
    return {};
}

/* ---- Reset ---- */

void GraphIsomorphism11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
