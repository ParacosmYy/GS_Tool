/**
 * @file IndependentSet3.cpp
 * @brief IndependentSet3 实现
 *
 * 实现最大独立集求解：分支限界、度排序、贪心上界。
 */

#include "utils/graph189/IndependentSet3.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

IndependentSet3::IndependentSet3(QObject *parent)
    : QObject(parent)
{
}

IndependentSet3::~IndependentSet3() = default;

/* ---- Configuration ---- */

void IndependentSet3::setOrdering(Ordering order) { m_ordering = order; }
void IndependentSet3::setTimeLimitMs(int ms) { m_timeLimitMs = qMax(100, ms); }

/* ---- Order vertices by degree ---- */

QVector<int> IndependentSet3::orderByDegree(
    const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<QPair<int, int>> degreeIdx(n);
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j)
            deg += adj[i][j];
        degreeIdx[i] = {deg, i};
    }

    switch (m_ordering) {
    case DegreeAscending:
        std::sort(degreeIdx.begin(), degreeIdx.end());
        break;
    case DegreeDescending:
        std::sort(degreeIdx.begin(), degreeIdx.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        break;
    default:
        break;
    }

    QVector<int> ordering(n);
    for (int i = 0; i < n; ++i)
        ordering[i] = degreeIdx[i].second;
    return ordering;
}

/* ---- Upper bound via greedy coloring ---- */

int IndependentSet3::upperBound(const QVector<QVector<int>>& adj,
                                 const QVector<int>& ordering,
                                 const QVector<bool>& excluded,
                                 int startIdx) const
{
    /* Count remaining candidate vertices */
    int remaining = 0;
    for (int i = startIdx; i < ordering.size(); ++i)
        if (!excluded[ordering[i]]) ++remaining;
    return remaining; /* Trivial upper bound */
}

/* ---- Branch and bound search ---- */

void IndependentSet3::search(const QVector<QVector<int>>& adj,
                              const QVector<int>& ordering,
                              QVector<int>& current,
                              QVector<bool>& excluded,
                              int idx)
{
    int n = ordering.size();
    m_nodesExplored++;

    /* Skip excluded vertices */
    while (idx < n && excluded[ordering[idx]])
        ++idx;

    /* Base case: no more candidates */
    if (idx >= n) {
        if (current.size() > static_cast<int>(m_bestSize)) {
            m_bestSize = current.size();
            m_bestSet = current;
        }
        return;
    }

    /* Upper bound pruning */
    int ub = static_cast<int>(current.size()) +
        upperBound(adj, ordering, excluded, idx);
    if (ub <= m_bestSize) {
        m_pruningCount++;
        return;
    }

    /* Branch 1: Include ordering[idx] */
    int v = ordering[idx];
    current.append(v);
    /* Mark v and its neighbors as excluded */
    QVector<int> newlyExcluded;
    for (int j = 0; j < n; ++j) {
        if (adj[v][ordering[j]] && !excluded[ordering[j]]) {
            excluded[ordering[j]] = true;
            newlyExcluded.append(ordering[j]);
        }
    }
    excluded[v] = true;

    search(adj, ordering, current, excluded, idx + 1);

    /* Restore */
    current.removeLast();
    excluded[v] = false;
    for (int j : newlyExcluded)
        excluded[j] = false;

    /* Branch 2: Exclude ordering[idx] */
    excluded[v] = true;
    search(adj, ordering, current, excluded, idx + 1);
    excluded[v] = false;
}

/* ---- Main solve ---- */

QVector<int> IndependentSet3::solve(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    if (n == 0) return {};

    m_bestSet.clear();
    m_bestSize = 0;
    m_nodesExplored = 0;
    m_pruningCount = 0;

    QVector<int> ordering = orderByDegree(adjacency);
    QVector<int> current;
    QVector<bool> excluded(n, false);

    search(adjacency, ordering, current, excluded, 0);

    m_stats.totalSolves++;
    m_stats.bestSize = m_bestSize;
    m_stats.nodesExplored = m_nodesExplored;
    m_stats.pruningCount = m_pruningCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_bestSize, m_nodesExplored);
    return m_bestSet;
}

/* ---- Solve from edge list ---- */

QVector<int> IndependentSet3::solveFromEdges(
    const QVector<QPair<int, int>>& edges,
    int numVertices)
{
    QVector<QVector<int>> adj(numVertices, QVector<int>(numVertices, 0));
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < numVertices &&
            e.second >= 0 && e.second < numVertices) {
            adj[e.first][e.second] = 1;
            adj[e.second][e.first] = 1;
        }
    }
    return solve(adj);
}

/* ---- Max size only ---- */

int IndependentSet3::maxSize(const QVector<QVector<int>>& adjacency)
{
    auto result = solve(adjacency);
    return result.size();
}

/* ---- Validate ---- */

bool IndependentSet3::validateIndependentSet(
    const QVector<QVector<int>>& adjacency,
    const QVector<int>& set) const
{
    int n = adjacency.size();
    for (int i = 0; i < set.size(); ++i) {
        for (int j = i + 1; j < set.size(); ++j) {
            int u = set[i], v = set[j];
            if (u < 0 || u >= n || v < 0 || v >= n) return false;
            if (adjacency[u][v] != 0) return false;
        }
    }
    return true;
}

/* ---- Statistics ---- */

void IndependentSet3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
