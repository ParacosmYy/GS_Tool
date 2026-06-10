/**
 * @file TransitiveClosure7.cpp
 * @brief TransitiveClosure7 实现
 *
 * 实现传递闭包：Floyd-Warshall位向量优化与Warshall位并行可达性计算。
 */

#include "utils/graph282/TransitiveClosure7.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransitiveClosure7::TransitiveClosure7(QObject *parent)
    : QObject(parent) {}

TransitiveClosure7::~TransitiveClosure7() = default;

/* ---- Build from adjacency matrix ---- */

void TransitiveClosure7::setAdjacencyMatrix(const QVector<QVector<int>>& matrix)
{
    m_n = matrix.size();
    m_adjMatrix = matrix;
    // Ensure diagonal is set (each vertex reaches itself)
    for (int i = 0; i < m_n; ++i) {
        if (i < m_adjMatrix.size() && i < m_adjMatrix[i].size())
            m_adjMatrix[i][i] = 1;
    }
}

/* ---- Build from edge list ---- */

void TransitiveClosure7::setEdgeList(int numVertices,
                                      const QVector<QPair<int, int>>& edges)
{
    m_n = numVertices;
    m_adjMatrix.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_adjMatrix[i].fill(0, m_n);

    // Set diagonal
    for (int i = 0; i < m_n; ++i)
        m_adjMatrix[i][i] = 1;

    // Set edges
    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < m_n &&
            edge.second >= 0 && edge.second < m_n) {
            m_adjMatrix[edge.first][edge.second] = 1;
        }
    }
}

/* ---- Pack adjacency into bit-vectors ---- */

void TransitiveClosure7::packBits()
{
    m_bitCols = (m_n + 63) / 64;
    m_bitRows.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_bitRows[i].fill(0, m_bitCols);
        for (int j = 0; j < m_n; ++j) {
            if (i < m_adjMatrix.size() && j < m_adjMatrix[i].size() &&
                m_adjMatrix[i][j]) {
                int word = j / 64;
                int bit = j % 64;
                m_bitRows[i][word] |= (1ULL << bit);
            }
        }
    }
}

/* ---- Unpack bit-vectors to int matrix ---- */

QVector<QVector<int>> TransitiveClosure7::unpackBits() const
{
    QVector<QVector<int>> result(m_n, QVector<int>(m_n, 0));
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            int word = j / 64;
            int bit = j % 64;
            if (word < m_bitRows[i].size() &&
                (m_bitRows[i][word] & (1ULL << bit)))
                result[i][j] = 1;
        }
    }
    return result;
}

/* ---- Standard Floyd-Warshall transitive closure ---- */

QVector<QVector<int>> TransitiveClosure7::compute()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    // Initialize closure from adjacency matrix
    m_closure = m_adjMatrix;

    // Floyd-Warshall: for each intermediate vertex k
    for (int k = 0; k < m_n; ++k) {
        for (int i = 0; i < m_n; ++i) {
            if (m_closure[i][k]) {
                for (int j = 0; j < m_n; ++j) {
                    m_closure[i][j] = m_closure[i][j] || m_closure[k][j];
                }
            }
        }
    }

    // Count reachable pairs
    int reachable = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            if (m_closure[i][j]) reachable++;

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numReachable = reachable;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit closureComputed(m_n, reachable, elapsed);

    return m_closure;
}

/* ---- Bit-parallel Warshall algorithm ---- */

QVector<QVector<int>> TransitiveClosure7::computeBitParallel()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    // Pack adjacency into bit-vectors
    packBits();

    // Bit-parallel Warshall: for each intermediate vertex k
    // Row[k] is a bitmask of vertices reachable from k
    // For each row i: if i->k, then row[i] |= row[k]
    for (int k = 0; k < m_n; ++k) {
        int kWord = k / 64;
        int kBit = k % 64;
        for (int i = 0; i < m_n; ++i) {
            // Check if i can reach k
            if (kWord < m_bitRows[i].size() &&
                (m_bitRows[i][kWord] & (1ULL << kBit))) {
                // i->k: merge row k into row i via bitwise OR
                for (int w = 0; w < m_bitCols; ++w) {
                    m_bitRows[i][w] |= m_bitRows[k][w];
                }
            }
        }
    }

    // Unpack result
    m_closure = unpackBits();

    int reachable = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            if (m_closure[i][j]) reachable++;

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numReachable = reachable;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit closureComputed(m_n, reachable, elapsed);

    return m_closure;
}

/* ---- Reachability queries ---- */

bool TransitiveClosure7::isReachable(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    if (m_closure.isEmpty()) return false;
    return m_closure[u][v] != 0;
}

QVector<int> TransitiveClosure7::reachableFrom(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_n || m_closure.isEmpty()) return result;
    for (int v = 0; v < m_n; ++v) {
        if (m_closure[u][v]) result.append(v);
    }
    return result;
}

/* ---- Reset ---- */

void TransitiveClosure7::resetStatistics()
{
    m_adjMatrix.clear();
    m_closure.clear();
    m_bitRows.clear();
    m_eulerTour.clear();
    m_n = 0;
    m_bitCols = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
