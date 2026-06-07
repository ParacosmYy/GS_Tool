/**
 * @file TransitiveClosure2.cpp
 * @brief TransitiveClosure2 实现
 *
 * 实现传递闭包：Floyd-Warshall算法、位打包稠密图优化、可达性查询。
 */

#include "utils/graph201/TransitiveClosure2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransitiveClosure2::TransitiveClosure2(QObject *parent) : QObject(parent) {}
TransitiveClosure2::~TransitiveClosure2() = default;

/* ---- Configuration ---- */

void TransitiveClosure2::setAdjacencyMatrix(const QVector<QVector<int>>& matrix)
{
    m_numVertices = matrix.size();
    m_closure = matrix;
    m_bitClosure.clear();
    m_bitWords = 0;
}

void TransitiveClosure2::setEdgeList(int numVertices,
                                      const QVector<QPair<int, int>>& edges)
{
    m_numVertices = numVertices;
    m_closure.assign(numVertices, QVector<int>(numVertices, 0));
    for (int i = 0; i < numVertices; ++i) m_closure[i][i] = 1;
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < numVertices &&
            e.second >= 0 && e.second < numVertices)
            m_closure[e.first][e.second] = 1;
    }
    m_bitClosure.clear();
    m_bitWords = 0;
}

/* ---- Ensure closure computed ---- */

void TransitiveClosure2::ensureClosure()
{
    if (m_closure.isEmpty() || m_closure[0].size() != m_numVertices) return;
    // Check if already computed (reflexive test as heuristic)
}

void TransitiveClosure2::ensureBitClosure()
{
    if (!m_bitClosure.isEmpty()) return;
    // Compute integer closure first if needed
    if (m_closure.isEmpty() || m_closure.size() != m_numVertices)
        return;
    m_bitWords = (m_numVertices + 63) / 64;
    m_bitClosure.resize(m_numVertices * m_bitWords, 0ULL);
    for (int i = 0; i < m_numVertices; ++i)
        for (int j = 0; j < m_numVertices; ++j)
            if (m_closure[i][j])
                m_bitClosure[i * m_bitWords + j / 64] |= (1ULL << (j % 64));
}

/* ---- Floyd-Warshall transitive closure ---- */

QVector<QVector<int>> TransitiveClosure2::compute()
{
    QElapsedTimer timer;
    timer.start();

    // Ensure reflexive
    for (int i = 0; i < m_numVertices; ++i)
        if (i < m_closure.size() && i < m_closure[i].size())
            m_closure[i][i] = 1;

    // Floyd-Warshall: closure[i][j] |= closure[i][k] && closure[k][j]
    for (int k = 0; k < m_numVertices; ++k) {
        for (int i = 0; i < m_numVertices; ++i) {
            if (m_closure[i][k] == 0) continue;
            for (int j = 0; j < m_numVertices; ++j) {
                m_closure[i][j] |= (m_closure[k][j] & m_closure[i][k]);
            }
        }
    }

    m_bitClosure.clear(); // Invalidate bit-packed cache

    int edges = 0;
    for (int i = 0; i < m_numVertices; ++i)
        for (int j = 0; j < m_numVertices; ++j)
            edges += m_closure[i][j];

    m_stats.totalRuns++;
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = edges;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit closureComputed(m_numVertices, timer.elapsed());
    return m_closure;
}

/* ---- Bit-packed transitive closure ---- */

QVector<quint64> TransitiveClosure2::computeBitPacked()
{
    QElapsedTimer timer;
    timer.start();

    m_bitWords = (m_numVertices + 63) / 64;
    m_bitClosure.resize(m_numVertices * m_bitWords, 0ULL);

    // Initialize from adjacency matrix
    for (int i = 0; i < m_numVertices; ++i) {
        // Set self-loop
        m_bitClosure[i * m_bitWords + i / 64] |= (1ULL << (i % 64));
        for (int j = 0; j < m_numVertices; ++j) {
            if (m_closure[i][j])
                m_bitClosure[i * m_bitWords + j / 64] |= (1ULL << (j % 64));
        }
    }

    // Floyd-Warshall with bit-packing
    for (int k = 0; k < m_numVertices; ++k) {
        int kW = k / 64;
        quint64 kBit = 1ULL << (k % 64);
        for (int i = 0; i < m_numVertices; ++i) {
            // Check if i->k exists
            if (!(m_bitClosure[i * m_bitWords + kW] & kBit)) continue;
            // OR the k-th row into i-th row
            for (int w = 0; w < m_bitWords; ++w)
                m_bitClosure[i * m_bitWords + w] |= m_bitClosure[k * m_bitWords + w];
        }
    }

    // Sync integer closure
    for (int i = 0; i < m_numVertices; ++i) {
        if (m_closure.size() <= i) m_closure.resize(i + 1);
        m_closure[i].resize(m_numVertices, 0);
        for (int j = 0; j < m_numVertices; ++j) {
            m_closure[i][j] = (m_bitClosure[i * m_bitWords + j / 64] >> (j % 64)) & 1;
        }
    }

    m_stats.totalRuns++;
    m_stats.numVertices = m_numVertices;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit closureComputed(m_numVertices, timer.elapsed());
    return m_bitClosure;
}

/* ---- Reachability queries ---- */

bool TransitiveClosure2::reachable(int u, int v) const
{
    if (u < 0 || u >= m_numVertices || v < 0 || v >= m_numVertices) return false;
    if (u >= m_closure.size() || v >= m_closure[u].size()) return false;
    return m_closure[u][v] != 0;
}

bool TransitiveClosure2::reachablePacked(int u, int v) const
{
    if (u < 0 || u >= m_numVertices || v < 0 || v >= m_numVertices) return false;
    if (m_bitClosure.isEmpty()) return false;
    return (m_bitClosure[u * m_bitWords + v / 64] >> (v % 64)) & 1;
}

/* ---- Reachable set ---- */

QVector<int> TransitiveClosure2::reachableSet(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_numVertices) return result;
    for (int v = 0; v < m_numVertices; ++v)
        if (reachable(u, v)) result.append(v);
    return result;
}

/* ---- Count strongly connected components ---- */

int TransitiveClosure2::countStronglyConnected() const
{
    if (m_numVertices == 0) return 0;
    QVector<bool> visited(m_numVertices, false);
    int count = 0;
    for (int i = 0; i < m_numVertices; ++i) {
        if (visited[i]) continue;
        // Start new SCC: all mutually reachable from i
        visited[i] = true;
        for (int j = i + 1; j < m_numVertices; ++j) {
            if (!visited[j] && reachable(i, j) && reachable(j, i)) {
                visited[j] = true;
            }
        }
        ++count;
    }
    return count;
}

/* ---- Reset ---- */

void TransitiveClosure2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
