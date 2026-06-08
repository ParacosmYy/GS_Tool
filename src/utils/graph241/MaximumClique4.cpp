/**
 * @file MaximumClique4.cpp
 * @brief MaximumClique4 实现
 *
 * 实现位并行Bron-Kerbosch枢轴算法与退化序密集图剪枝。
 */

#include "utils/graph241/MaximumClique4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MaximumClique4::MaximumClique4(QObject *parent) : QObject(parent) {}
MaximumClique4::~MaximumClique4() = default;

/* ---- Popcount ---- */

int MaximumClique4::popcount(quint64 v)
{
    int count = 0;
    while (v) {
        count++;
        v &= v - 1;
    }
    return count;
}

/* ---- Set graph via adjacency matrix ---- */

void MaximumClique4::setGraph(const QVector<QVector<int>>& adjacencyMatrix)
{
    int n = adjacencyMatrix.size();
    m_numVertices = n;
    m_adjBit.resize(n, 0);
    m_adjList.resize(n);
    int edges = 0;

    for (int i = 0; i < n; ++i) {
        m_adjBit[i] = 0;
        m_adjList[i].clear();
        for (int j = 0; j < n; ++j) {
            if (adjacencyMatrix[i][j] && i != j) {
                if (j < 64) m_adjBit[i] |= (1ULL << j);
                m_adjList[i].append(j);
                edges++;
            }
        }
    }
    m_stats.numVertices = n;
    m_stats.numEdges = edges / 2;
}

/* ---- Set graph via edge list ---- */

void MaximumClique4::setGraph(int vertices,
                                const QVector<QPair<int, int>>& edges)
{
    m_numVertices = vertices;
    m_adjBit.resize(vertices, 0);
    m_adjList.resize(vertices);
    for (int i = 0; i < vertices; ++i) {
        m_adjBit[i] = 0;
        m_adjList[i].clear();
    }

    for (const auto& e : edges) {
        if (e.first < 64) m_adjBit[e.first] |= (1ULL << e.first);
        if (e.second < 64) m_adjBit[e.first] |= (1ULL << e.second);
        m_adjList[e.first].append(e.second);
        m_adjList[e.second].append(e.first);
    }
    m_stats.numVertices = vertices;
    m_stats.numEdges = edges.size();
}

/* ---- Choose pivot (Tomita heuristic) ---- */

int MaximumClique4::choosePivot(quint64 P, quint64 X) const
{
    quint64 PU = P | X;
    int best = -1;
    int bestCount = -1;

    quint64 temp = PU;
    while (temp) {
        int v = __builtin_ctzll(temp);
        int count = popcount(P & m_adjBit[v]);
        if (count > bestCount) {
            bestCount = count;
            best = v;
        }
        temp &= temp - 1;
    }
    return best;
}

/* ---- Bitmask to vertex list ---- */

QVector<int> MaximumClique4::bitsetToVertices(quint64 mask) const
{
    QVector<int> result;
    while (mask) {
        int v = __builtin_ctzll(mask);
        result.append(v);
        mask &= mask - 1;
    }
    return result;
}

/* ---- Bit-parallel Bron-Kerbosch with pivot ---- */

void MaximumClique4::bkSearch(quint64 R, quint64 P, quint64 X, int depth)
{
    if (P == 0 && X == 0) {
        int size = popcount(R);
        m_stats.numBacktracks++;
        if (size > m_bestSize) {
            m_bestSize = size;
            m_bestClique = bitsetToVertices(R);
        }
        return;
    }

    // Pruning: even if all remaining vertices join, can we beat best?
    if (popcount(R) + popcount(P) <= m_bestSize)
        return;

    int u = choosePivot(P, X);
    if (u < 0) return;

    // Candidates = P \ N(u)
    quint64 candidates = P & ~m_adjBit[u];

    while (candidates) {
        int v = __builtin_ctzll(candidates);
        quint64 bit = 1ULL << v;

        bkSearch(R | bit, P & m_adjBit[v], X & m_adjBit[v], depth + 1);

        P &= ~bit;
        X |= bit;
        candidates &= ~bit;
    }
}

/* ---- Degeneracy ordering ---- */

QVector<int> MaximumClique4::degeneracyOrdering() const
{
    int n = m_numVertices;
    QVector<int> degree(n);
    for (int i = 0; i < n; ++i)
        degree[i] = m_adjList[i].size();

    QVector<bool> removed(n, false);
    QVector<int> order;
    order.reserve(n);

    for (int iter = 0; iter < n; ++iter) {
        // Find minimum degree among non-removed vertices
        int minDeg = std::numeric_limits<int>::max();
        int minV = -1;
        for (int i = 0; i < n; ++i) {
            if (!removed[i] && degree[i] < minDeg) {
                minDeg = degree[i];
                minV = i;
            }
        }
        if (minV < 0) break;

        order.append(minV);
        removed[minV] = true;
        for (int nb : m_adjList[minV])
            if (!removed[nb]) degree[nb]--;
    }
    return order;
}

/* ---- Graph density ---- */

double MaximumClique4::density() const
{
    if (m_numVertices < 2) return 0.0;
    double maxEdges = static_cast<double>(m_numVertices) *
                      (m_numVertices - 1) / 2.0;
    return (maxEdges > 0) ? m_stats.numEdges / maxEdges : 0.0;
}

/* ---- Solve ---- */

QVector<int> MaximumClique4::solve()
{
    QElapsedTimer timer;
    timer.start();

    m_bestClique.clear();
    m_bestSize = 0;
    m_stats.numBacktracks = 0;

    // Initial candidate set: all vertices
    quint64 P = 0;
    int limit = qMin(m_numVertices, 64);
    for (int i = 0; i < limit; ++i)
        P |= (1ULL << i);

    bkSearch(0, P, 0, 0);

    m_stats.maxCliqueSize = m_bestSize;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cliqueFound(m_bestSize, timer.elapsed());
    return m_bestClique;
}

/* ---- Reset ---- */

void MaximumClique4::resetStatistics()
{
    m_adjBit.clear();
    m_adjList.clear();
    m_bestClique.clear();
    m_bestSize = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
