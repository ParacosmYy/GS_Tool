/**
 * @file MaximumClique.cpp
 * @brief MaximumClique 实现
 *
 * 实现Bron-Kerbosch最大团算法：枢轴剪枝、退化序预处理。
 */

#include "utils/graph188/MaximumClique.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MaximumClique::MaximumClique(QObject *parent)
    : QObject(parent)
{
}

MaximumClique::~MaximumClique() = default;

/* ---- Build from adjacency matrix ---- */

void MaximumClique::buildFromMatrix(const QVector<QVector<int>>& adj)
{
    m_n = adj.size();
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_adj[i].clear();
        for (int j = 0; j < m_n; ++j) {
            if (i != j && adj[i][j] != 0)
                m_adj[i].append(j);
        }
    }
}

/* ---- Build from edge list ---- */

void MaximumClique::buildFromEdges(const QVector<QPair<int, int>>& edges, int n)
{
    m_n = n;
    m_adj.resize(n);
    for (int i = 0; i < n; ++i) m_adj[i].clear();
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
        }
    }
}

/* ---- Compute degrees ---- */

QVector<int> MaximumClique::computeDegrees() const
{
    QVector<int> deg(m_n, 0);
    for (int i = 0; i < m_n; ++i)
        deg[i] = m_adj[i].size();
    return deg;
}

/* ---- Degeneracy ordering ---- */

QVector<int> MaximumClique::degeneracyOrdering() const
{
    int maxDeg = 0;
    QVector<int> deg = computeDegrees();
    for (int d : deg) maxDeg = qMax(maxDeg, d);

    /* Bucket sort by degree */
    QVector<QVector<int>> buckets(maxDeg + 1);
    for (int i = 0; i < m_n; ++i)
        buckets[deg[i]].append(i);

    QVector<int> ordering;
    ordering.reserve(m_n);
    QVector<bool> removed(m_n, false);

    for (int d = 0; d <= maxDeg; ++d) {
        for (int idx = 0; idx < buckets[d].size(); ++idx) {
            int v = buckets[d][idx];
            if (removed[v]) continue;
            ordering.append(v);
            removed[v] = true;
            /* Update neighbors */
            for (int u : m_adj[v]) {
                if (!removed[u] && deg[u] > d) {
                    deg[u]--;
                    buckets[deg[u]].append(u);
                }
            }
        }
    }
    return ordering;
}

/* ---- Greedy coloring for upper bound ---- */

int MaximumClique::greedyColoring(const QVector<int>& vertices) const
{
    int n = vertices.size();
    if (n == 0) return 0;

    QVector<int> color(m_n, -1);
    int maxColor = 0;

    for (int i = n - 1; i >= 0; --i) {
        int v = vertices[i];
        QVector<bool> used(n + 1, false);
        for (int u : m_adj[v]) {
            if (color[u] >= 0 && color[u] <= n)
                used[color[u]] = true;
        }
        int c = 0;
        while (c <= n && used[c]) c++;
        color[v] = c;
        maxColor = qMax(maxColor, c);
    }
    return maxColor + 1;
}

/* ---- Bron-Kerbosch with pivot (Tomita) ---- */

void MaximumClique::bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X)
{
    if (P.isEmpty() && X.isEmpty()) {
        /* Found a maximal clique */
        if (R.size() > m_bestClique.size())
            m_bestClique = R;
        return;
    }

    /* Color-based upper bound pruning */
    int bound = R.size() + greedyColoring(P);
    if (bound <= m_bestClique.size()) return;

    /* Choose pivot: maximize |P ∩ N(u)| (Tomita heuristic) */
    int pivot = -1;
    int maxNeighbors = -1;
    QVector<int> PX;
    PX.reserve(P.size() + X.size());
    PX.append(P);
    PX.append(X);

    for (int u : PX) {
        int count = 0;
        for (int p : P) {
            for (int v : m_adj[u]) {
                if (v == p) { count++; break; }
            }
        }
        if (count > maxNeighbors) { maxNeighbors = count; pivot = u; }
    }

    /* Candidates = P \ N(pivot) */
    QVector<int> candidates;
    for (int p : P) {
        bool inNeighbor = false;
        if (pivot >= 0) {
            for (int v : m_adj[pivot]) {
                if (v == p) { inNeighbor = true; break; }
            }
        }
        if (!inNeighbor) candidates.append(p);
    }

    for (int v : candidates) {
        m_nodesExplored++;
        if (m_nodesExplored > 10000000) return; /* Safety cutoff */

        R.append(v);

        /* P' = P ∩ N(v), X' = X ∩ N(v) */
        QVector<int> newP, newX;
        for (int p : P) {
            for (int u : m_adj[v]) {
                if (u == p) { newP.append(p); break; }
            }
        }
        for (int x : X) {
            for (int u : m_adj[v]) {
                if (u == x) { newX.append(x); break; }
            }
        }

        bronKerboschPivot(R, newP, newX);
        R.removeLast();

        /* Move v from P to X */
        P.removeOne(v);
        X.append(v);
    }
}

/* ---- Find maximum clique ---- */

QVector<int> MaximumClique::findMaximumClique()
{
    QElapsedTimer timer;
    timer.start();

    m_bestClique.clear();
    m_nodesExplored = 0;

    QVector<int> R, P, X;
    P.reserve(m_n);
    for (int i = 0; i < m_n; ++i) P.append(i);

    bronKerboschPivot(R, P, X);

    m_stats.totalSearches++;
    m_stats.lastCliqueSize = m_bestClique.size();
    m_stats.maxCliqueSize = qMax(m_stats.maxCliqueSize, m_stats.lastCliqueSize);
    m_stats.nodesExplored = m_nodesExplored;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(m_stats.lastCliqueSize, m_nodesExplored);
    return m_bestClique;
}

/* ---- Enumerate all maximal cliques ---- */

void MaximumClique::bronKerboschEnumerate(QVector<int>& R, QVector<int>& P,
                                           QVector<int>& X,
                                           QVector<QVector<int>>& results)
{
    if (P.isEmpty() && X.isEmpty()) {
        results.append(R);
        return;
    }

    if (results.size() > 100000) return; /* Safety cutoff */

    int pivot = P.isEmpty() ? -1 : P[0];
    QVector<int> candidates;
    for (int p : P) {
        bool inNeighbor = false;
        if (pivot >= 0) {
            for (int v : m_adj[pivot]) {
                if (v == p) { inNeighbor = true; break; }
            }
        }
        if (!inNeighbor) candidates.append(p);
    }

    for (int v : candidates) {
        R.append(v);
        QVector<int> newP, newX;
        for (int p : P) {
            for (int u : m_adj[v])
                if (u == p) { newP.append(p); break; }
        }
        for (int x : X) {
            for (int u : m_adj[v])
                if (u == x) { newX.append(x); break; }
        }
        bronKerboschEnumerate(R, newP, newX, results);
        R.removeLast();
        P.removeOne(v);
        X.append(v);
    }
}

QVector<QVector<int>> MaximumClique::enumerateAllMaximal()
{
    QVector<QVector<int>> results;
    QVector<int> R, P, X;
    for (int i = 0; i < m_n; ++i) P.append(i);
    bronKerboschEnumerate(R, P, X, results);
    return results;
}

/* ---- Max clique size only ---- */

int MaximumClique::maxCliqueSize()
{
    auto clique = findMaximumClique();
    return clique.size();
}

int MaximumClique::nodeCount() const { return m_n; }

/* ---- Statistics ---- */

void MaximumClique::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
