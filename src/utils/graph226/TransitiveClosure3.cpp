/**
 * @file TransitiveClosure3.cpp
 * @brief TransitiveClosure3 实现
 *
 * 实现传递闭包：Floyd-Warshall位并行优化、可达性位图压缩。
 */

#include "utils/graph226/TransitiveClosure3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransitiveClosure3::TransitiveClosure3(QObject *parent) : QObject(parent) {}
TransitiveClosure3::~TransitiveClosure3() = default;

/* ---- Bit manipulation helpers ---- */

void TransitiveClosure3::setBit(QVector<quint64>& bitmap, int pos)
{
    int word = pos >> 6;    // pos / 64
    int bit = pos & 63;     // pos % 64
    if (word >= 0 && word < bitmap.size())
        bitmap[word] |= (1ULL << bit);
}

bool TransitiveClosure3::testBit(const QVector<quint64>& bitmap, int pos)
{
    int word = pos >> 6;
    int bit = pos & 63;
    if (word >= 0 && word < bitmap.size())
        return (bitmap[word] >> bit) & 1;
    return false;
}

int TransitiveClosure3::popCount(const QVector<quint64>& bitmap)
{
    int count = 0;
    for (quint64 w : bitmap)
        count += __builtin_popcountll(w);
    return count;
}

/* ---- Build from adjacency list ---- */

void TransitiveClosure3::build(const QVector<QVector<int>>& adjList)
{
    QElapsedTimer timer;
    timer.start();

    m_n = adjList.size();
    if (m_n == 0) return;

    m_wordsPerRow = (m_n + 63) / 64;

    // Initialize reachability: each vertex reaches itself
    m_reach.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_reach[i].fill(0, m_wordsPerRow);
        setBit(m_reach[i], i);
        // Set direct successors
        for (int v : adjList[i]) {
            if (v >= 0 && v < m_n)
                setBit(m_reach[i], v);
        }
    }

    m_stats.numEdges = 0;
    for (const auto& adj : adjList) m_stats.numEdges += adj.size();

    floydWarshallBitParallel();

    // Compute density
    int totalReachable = 0;
    for (int i = 0; i < m_n; ++i)
        totalReachable += popCount(m_reach[i]);
    m_stats.reachabilityDensity = (m_n > 0)
        ? static_cast<double>(totalReachable) / (m_n * m_n) : 0.0;
    m_stats.compressedSize = m_n * m_wordsPerRow * 8;

    m_stats.numVertices = m_n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit closureComputed(m_n, m_stats.reachabilityDensity, timer.elapsed());
}

/* ---- Build from edge list ---- */

void TransitiveClosure3::buildFromEdges(const QVector<QPair<int, int>>& edges,
                                          int numVertices)
{
    QVector<QVector<int>> adjList(numVertices);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < numVertices)
            adjList[e.first].append(e.second);
    }
    build(adjList);
}

/* ---- Floyd-Warshall with bit-parallel optimization ---- */

void TransitiveClosure3::floydWarshallBitParallel()
{
    // For each intermediate vertex k, update reachability
    // Bit-parallel: process 64 vertices at once per word
    for (int k = 0; k < m_n; ++k) {
        int kWord = k >> 6;
        quint64 kMask = 1ULL << (k & 63);

        for (int i = 0; i < m_n; ++i) {
            // If i can reach k, OR i's row with k's row
            if (m_reach[i][kWord] & kMask) {
                for (int w = 0; w < m_wordsPerRow; ++w)
                    m_reach[i][w] |= m_reach[k][w];
            }
        }
    }
}

/* ---- Reachability query ---- */

bool TransitiveClosure3::canReach(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    return testBit(m_reach[u], v);
}

/* ---- Reachable from u ---- */

QVector<int> TransitiveClosure3::reachableFrom(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_n) return result;
    for (int v = 0; v < m_n; ++v) {
        if (testBit(m_reach[u], v))
            result.append(v);
    }
    return result;
}

/* ---- Reaching to v ---- */

QVector<int> TransitiveClosure3::reachingTo(int v) const
{
    QVector<int> result;
    if (v < 0 || v >= m_n) return result;
    for (int u = 0; u < m_n; ++u) {
        if (testBit(m_reach[u], v))
            result.append(u);
    }
    return result;
}

/* ---- Reachability bitmap ---- */

QVector<quint64> TransitiveClosure3::reachabilityBitmap(int u) const
{
    if (u < 0 || u >= m_n) return {};
    return m_reach[u];
}

/* ---- Batch query ---- */

QVector<bool> TransitiveClosure3::batchQuery(const QVector<QPair<int, int>>& queries) const
{
    QVector<bool> results;
    results.reserve(queries.size());
    for (const auto& q : queries)
        results.append(canReach(q.first, q.second));
    return results;
}

/* ---- Closure matrix ---- */

QVector<QVector<quint64>> TransitiveClosure3::closureMatrix() const
{
    return m_reach;
}

/* ---- Reset ---- */

void TransitiveClosure3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reach.clear();
    m_n = 0;
    m_wordsPerRow = 0;
}
