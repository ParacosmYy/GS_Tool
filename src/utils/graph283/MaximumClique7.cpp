/**
 * @file MaximumClique7.cpp
 * @brief MaximumClique7 实现
 *
 * 实现最大团：Bron-Kerbosch轴心选择与退化排序高效极大枚举。
 */

#include "utils/graph283/MaximumClique7.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MaximumClique7::MaximumClique7(QObject *parent)
    : QObject(parent) {}

MaximumClique7::~MaximumClique7() = default;

/* ---- Graph loading ---- */

void MaximumClique7::setAdjacencyMatrix(const QVector<QVector<int>>& matrix)
{
    m_n = matrix.size();
    m_adjMatrix = matrix;
    m_adj.resize(m_n);

    int edges = 0;
    for (int i = 0; i < m_n; ++i) {
        m_adj[i].clear();
        for (int j = 0; j < m_n; ++j) {
            if (i != j && matrix[i][j])
                m_adj[i].append(j);
        }
        edges += m_adj[i].size();
    }
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges / 2;
}

void MaximumClique7::setEdgeList(int numVertices, const QVector<QPair<int,int>>& edges)
{
    m_n = numVertices;
    m_adjMatrix.resize(m_n);
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_adj[i].clear();
        m_adjMatrix[i].resize(m_n);
        m_adjMatrix[i].fill(0);
    }
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_n && e.second >= 0 && e.second < m_n) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
            m_adjMatrix[e.first][e.second] = 1;
            m_adjMatrix[e.second][e.first] = 1;
        }
    }
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();
}

/* ---- Intersect with neighbors ---- */

QVector<int> MaximumClique7::intersectWithNeighbors(const QVector<int>& set, int v) const
{
    QVector<int> result;
    for (int u : set) {
        if (m_adjMatrix[v][u]) result.append(u);
    }
    return result;
}

/* ---- Pivot selection (Tomita: maximize |P ∩ N(u)|) ---- */

int MaximumClique7::selectPivot(const QVector<int>& P, const QVector<int>& X) const
{
    int best = -1;
    int bestCount = -1;

    // Choose from P ∪ X
    QVector<int> candidates = P;
    for (int x : X) candidates.append(x);

    for (int u : candidates) {
        int count = 0;
        for (int p : P) {
            if (m_adjMatrix[u][p]) count++;
        }
        if (count > bestCount) {
            bestCount = count;
            best = u;
        }
    }
    return best;
}

/* ---- Bron-Kerbosch with pivot ---- */

void MaximumClique7::bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X,
                                        QVector<QVector<int>>& results, int maxCount) const
{
    if (results.size() >= maxCount) return;

    if (P.isEmpty() && X.isEmpty()) {
        // R is a maximal clique
        results.append(R);
        return;
    }

    int pivot = selectPivot(P, X);

    // Candidates = P \ N(pivot)
    QVector<int> candidates;
    for (int v : P) {
        if (pivot < 0 || !m_adjMatrix[pivot][v])
            candidates.append(v);
    }

    for (int v : candidates) {
        if (results.size() >= maxCount) return;

        R.append(v);
        QVector<int> newP = intersectWithNeighbors(P, v);
        QVector<int> newX = intersectWithNeighbors(X, v);

        bronKerboschPivot(R, newP, newX, results, maxCount);

        R.removeLast();

        // Move v from P to X
        int idx = P.indexOf(v);
        if (idx >= 0) P.removeAt(idx);
        X.append(v);
    }
}

/* ---- Core decomposition for degeneracy ordering ---- */

QVector<int> MaximumClique7::coreDecomposition() const
{
    QVector<int> degree(m_n);
    for (int i = 0; i < m_n; ++i)
        degree[i] = m_adj[i].size();

    int maxDeg = 0;
    for (int d : degree)
        if (d > maxDeg) maxDeg = d;

    // Bucket sort by degree
    QVector<QVector<int>> buckets(maxDeg + 1);
    for (int i = 0; i < m_n; ++i)
        buckets[degree[i]].append(i);

    QVector<bool> removed(m_n, false);
    QVector<int> ordering;
    ordering.reserve(m_n);

    for (int d = 0; d <= maxDeg; ++d) {
        for (int idx = 0; idx < buckets[d].size(); ++idx) {
            int v = buckets[d][idx];
            if (removed[v]) continue;
            removed[v] = true;
            ordering.append(v);

            for (int u : m_adj[v]) {
                if (removed[u]) continue;
                int oldDeg = degree[u];
                degree[u]--;
                if (degree[u] < oldDeg && degree[u] >= 0)
                    buckets[degree[u]].append(u);
            }
        }
    }
    return ordering;
}

/* ---- Degeneracy ordering ---- */

QVector<int> MaximumClique7::degeneracyOrdering() const
{
    return coreDecomposition();
}

/* ---- Find maximum clique ---- */

QVector<int> MaximumClique7::findMaximumClique()
{
    QElapsedTimer timer;
    timer.start();

    m_maximalCliques.clear();
    m_maxClique.clear();

    if (m_n == 0) return {};

    // Use degeneracy ordering for initial vertex ordering
    QVector<int> order = degeneracyOrdering();

    QVector<int> R, P, X;
    for (int i = 0; i < m_n; ++i) P.append(i);

    bronKerboschPivot(R, P, X, m_maximalCliques, 10000);

    // Find the largest
    int maxSize = 0;
    for (const auto& cl : m_maximalCliques) {
        if (cl.size() > maxSize) {
            maxSize = cl.size();
            m_maxClique = cl;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.maxCliqueSize = m_maxClique.size();
    m_stats.totalMaximalCliques = m_maximalCliques.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cliqueFound(m_stats.maxCliqueSize, m_stats.totalMaximalCliques, elapsed);

    return m_maxClique;
}

/* ---- Enumerate all maximal cliques ---- */

QVector<QVector<int>> MaximumClique7::enumerateMaximalCliques(int maxCount)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> results;
    if (m_n == 0) return results;

    QVector<int> R, P, X;
    for (int i = 0; i < m_n; ++i) P.append(i);

    bronKerboschPivot(R, P, X, results, maxCount);

    double elapsed = timer.elapsed();
    m_stats.totalMaximalCliques = results.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return results;
}

/* ---- Reset ---- */

void MaximumClique7::resetStatistics()
{
    m_n = 0;
    m_adj.clear();
    m_adjMatrix.clear();
    m_maxClique.clear();
    m_maximalCliques.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
