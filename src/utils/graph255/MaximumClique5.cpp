/**
 * @file MaximumClique5.cpp
 * @brief MaximumClique5 实现
 *
 * 实现最大团：Bron-Kerbosch枢轴选择与退化排序有界搜索树。
 */

#include "utils/graph255/MaximumClique5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MaximumClique5::MaximumClique5(QObject *parent) : QObject(parent) {}
MaximumClique5::~MaximumClique5() = default;

/* ---- Configuration ---- */

void MaximumClique5::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_n = adjacency.size();
    m_adj.resize(m_n);
    int edgeCount = 0;
    for (int i = 0; i < m_n; ++i) {
        m_adj[i].clear();
        for (int j = 0; j < m_n; ++j) {
            if (adjacency[i][j] && i != j) {
                m_adj[i].append(j);
                edgeCount++;
            }
        }
    }
    m_stats.numVertices = m_n;
    m_stats.numEdges = edgeCount / 2;
}

void MaximumClique5::setEdgeList(int numVertices, const QVector<QPair<int,int>>& edges)
{
    m_n = numVertices;
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_adj[i].clear();
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_n && e.second >= 0 && e.second < m_n) {
            if (!m_adj[e.first].contains(e.second)) m_adj[e.first].append(e.second);
            if (!m_adj[e.second].contains(e.first)) m_adj[e.second].append(e.first);
        }
    }
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();
}

/* ---- Degeneracy ordering ---- */

QVector<int> MaximumClique5::degeneracyOrdering() const
{
    int n = m_n;
    QVector<int> degree(n, 0);
    for (int i = 0; i < n; ++i)
        degree[i] = m_adj[i].size();

    QVector<int> order;
    QVector<bool> used(n, false);

    for (int iter = 0; iter < n; ++iter) {
        int minDeg = n + 1, minV = -1;
        for (int i = 0; i < n; ++i) {
            if (!used[i] && degree[i] < minDeg) {
                minDeg = degree[i];
                minV = i;
            }
        }
        if (minV < 0) break;
        order.append(minV);
        used[minV] = true;
        for (int nb : m_adj[minV])
            if (!used[nb]) degree[nb]--;
    }
    return order;
}

/* ---- Intersect set with neighbors ---- */

QVector<int> MaximumClique5::intersectNeighbors(const QVector<int>& S, int v) const
{
    QVector<int> result;
    for (int u : S) {
        if (m_adj[v].contains(u))
            result.append(u);
    }
    return result;
}

/* ---- Select pivot maximizing |P ∩ N(u)| ---- */

int MaximumClique5::selectPivot(const QVector<int>& P, const QVector<int>& X) const
{
    int bestPivot = -1;
    int bestCount = -1;
    QVector<int> candidates = P;
    candidates.append(X);

    for (int u : candidates) {
        int count = 0;
        for (int p : P) {
            if (m_adj[u].contains(p)) count++;
        }
        if (count > bestCount) {
            bestCount = count;
            bestPivot = u;
        }
    }
    return bestPivot;
}

/* ---- Bron-Kerbosch with pivot (Tomita) ---- */

void MaximumClique5::bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X)
{
    if (P.isEmpty() && X.isEmpty()) {
        if (R.size() > m_bestSize) {
            m_bestSize = R.size();
            m_bestClique = R;
        }
        return;
    }

    // Pruning: even if all of P joins R, can't beat best
    if (R.size() + P.size() <= m_bestSize) return;

    // Pivot selection
    int pivot = selectPivot(P, X);
    QVector<int> candidates;
    for (int v : P) {
        if (!m_adj[pivot].contains(v))
            candidates.append(v);
    }

    for (int v : candidates) {
        R.append(v);
        QVector<int> newP = intersectNeighbors(P, v);
        QVector<int> newX = intersectNeighbors(X, v);
        bronKerboschPivot(R, newP, newX);
        R.removeLast();
        P.removeOne(v);
        X.append(v);
    }
}

/* ---- Solve ---- */

QVector<int> MaximumClique5::solve()
{
    QElapsedTimer timer;
    timer.start();

    m_bestClique.clear();
    m_bestSize = 0;

    // Use degeneracy ordering for bounded search tree
    QVector<int> order = degeneracyOrdering();

    // Process vertices in degeneracy order: each vertex's later neighbors form P
    for (int i = 0; i < order.size(); ++i) {
        int v = order[i];
        QVector<int> R = {v};
        QVector<int> P, X;

        // P = neighbors of v that come later in degeneracy order
        for (int j = i + 1; j < order.size(); ++j) {
            if (m_adj[v].contains(order[j]))
                P.append(order[j]);
        }
        // X = neighbors of v that come earlier
        for (int j = 0; j < i; ++j) {
            if (m_adj[v].contains(order[j]))
                X.append(order[j]);
        }

        bronKerboschPivot(R, P, X);
    }

    m_stats.maxCliqueSize = m_bestSize;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit searchCompleted(m_bestSize, timer.elapsed());
    return m_bestClique;
}

/* ---- Max clique size ---- */

int MaximumClique5::maxCliqueSize() const { return m_bestSize; }

/* ---- Reset ---- */

void MaximumClique5::resetStatistics()
{
    m_adj.clear(); m_bestClique.clear();
    m_bestSize = 0; m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
