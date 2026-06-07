/**
 * @file MaximumClique2.cpp
 * @brief MaximumClique2 实现
 *
 * 实现最大团搜索：Tomita着色上界剪枝、递归分支限界、贪心着色排序。
 */

#include "utils/graph202/MaximumClique2.h"

#include <QElapsedTimer>
#include <algorithm>
#include <functional>

/* ---- Construction / Destruction ---- */

MaximumClique2::MaximumClique2(QObject *parent) : QObject(parent) {}
MaximumClique2::~MaximumClique2() = default;

/* ---- Configuration ---- */

void MaximumClique2::setAdjacencyMatrix(const QVector<QVector<bool>>& adj)
{
    m_adjMatrix = adj;
    m_n = adj.size();
    buildAdjList();
}

void MaximumClique2::setAdjacencyList(const QVector<QVector<int>>& adjList)
{
    m_adjList = adjList;
    m_n = adjList.size();

    // Build adjacency matrix from list
    m_adjMatrix.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_adjMatrix[i].resize(m_n, false);
        for (int v : adjList[i])
            if (v >= 0 && v < m_n) m_adjMatrix[i][v] = true;
    }
}

/* ---- Build adjacency list from matrix ---- */

void MaximumClique2::buildAdjList()
{
    m_adjList.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_adjList[i].clear();
        for (int j = 0; j < m_n; ++j) {
            if (m_adjMatrix[i][j]) m_adjList[i].append(j);
        }
    }
}

/* ---- Check if vertex is adjacent to all clique members ---- */

bool MaximumClique2::isCliqueMember(int v, const QVector<int>& clique) const
{
    for (int u : clique) {
        if (v != u && !m_adjMatrix[v][u]) return false;
    }
    return true;
}

/* ---- Greedy coloring (returns color number for each vertex) ---- */

QVector<int> MaximumClique2::greedyColoring(const QVector<int>& vertices) const
{
    int nv = vertices.size();
    QVector<int> color(nv, -1);

    for (int i = 0; i < nv; ++i) {
        int v = vertices[i];
        // Find colors used by neighbors among already-colored vertices
        QVector<bool> used(nv + 1, false);
        for (int j = 0; j < i; ++j) {
            int u = vertices[j];
            if (m_adjMatrix[v][u] && color[j] >= 0)
                used[color[j]] = true;
        }
        // Assign smallest available color
        int c = 0;
        while (used[c]) ++c;
        color[i] = c;
    }
    return color;
}

/* ---- Tomita-style coloring with sorting ---- */

QVector<int> MaximumClique2::colorSort(QVector<int>& candidates) const
{
    int nv = candidates.size();
    QVector<int> colorBounds(nv);
    QVector<int> sorted = candidates;
    QVector<int> colors(nv, 0);

    // Greedy coloring with ordering
    for (int i = 0; i < nv; ++i) {
        QVector<bool> used(nv + 1, false);
        for (int j = 0; j < i; ++j) {
            if (m_adjMatrix[sorted[i]][sorted[j]])
                used[colors[j]] = true;
        }
        int c = 0;
        while (used[c]) ++c;
        colors[i] = c;
    }

    // Sort candidates by decreasing color number
    QVector<QPair<int, int>> cv;
    for (int i = 0; i < nv; ++i)
        cv.append({sorted[i], colors[i]});
    std::sort(cv.begin(), cv.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (int i = 0; i < nv; ++i) {
        candidates[i] = cv[i].first;
        colorBounds[i] = cv[i].second;
    }
    return colorBounds;
}

/* ---- Recursive branch-and-bound ---- */

void MaximumClique2::expand(QVector<int>& current, QVector<int>& candidates)
{
    if (candidates.isEmpty()) {
        if (current.size() > m_bestClique.size())
            m_bestClique = current;
        return;
    }

    // Color-based upper bound pruning
    QVector<int> colorBounds = colorSort(candidates);

    for (int i = 0; i < candidates.size(); ++i) {
        // Tomita pruning: if |current| + colorBound <= best, skip rest
        if (static_cast<int>(current.size()) + colorBounds[i]
            <= static_cast<int>(m_bestClique.size()))
            break;

        int v = candidates[i];

        // Build new candidate set: neighbors of v that appear after v
        QVector<int> newCandidates;
        for (int j = i + 1; j < candidates.size(); ++j) {
            if (m_adjMatrix[v][candidates[j]])
                newCandidates.append(candidates[j]);
        }

        current.append(v);
        m_nodesExplored++;

        expand(current, newCandidates);
        current.removeLast();
    }
}

/* ---- Main search ---- */

QVector<int> MaximumClique2::findMaximumClique()
{
    QElapsedTimer timer;
    timer.start();

    m_bestClique.clear();
    m_nodesExplored = 0;

    if (m_n == 0) return {};

    // Initial candidates: all vertices
    QVector<int> candidates(m_n);
    for (int i = 0; i < m_n; ++i) candidates[i] = i;

    QVector<int> current;
    expand(current, candidates);

    m_stats.totalSearches++;
    m_stats.numVertices = m_n;
    m_stats.maxCliqueSize = m_bestClique.size();
    m_stats.nodesExplored = m_nodesExplored;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(m_bestClique.size(), m_nodesExplored, timer.elapsed());
    return m_bestClique;
}

/* ---- Reset ---- */

void MaximumClique2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bestClique.clear();
    m_adjMatrix.clear();
    m_adjList.clear();
    m_n = 0;
}
