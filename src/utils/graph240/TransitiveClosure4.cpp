/**
 * @file TransitiveClosure4.cpp
 * @brief TransitiveClosure4 实现
 *
 * 实现传递闭包：BFS位集传播、压缩稀疏行可达性索引。
 */

#include "utils/graph240/TransitiveClosure4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/* ---- Construction / Destruction ---- */

TransitiveClosure4::TransitiveClosure4(QObject *parent) : QObject(parent) {}
TransitiveClosure4::~TransitiveClosure4() = default;

/* ---- Build graph ---- */

void TransitiveClosure4::buildGraph(int numVertices,
                                     const QVector<QPair<int, int>>& edges)
{
    m_numVertices = numVertices;
    m_adj.resize(numVertices);
    for (int i = 0; i < numVertices; ++i) m_adj[i].clear();

    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < numVertices &&
            edge.second >= 0 && edge.second < numVertices) {
            m_adj[edge.first].append(edge.second);
        }
    }
    m_stats.numVertices = numVertices;
    m_stats.numEdges = edges.size();
}

/* ---- BFS bitset propagation ---- */

QBitArray TransitiveClosure4::bfsBitset(int source, QVector<int>& hops) const
{
    QBitArray reached(m_numVertices, false);
    hops.resize(m_numVertices);
    hops.fill(-1);

    std::queue<int> q;
    q.push(source);
    reached.setBit(source);
    hops[source] = 0;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : m_adj[u]) {
            if (!reached.testBit(v)) {
                reached.setBit(v);
                hops[v] = hops[u] + 1;
                q.push(v);
            }
        }
    }
    return reached;
}

/* ---- Compute closure ---- */

void TransitiveClosure4::computeClosure()
{
    QElapsedTimer timer;
    timer.start();

    m_closure.resize(m_numVertices);
    m_hopDist.resize(m_numVertices);
    int reachablePairs = 0;

    for (int u = 0; u < m_numVertices; ++u) {
        m_closure[u] = bfsBitset(u, m_hopDist[u]);
        for (int v = 0; v < m_numVertices; ++v)
            if (m_closure[u].testBit(v)) reachablePairs++;
        m_stats.numBfsRuns++;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit closureComputed(m_numVertices, reachablePairs, elapsed);
}

/* ---- Query reachability ---- */

TransitiveClosure4::ReachResult TransitiveClosure4::query(int u, int v) const
{
    ReachResult result;
    if (u < 0 || u >= m_numVertices || v < 0 || v >= m_numVertices)
        return result;
    if (m_closure.isEmpty()) return result;

    result.reachable = m_closure[u].testBit(v);
    result.hopCount = result.reachable ? m_hopDist[u][v] : -1;
    return result;
}

/* ---- Reachable set ---- */

QVector<int> TransitiveClosure4::reachableSet(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_numVertices || m_closure.isEmpty()) return result;
    for (int v = 0; v < m_numVertices; ++v)
        if (m_closure[u].testBit(v)) result.append(v);
    return result;
}

/* ---- Build CSR reachability index ---- */

void TransitiveClosure4::buildCSRIndex()
{
    QElapsedTimer timer;
    timer.start();

    if (m_closure.isEmpty()) return;

    // Count total reachable pairs for CSR allocation
    int totalTargets = 0;
    for (int u = 0; u < m_numVertices; ++u) {
        for (int v = 0; v < m_numVertices; ++v)
            if (m_closure[u].testBit(v)) totalTargets++;
    }

    m_csrRowStart.resize(m_numVertices + 1, 0);
    m_csrTargets.resize(totalTargets);

    // Build row pointers
    m_csrRowStart[0] = 0;
    for (int u = 0; u < m_numVertices; ++u) {
        int count = 0;
        for (int v = 0; v < m_numVertices; ++v)
            if (m_closure[u].testBit(v)) count++;
        m_csrRowStart[u + 1] = m_csrRowStart[u] + count;
    }

    // Fill targets (sorted per row for binary search)
    int pos = 0;
    for (int u = 0; u < m_numVertices; ++u) {
        for (int v = 0; v < m_numVertices; ++v) {
            if (m_closure[u].testBit(v))
                m_csrTargets[pos++] = v;
        }
        // Targets are already sorted by v
    }

    m_stats.indexBuildTimeMs = timer.elapsed();
}

/* ---- CSR reachability check ---- */

bool TransitiveClosure4::csrReachable(int u, int v) const
{
    if (u < 0 || u >= m_numVertices || v < 0 || v >= m_numVertices)
        return false;
    if (m_csrRowStart.isEmpty()) return false;

    // Binary search in sorted targets for row u
    int lo = m_csrRowStart[u];
    int hi = m_csrRowStart[u + 1] - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (m_csrTargets[mid] == v) return true;
        if (m_csrTargets[mid] < v) lo = mid + 1;
        else hi = mid - 1;
    }
    return false;
}

/* ---- Reset ---- */

void TransitiveClosure4::resetStatistics()
{
    m_adj.clear();
    m_closure.clear();
    m_hopDist.clear();
    m_csrRowStart.clear();
    m_csrTargets.clear();
    m_numVertices = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
