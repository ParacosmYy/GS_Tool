/**
 * @file StoerWagner.cpp
 * @brief StoerWagner 实现
 *
 * 实现Stoer-Wagner全局最小割：最大邻接序(MA ordering) + 反复收缩顶点。
 */

#include "utils/graph180/StoerWagner.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

StoerWagner::StoerWagner(QObject* parent)
    : QObject(parent)
{
}

StoerWagner::~StoerWagner() = default;

void StoerWagner::setAdjacencyMatrix(const QVector<QVector<double>>& matrix)
{
    m_n = matrix.size();
    m_adjMatrix = matrix;
    /* Ensure square */
    for (auto& row : m_adjMatrix)
        row.resize(m_n, 0.0);
}

void StoerWagner::setEdgeList(int n, const QVector<QPair<QPair<int, int>, double>>& edges)
{
    m_n = n;
    m_adjMatrix.resize(n);
    for (int i = 0; i < n; ++i)
        m_adjMatrix[i].fill(0.0, n);

    for (const auto& edge : edges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double w = edge.second;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            m_adjMatrix[u][v] += w;
            m_adjMatrix[v][u] += w;
        }
    }
}

QVector<int> StoerWagner::maOrdering(const QVector<bool>& merged,
                                      const QVector<QVector<double>>& adj) const
{
    int n = m_n;
    QVector<int> order;
    QVector<bool> visited(n, false);
    QVector<double> key(n, 0.0);

    /* Start from first non-merged vertex */
    int start = 0;
    while (start < n && merged[start]) start++;
    if (start >= n) return order;

    visited[start] = true;
    order.append(start);

    /* Initialize keys with edges from start */
    for (int v = 0; v < n; ++v) {
        if (!merged[v] && !visited[v])
            key[v] = adj[start][v];
    }

    /* Greedy: always add the vertex with maximum key value */
    for (int step = 1; step < n; ++step) {
        int best = -1;
        double bestKey = -1.0;
        for (int v = 0; v < n; ++v) {
            if (!merged[v] && !visited[v] && key[v] > bestKey) {
                bestKey = key[v];
                best = v;
            }
        }
        if (best < 0) break;

        visited[best] = true;
        order.append(best);

        /* Update keys */
        for (int v = 0; v < n; ++v) {
            if (!merged[v] && !visited[v])
                key[v] += adj[best][v];
        }
    }

    return order;
}

QPair<double, QPair<int, int>> StoerWagner::minimumCutPhase(
    const QVector<bool>& merged,
    const QVector<QVector<double>>& adj) const
{
    QVector<int> order = maOrdering(merged, adj);

    if (order.size() < 2) {
        return qMakePair(std::numeric_limits<double>::max(),
                         qMakePair(-1, -1));
    }

    /* The cut of this phase: last vertex separated from the rest */
    int t = order.last();
    int s = order[order.size() - 2];

    /* Compute cut weight: sum of edges from t to all others */
    double cutWeight = 0.0;
    for (int v = 0; v < m_n; ++v) {
        if (v != t && !merged[v])
            cutWeight += adj[t][v];
    }

    return qMakePair(cutWeight, qMakePair(s, t));
}

StoerWagner::CutResult StoerWagner::computeMinCut()
{
    QElapsedTimer timer;
    timer.start();

    CutResult best;
    best.weight = std::numeric_limits<double>::max();

    if (m_n <= 0) return best;

    /* Track merged vertices and edge counts */
    QVector<bool> merged(m_n, false);
    QVector<QVector<double>> adj = m_adjMatrix;
    int edgeCount = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (adj[i][j] > 0) edgeCount++;

    /* Track which original vertices belong to each supernode */
    QVector<QVector<int>> groups(m_n);
    for (int i = 0; i < m_n; ++i) groups[i].append(i);

    int remaining = m_n;
    QVector<int> lastPair(2, -1);

    for (int phase = 0; phase < m_n - 1; ++phase) {
        auto result = minimumCutPhase(merged, adj);
        double cutWeight = result.first;
        int s = result.second.first;
        int t = result.second.second;

        if (s < 0 || t < 0) break;

        if (cutWeight < best.weight) {
            best.weight = cutWeight;
            lastPair[0] = s;
            lastPair[1] = t;
        }

        /* Merge s and t: absorb t into s */
        for (int v = 0; v < m_n; ++v) {
            if (v != s && v != t && !merged[v]) {
                adj[s][v] += adj[t][v];
                adj[v][s] += adj[t][v];
            }
            adj[t][v] = 0.0;
            adj[v][t] = 0.0;
        }
        merged[t] = true;
        groups[s].append(groups[t]);
        groups[t].clear();
        remaining--;
    }

    /* Reconstruct partition from the best cut */
    /* Find which supernode was the "t" in the best phase */
    /* Use union-find over the merge sequence to find the two partitions */
    QVector<int> parent(m_n);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    /* Re-run merges to find the cut that gave best.weight */
    merged.fill(false);
    adj = m_adjMatrix;
    int bestS = -1, bestT = -1;

    for (int phase = 0; phase < m_n - 1; ++phase) {
        auto result = minimumCutPhase(merged, adj);
        if (result.first <= best.weight + 1e-10) {
            bestS = result.second.first;
            bestT = result.second.second;
            break;
        }
        int s = result.second.first;
        int t = result.second.second;
        if (s < 0 || t < 0) break;
        for (int v = 0; v < m_n; ++v) {
            if (v != s && v != t && !merged[v]) {
                adj[s][v] += adj[t][v];
                adj[v][s] += adj[t][v];
            }
            adj[t][v] = 0.0;
            adj[v][t] = 0.0;
        }
        merged[t] = true;
    }

    /* The partition is: all vertices merged into bestT vs the rest */
    /* Rebuild groups properly */
    merged.fill(false);
    QVector<int> superGroup(m_n);
    for (int i = 0; i < m_n; ++i) superGroup[i] = i;

    adj = m_adjMatrix;
    for (int phase = 0; phase < m_n - 1; ++phase) {
        auto result = minimumCutPhase(merged, adj);
        int s = result.second.first;
        int t = result.second.second;
        if (s < 0 || t < 0) break;

        if (result.first <= best.weight + 1e-10 && t == bestT) {
            /* This is the cut phase: t's group vs rest */
            break;
        }

        for (int v = 0; v < m_n; ++v) {
            if (v != s && v != t && !merged[v]) {
                adj[s][v] += adj[t][v];
                adj[v][s] += adj[t][v];
            }
            adj[t][v] = 0.0;
            adj[v][t] = 0.0;
        }
        merged[t] = true;
        /* Merge t's group into s */
        for (int i = 0; i < m_n; ++i) {
            if (superGroup[i] == t) superGroup[i] = s;
        }
    }

    /* Partition by whether in bestT's group */
    int targetGroup = bestT;
    /* Find which supernode bestT belongs to */
    targetGroup = superGroup[bestT];
    for (int i = 0; i < m_n; ++i) {
        if (superGroup[i] == targetGroup)
            best.partitionA.append(i);
        else
            best.partitionB.append(i);
    }

    m_stats.totalRuns++;
    m_stats.lastVertexCount = m_n;
    m_stats.lastEdgeCount = edgeCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit minCutComputed(best.weight);
    return best;
}

void StoerWagner::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
