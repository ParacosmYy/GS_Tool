/**
 * @file TransitiveClosure.cpp
 * @brief TransitiveClosure 实现
 *
 * 实现传递闭包：Floyd-Warshall O(n³)和Warshall位集优化。
 */

#include "utils/graph184/TransitiveClosure.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ---- Construction ---- */

TransitiveClosure::TransitiveClosure(QObject *parent) : QObject(parent) {}
TransitiveClosure::~TransitiveClosure() = default;

/* ---- Configuration ---- */

void TransitiveClosure::setAlgorithm(Algorithm algo) { m_algo = algo; }

/* ---- Build from adjacency list ---- */

void TransitiveClosure::build(const QVector<QVector<int>>& adjList, int n)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    m_closure.assign(n, QVector<bool>(n, false));

    /* Initialize: reflexive + direct edges */
    for (int i = 0; i < n; ++i) {
        m_closure[i][i] = true;
        for (int v : adjList.value(i))
            if (v >= 0 && v < n) m_closure[i][v] = true;
    }

    /* Count edges */
    int edges = 0;
    for (const auto& list : adjList) edges += list.size();

    /* Run selected algorithm */
    switch (m_algo) {
    case FloydWarshall: runFloydWarshall(); break;
    case WarshallBitset: runWarshallBitset(); break;
    }

    m_stats.totalRuns++;
    m_stats.lastVertices = n;
    m_stats.lastEdges = edges;
    m_stats.reachablePairs = countReachablePairs();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit closureComputed(n, m_stats.reachablePairs);
}

/* ---- Build from edge list ---- */

void TransitiveClosure::buildFromEdges(const QVector<QPair<int, int>>& edges, int n)
{
    QVector<QVector<int>> adjList(n);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n)
            adjList[e.first].append(e.second);
    }
    build(adjList, n);
}

/* ---- Floyd-Warshall O(n³) ---- */

void TransitiveClosure::runFloydWarshall()
{
    for (int k = 0; k < m_n; ++k)
        for (int i = 0; i < m_n; ++i)
            if (m_closure[i][k])
                for (int j = 0; j < m_n; ++j)
                    m_closure[i][j] = m_closure[i][j] || m_closure[k][j];
}

/* ---- Warshall with bitset optimization ---- */

void TransitiveClosure::runWarshallBitset()
{
    /* Pack each row into 64-bit words */
    int wordsPerRow = (m_n + 63) / 64;

    QVector<quint64> rows(m_n * wordsPerRow, 0);

    /* Initialize bitset from closure */
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            if (m_closure[i][j])
                rows[i * wordsPerRow + j / 64] |= (1ULL << (j % 64));
        }
    }

    /* Warshall: for each intermediate k, OR row k into row i if bit k is set */
    for (int k = 0; k < m_n; ++k) {
        int kW = k / 64;
        quint64 kMask = 1ULL << (k % 64);
        for (int i = 0; i < m_n; ++i) {
            if (rows[i * wordsPerRow + kW] & kMask) {
                for (int w = 0; w < wordsPerRow; ++w)
                    rows[i * wordsPerRow + w] |= rows[k * wordsPerRow + w];
            }
        }
    }

    /* Unpack back to boolean matrix */
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            m_closure[i][j] =
                (rows[i * wordsPerRow + j / 64] >> (j % 64)) & 1;
        }
    }
}

/* ---- Queries ---- */

bool TransitiveClosure::reachable(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    return m_closure[u][v];
}

QVector<int> TransitiveClosure::reachableFrom(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_n) return result;
    for (int v = 0; v < m_n; ++v)
        if (m_closure[u][v]) result.append(v);
    return result;
}

QVector<QVector<bool>> TransitiveClosure::closureMatrix() const
{
    return m_closure;
}

quint64 TransitiveClosure::countReachablePairs() const
{
    quint64 count = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            if (m_closure[i][j]) count++;
    return count;
}

/* ---- Statistics ---- */

void TransitiveClosure::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
