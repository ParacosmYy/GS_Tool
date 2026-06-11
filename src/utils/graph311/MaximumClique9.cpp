/**
 * @file MaximumClique9.cpp
 * @brief MaximumClique9 实现
 *
 * 实现最大团：分支定界顶点着色与度数剪枝实现精确最大团枚举。
 */

#include "utils/graph311/MaximumClique9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MaximumClique9::MaximumClique9(QObject *parent)
    : QObject(parent) {}

MaximumClique9::~MaximumClique9() = default;

/* ---- Greedy vertex coloring for upper bound ---- */

QVector<int> MaximumClique9::greedyColor(
    const QVector<QVector<int>>& adj,
    const QVector<int>& candidates) const
{
    int n = candidates.size();
    if (n == 0) return {};

    // Map vertex -> index in candidates for fast adjacency lookup
    QVector<int> color(n, 0);
    QVector<bool> used(n + 1, false);

    for (int i = 0; i < n; ++i) {
        // Reset used colors
        used.fill(false);

        // Mark colors of already-colored neighbors
        for (int j = 0; j < i; ++j) {
            if (adj[candidates[i]][candidates[j]]) {
                if (color[j] <= n)
                    used[color[j]] = true;
            }
        }

        // Assign smallest available color
        int c = 1;
        while (c <= n && used[c]) c++;
        color[i] = c;
    }
    return color;
}

/* ---- Recursive branch-and-bound with coloring ---- */

void MaximumClique9::expand(const QVector<QVector<int>>& adj,
                             QVector<int>& current,
                             QVector<int>& candidates)
{
    if (candidates.isEmpty()) {
        // Update best clique
        if (current.size() > m_bestSize) {
            m_bestClique = current;
            m_bestSize = current.size();
        }
        return;
    }

    // Color candidates for pruning
    m_colorCalls++;
    auto colors = greedyColor(adj, candidates);

    // Process candidates in reverse order (highest color first for better pruning)
    int n = candidates.size();
    for (int i = n - 1; i >= 0; --i) {
        m_nodesExplored++;

        // Pruning: if current size + color upper bound <= best, skip
        if (static_cast<int>(current.size()) + colors[i] <= m_bestSize)
            return;

        // Select candidate vertex
        int v = candidates[i];
        current.append(v);

        // Build new candidate set: neighbors of v in remaining candidates
        QVector<int> newCandidates;
        for (int j = 0; j < i; ++j) {
            if (adj[v][candidates[j]])
                newCandidates.append(candidates[j]);
        }

        // Recurse
        expand(adj, current, newCandidates);

        // Backtrack
        current.pop_back();
    }
}

/* ---- Degree-based ordering (descending) ---- */

QVector<int> MaximumClique9::degreeOrder(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<QPair<int, int>> degreeIdx;
    degreeIdx.reserve(n);
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j)
            if (adj[i][j]) deg++;
        degreeIdx.append({deg, i});
    }
    // Sort descending by degree
    std::sort(degreeIdx.begin(), degreeIdx.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    QVector<int> order;
    order.reserve(n);
    for (const auto& p : degreeIdx)
        order.append(p.second);
    return order;
}

/* ---- Find maximum clique from adjacency matrix ---- */

MaximumClique9::CliqueResult MaximumClique9::findMaximumClique(
    const QVector<QVector<int>>& adjMatrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjMatrix.size();
    CliqueResult result;

    m_bestClique.clear();
    m_bestSize = 0;
    m_nodesExplored = 0;
    m_colorCalls = 0;

    // Build adjacency list from matrix
    QVector<QVector<int>> adjLists(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (adjMatrix[i][j])
                adjLists[i].append(j);
        }
    }

    // Degree-based ordering for initial candidates
    auto ordered = degreeOrder(adjMatrix);

    QVector<int> current;
    expand(adjMatrix, current, ordered);

    result.maxClique = m_bestClique;
    result.maxCliqueSize = m_bestSize;
    result.nodesExplored = m_nodesExplored;
    result.colorCalls = m_colorCalls;
    result.searchTimeMs = timer.elapsed();

    m_stats.totalSearches++;
    m_stats.lastGraphSize = n;
    m_stats.largestCliqueFound = qMax(m_stats.largestCliqueFound, m_bestSize);
    m_timeSum += result.searchTimeMs;
    m_stats.avgSearchTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchDone(n, m_bestSize, result.searchTimeMs);
    return result;
}

/* ---- Find maximum clique from adjacency lists ---- */

MaximumClique9::CliqueResult MaximumClique9::findMaximumCliqueList(
    const QVector<QVector<int>>& adjLists)
{
    int n = adjLists.size();

    // Convert to adjacency matrix
    QVector<QVector<int>> adjMatrix(n, QVector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j : adjLists[i]) {
            if (j >= 0 && j < n) {
                adjMatrix[i][j] = 1;
                adjMatrix[j][i] = 1;
            }
        }
    }
    return findMaximumClique(adjMatrix);
}

/* ---- Reset ---- */

void MaximumClique9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
