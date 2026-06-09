/**
 * @file TransitiveClosure6.cpp
 * @brief TransitiveClosure6 实现
 *
 * 实现传递闭包：Floyd-Warshall全对可达性与后继矩阵路径重建。
 */

#include "utils/graph268/TransitiveClosure6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

TransitiveClosure6::TransitiveClosure6(QObject *parent)
    : QObject(parent) {}
TransitiveClosure6::~TransitiveClosure6() = default;

/* ---- Set graph adjacency matrix ---- */

void TransitiveClosure6::setGraph(const QVector<QVector<double>>& adjMatrix)
{
    m_n = adjMatrix.size();
    m_dist = adjMatrix;
    m_reachability.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_reachability[i].resize(m_n, false);

    m_stats.numNodes = m_n;
    m_stats.numEdges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            if (i != j && adjMatrix[i][j] < INF / 2) m_stats.numEdges++;
}

/* ---- Initialize successor matrix ---- */

void TransitiveClosure6::initSuccessor()
{
    m_successor.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_successor[i].resize(m_n, -1);
        for (int j = 0; j < m_n; ++j) {
            if (i == j) {
                m_successor[i][j] = i;
            } else if (m_dist[i][j] < INF / 2) {
                m_successor[i][j] = j;
            }
        }
    }
}

/* ---- Compute transitive closure (Floyd-Warshall) ---- */

QVector<QVector<bool>> TransitiveClosure6::compute()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    initSuccessor();

    // Floyd-Warshall: O(V^3)
    for (int k = 0; k < m_n; ++k) {
        for (int i = 0; i < m_n; ++i) {
            for (int j = 0; j < m_n; ++j) {
                double through = m_dist[i][k] + m_dist[k][j];
                if (through < m_dist[i][j]) {
                    m_dist[i][j] = through;
                    // Update successor: next node on path i->j
                    m_successor[i][j] = m_successor[i][k];
                }
            }
        }
    }

    // Build reachability matrix
    int reachablePairs = 0;
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            m_reachability[i][j] = (m_dist[i][j] < INF / 2);
            if (m_reachability[i][j] && i != j) reachablePairs++;
        }
    }

    m_stats.numReachablePairs = reachablePairs;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit closureComputed(m_n, reachablePairs, elapsed);
    return m_reachability;
}

/* ---- Distance matrix ---- */

QVector<QVector<double>> TransitiveClosure6::distanceMatrix() const
{
    return m_dist;
}

/* ---- Reconstruct shortest path via successor matrix ---- */

QVector<int> TransitiveClosure6::reconstructPath(int from, int to) const
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return {};
    if (!m_reachability[from][to]) return {};

    QVector<int> path;
    path.append(from);
    int current = from;
    // Follow successor chain, with cycle guard
    int maxSteps = m_n + 1;
    while (current != to && maxSteps-- > 0) {
        int next = m_successor[current][to];
        if (next < 0 || next == current) break;
        path.append(next);
        current = next;
    }
    if (current != to) return {};

    double pathLen = 0.0;
    for (int i = 0; i + 1 < path.size(); ++i)
        pathLen += m_dist[path[i]][path[i + 1]];

    m_pathSum += pathLen;
    m_pathCount++;
    m_stats.avgPathLength = m_pathSum / m_pathCount;

    return path;
}

/* ---- Check reachability ---- */

bool TransitiveClosure6::isReachable(int from, int to) const
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return false;
    return m_reachability[from][to];
}

/* ---- Reset ---- */

void TransitiveClosure6::resetStatistics()
{
    m_dist.clear();
    m_successor.clear();
    m_reachability.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_pathSum = 0.0;
    m_pathCount = 0;
}
