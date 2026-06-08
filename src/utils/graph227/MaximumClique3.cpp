/**
 * @file MaximumClique3.cpp
 * @brief MaximumClique3 实现
 *
 * 实现最大团搜索：Tomita分支限界、贪心着色剪枝、团枚举。
 */

#include "utils/graph227/MaximumClique3.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MaximumClique3::MaximumClique3(QObject *parent) : QObject(parent) {}
MaximumClique3::~MaximumClique3() = default;

/* ---- Set graph from adjacency matrix ---- */

void MaximumClique3::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_n = adjacency.size();
    m_adj = adjacency;
    m_bestClique.clear();

    // Count edges
    m_stats.numEdges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (i < m_adj[i].size() && m_adj[i][j])
                m_stats.numEdges++;
    m_stats.numVertices = m_n;
}

/* ---- Set graph from edge list ---- */

void MaximumClique3::setGraphFromEdges(int numVertices,
                                        const QVector<QPair<int, int>>& edges)
{
    m_n = numVertices;
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_adj[i].fill(0, m_n);

    for (const auto& [u, v] : edges) {
        if (u >= 0 && u < m_n && v >= 0 && v < m_n) {
            m_adj[u][v] = 1;
            m_adj[v][u] = 1;
        }
    }
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();
    m_bestClique.clear();
}

/* ---- Check adjacency to all clique members ---- */

bool MaximumClique3::isAdjacentToAll(int v, const QVector<int>& clique) const
{
    for (int u : clique) {
        if (v >= m_adj.size() || u >= m_adj[v].size() || !m_adj[v][u])
            return false;
    }
    return true;
}

/* ---- Greedy coloring for upper bound ---- */

QVector<int> MaximumClique3::greedyColoring(const QVector<int>& vertices) const
{
    int n = vertices.size();
    QVector<int> color(n, 0);
    QVector<bool> used(n + 1, false);

    for (int i = 0; i < n; ++i) {
        // Mark colors used by neighbors
        for (int c = 1; c <= n; ++c) used[c] = false;

        for (int j = 0; j < i; ++j) {
            int vi = vertices[i];
            int vj = vertices[j];
            if (vi < m_adj.size() && vj < m_adj[vi].size() && m_adj[vi][vj])
                used[color[j]] = true;
        }

        // Assign smallest available color
        for (int c = 1; c <= n; ++c) {
            if (!used[c]) { color[i] = c; break; }
        }
    }
    return color;
}

/* ---- Tomita expand (branch-and-bound) ---- */

void MaximumClique3::expand(QVector<int>& current, QVector<int>& candidates,
                             int& bestSize, int& nodesExplored)
{
    if (candidates.isEmpty()) {
        if (current.size() > bestSize) {
            bestSize = current.size();
            m_bestClique = current;
        }
        return;
    }

    // Color-based upper bound pruning
    auto colors = greedyColoring(candidates);
    int maxColor = 0;
    for (int c : colors) maxColor = qMax(maxColor, c);

    if (static_cast<int>(current.size()) + maxColor <= bestSize)
        return;  // Prune: no larger clique possible

    // Process candidates in reverse color order (highest color first for better pruning)
    QVector<QPair<int, int>> order;
    order.reserve(candidates.size());
    for (int i = 0; i < candidates.size(); ++i)
        order.append({colors[i], i});
    std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    for (const auto& [color, idx] : order) {
        int v = candidates[idx];

        // Prune if even best color can't improve
        if (static_cast<int>(current.size()) + color <= bestSize)
            return;

        current.append(v);
        nodesExplored++;

        // Build new candidates: neighbors of v that come after it
        QVector<int> newCands;
        for (int j = idx + 1; j < candidates.size(); ++j) {
            int u = candidates[j];
            if (v < m_adj.size() && u < m_adj[v].size() && m_adj[v][u])
                newCands.append(u);
        }

        expand(current, newCands, bestSize, nodesExplored);
        current.removeLast();
    }
}

/* ---- Find maximum clique ---- */

QVector<int> MaximumClique3::findMaximumClique()
{
    QElapsedTimer timer;
    timer.start();

    m_bestClique.clear();
    if (m_n == 0) return {};

    QVector<int> current;
    QVector<int> candidates;
    candidates.reserve(m_n);
    for (int i = 0; i < m_n; ++i) candidates.append(i);

    int bestSize = 0;
    int nodesExplored = 0;

    expand(current, candidates, bestSize, nodesExplored);

    m_stats.totalSearches++;
    m_stats.maxCliqueSize = m_bestClique.size();
    m_stats.nodesExplored = nodesExplored;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
    emit searchCompleted(m_bestClique.size(), nodesExplored, timer.elapsed());

    return m_bestClique;
}

/* ---- Find all maximal cliques ---- */

QVector<QVector<int>> MaximumClique3::findAllMaximalCliques(int maxSizeLimit)
{
    QVector<QVector<int>> result;
    if (m_n == 0) return result;

    // Simplified Bron-Kerbosch with pivot
    // Use adjacency for pivot selection
    QVector<int> R, P, X;
    P.reserve(m_n);
    for (int i = 0; i < m_n; ++i) P.append(i);

    // Iterative approach for all cliques (limited by maxSizeLimit)
    int limit = (maxSizeLimit > 0) ? maxSizeLimit : m_n;
    QVector<QPair<QVector<int>, QVector<int>>> stack;
    stack.append({{}, P});

    while (!stack.isEmpty()) {
        auto [curR, curP] = stack.takeLast();
        if (curP.isEmpty()) {
            if (!curR.isEmpty() && curR.size() <= limit)
                result.append(curR);
            continue;
        }
        if (static_cast<int>(curR.size()) >= limit) {
            result.append(curR);
            continue;
        }

        int v = curP.last();
        // Branch: include v
        QVector<int> newR = curR;
        newR.append(v);
        QVector<int> newP;
        for (int u : curP) {
            if (u != v && v < m_adj.size() && u < m_adj[v].size() && m_adj[v][u])
                newP.append(u);
        }
        if (newR.size() + newP.size() >= static_cast<size_t>(limit > 0 ? limit : 1))
            stack.append({newR, newP});

        // Branch: exclude v
        QVector<int> exclP;
        for (int u : curP)
            if (u != v) exclP.append(u);
        stack.append({curR, exclP});
    }

    return result;
}

/* ---- Reset ---- */

void MaximumClique3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bestClique.clear();
}
