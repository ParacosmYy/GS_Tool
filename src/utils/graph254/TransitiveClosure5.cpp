/**
 * @file TransitiveClosure5.cpp
 * @brief TransitiveClosure5 实现
 *
 * 实现传递闭包：Roy-Warshall位向量优化与压缩稀疏行可达矩阵。
 */

#include "utils/graph254/TransitiveClosure5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransitiveClosure5::TransitiveClosure5(QObject *parent) : QObject(parent) {}
TransitiveClosure5::~TransitiveClosure5() = default;

/* ---- Configuration ---- */

void TransitiveClosure5::setNumVertices(int n)
{
    m_n = qMax(0, n);
    m_blocks = (m_n + 63) / 64;
    // Initialize empty bit-vector matrix
    m_matrix.assign(m_n, QVector<quint64>(m_blocks, 0));
    // Set diagonal: each vertex reaches itself
    for (int i = 0; i < m_n; ++i)
        setBit(i, i);
    m_csrValid = false;
    m_stats.numVertices = m_n;
}

/* ---- Add edge ---- */

void TransitiveClosure5::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    setBit(u, v);
    m_csrValid = false;
    m_stats.numEdges++;
}

/* ---- Build from adjacency list ---- */

void TransitiveClosure5::buildFromAdjList(const QVector<QVector<int>>& adj)
{
    int n = adj.size();
    setNumVertices(n);
    for (int u = 0; u < n; ++u)
        for (int v : adj[u])
            setBit(u, v);
    m_csrValid = false;
}

/* ---- Bit operations ---- */

void TransitiveClosure5::setBit(int u, int v)
{
    int block = v / 64;
    int bit = v % 64;
    m_matrix[u][block] |= (1ULL << bit);
}

bool TransitiveClosure5::testBit(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    int block = v / 64;
    int bit = v % 64;
    return (m_matrix[u][block] >> bit) & 1;
}

/* ---- Compute transitive closure (Roy-Warshall with bit-vector) ---- */

void TransitiveClosure5::compute()
{
    QElapsedTimer timer;
    timer.start();

    // Roy-Warshall: for each pivot k, update reachability
    // Bit-vector optimization: process entire 64-bit words at once
    for (int k = 0; k < m_n; ++k) {
        int kBlock = k / 64;
        quint64 kMask = 1ULL << (k % 64);

        for (int i = 0; i < m_n; ++i) {
            // If i can reach k
            if (m_matrix[i][kBlock] & kMask) {
                // i can also reach everything k can reach: row_i |= row_k
                for (int b = 0; b < m_blocks; ++b)
                    m_matrix[i][b] |= m_matrix[k][b];
            }
        }
    }

    m_csrValid = false;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit computeCompleted(m_n, m_stats.numEdges, timer.elapsed());
}

/* ---- Query reachability ---- */

bool TransitiveClosure5::reachable(int u, int v) const
{
    return testBit(u, v);
}

/* ---- Get reachable set ---- */

QVector<int> TransitiveClosure5::reachableSet(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_n) return result;

    for (int v = 0; v < m_n; ++v) {
        if (testBit(u, v))
            result.append(v);
    }
    return result;
}

/* ---- Get reachability row as QBitArray ---- */

QBitArray TransitiveClosure5::reachabilityRow(int u) const
{
    if (u < 0 || u >= m_n) return {};

    QBitArray bits(m_n, false);
    for (int v = 0; v < m_n; ++v)
        bits[v] = testBit(u, v);
    return bits;
}

/* ---- Build CSR from bit-vector matrix ---- */

void TransitiveClosure5::buildCSR()
{
    m_csrOffsets.resize(m_n + 1, 0);
    m_csrTargets.clear();

    // Count targets per row
    for (int u = 0; u < m_n; ++u) {
        int count = 0;
        for (int v = 0; v < m_n; ++v)
            if (testBit(u, v)) count++;
        m_csrOffsets[u + 1] = m_csrOffsets[u] + count;
    }

    m_csrTargets.resize(m_csrOffsets[m_n]);
    QVector<int> writePos(m_n, 0);
    for (int u = 0; u < m_n; ++u) {
        int base = m_csrOffsets[u];
        for (int v = 0; v < m_n; ++v) {
            if (testBit(u, v))
                m_csrTargets[base + writePos[u]++] = v;
        }
    }
    m_csrValid = true;
}

/* ---- Reset ---- */

void TransitiveClosure5::resetStatistics()
{
    m_matrix.clear();
    m_csrOffsets.clear(); m_csrTargets.clear();
    m_n = 0; m_blocks = 0; m_csrValid = false;
    m_stats = Stats{}; m_timeSum = 0.0;
}
