/**
 * @file MaximumClique6.cpp
 * @brief MaximumClique6 实现
 *
 * 实现最大团：MaxSAT上界与渐进边界紧缩搜索空间。
 */

#include "utils/graph269/MaximumClique6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MaximumClique6::MaximumClique6(QObject *parent)
    : QObject(parent) {}
MaximumClique6::~MaximumClique6() = default;

/* ---- Set graph from adjacency matrix ---- */

void MaximumClique6::setGraph(const QVector<QVector<int>>& adjMatrix)
{
    m_n = adjMatrix.size();
    m_adjMat = adjMatrix;
    m_adj.resize(m_n);

    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            if (i < adjMatrix.size() && j < adjMatrix[i].size()
                && adjMatrix[i][j] != 0) {
                m_adj[i].append(j);
                m_adj[j].append(i);
            }
        }
    }

    m_stats.numVertices = m_n;
    m_stats.numEdges = countEdges();
}

/* ---- Set graph from adjacency list ---- */

void MaximumClique6::setGraphList(const QVector<QVector<int>>& adjList,
                                   int numVertices)
{
    m_n = numVertices;
    m_adj = adjList;
    m_adjMat.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_adjMat[i].resize(m_n, 0);

    for (int i = 0; i < m_n; ++i) {
        for (int j : adjList[i]) {
            if (j >= 0 && j < m_n) {
                m_adjMat[i][j] = 1;
            }
        }
    }

    m_stats.numVertices = m_n;
    m_stats.numEdges = countEdges();
}

/* ---- Count edges ---- */

int MaximumClique6::countEdges() const
{
    int count = 0;
    for (int i = 0; i < m_n; ++i)
        count += m_adj[i].size();
    return count / 2;
}

/* ---- Check clique membership ---- */

bool MaximumClique6::isCliqueVertex(int v, const QVector<int>& clique) const
{
    for (int u : clique) {
        if (m_adjMat[v][u] == 0) return false;
    }
    return true;
}

/* ---- Greedy coloring for upper bound ---- */

int MaximumClique6::greedyColor(const QVector<int>& vertices,
                                 QVector<int>& colors) const
{
    int n = vertices.size();
    colors.resize(n);
    colors.fill(-1);

    int maxColor = 0;
    for (int i = 0; i < n; ++i) {
        // Find colors used by already-colored neighbors
        QVector<bool> usedColor(n + 1, false);
        for (int j = 0; j < i; ++j) {
            if (m_adjMat[vertices[i]][vertices[j]] != 0) {
                if (colors[j] >= 0 && colors[j] <= n)
                    usedColor[colors[j]] = true;
            }
        }
        // Assign smallest available color
        int c = 0;
        while (c <= n && usedColor[c]) c++;
        colors[i] = c;
        maxColor = qMax(maxColor, c);
    }
    return maxColor + 1; // Chromatic number upper bound
}

/* ---- MaxSAT-based upper bound ---- */

int MaximumClique6::maxSatBound(const QVector<int>& candidates) const
{
    if (candidates.isEmpty()) return 0;

    // Use greedy coloring as MaxSAT relaxation upper bound
    QVector<int> colors;
    int chromaticUB = greedyColor(candidates, colors);

    // Tighten: also consider independent set cover
    // The max clique size <= chromatic number
    return chromaticUB;
}

/* ---- Branch-and-bound search ---- */

void MaximumClique6::branchBound(const QVector<int>& currentClique,
                                  QVector<int>& candidates)
{
    if (candidates.isEmpty()) {
        // Leaf: check if current clique is best
        if (currentClique.size() > m_bestSize) {
            m_bestSize = currentClique.size();
            m_bestClique = currentClique;
        }
        return;
    }

    // Progressive bounding: compute MaxSAT upper bound
    int ub = currentClique.size() + maxSatBound(candidates);
    if (ub <= m_bestSize) {
        // Prune: even optimal extension can't beat current best
        m_stats.numBoundPrunes++;
        return;
    }

    // Sort candidates by degree (descending) for better pruning
    std::sort(candidates.begin(), candidates.end(),
              [this](int a, int b) {
                  return m_adj[a].size() > m_adj[b].size();
              });

    // Branch on each candidate
    for (int i = 0; i < candidates.size(); ++i) {
        int v = candidates[i];

        // Check: v must connect to all current clique members
        if (!isCliqueVertex(v, currentClique)) continue;

        // Progressive bound: remaining candidates minus pruned
        int remainingUB = static_cast<int>(currentClique.size())
                          + static_cast<int>(candidates.size()) - i;
        if (remainingUB <= m_bestSize) {
            m_stats.numBoundPrunes++;
            break;
        }

        // Build new candidate set: neighbors of v among remaining
        QVector<int> newCandidates;
        for (int j = i + 1; j < candidates.size(); ++j) {
            int u = candidates[j];
            if (m_adjMat[v][u] != 0 && isCliqueVertex(u, currentClique)) {
                newCandidates.append(u);
            }
        }

        // Extend clique and recurse
        QVector<int> newClique = currentClique;
        newClique.append(v);
        branchBound(newClique, newCandidates);
    }
}

/* ---- Solve: find maximum clique ---- */

QVector<int> MaximumClique6::solve()
{
    QElapsedTimer timer;
    timer.start();

    m_bestClique.clear();
    m_bestSize = 0;
    m_stats.numBoundPrunes = 0;

    if (m_n == 0) return {};

    // Initial candidates: all vertices sorted by degree
    QVector<int> candidates;
    candidates.reserve(m_n);
    for (int i = 0; i < m_n; ++i)
        candidates.append(i);

    std::sort(candidates.begin(), candidates.end(),
              [this](int a, int b) {
                  return m_adj[a].size() > m_adj[b].size();
              });

    // Branch-and-bound from empty clique
    branchBound({}, candidates);

    m_stats.cliqueSize = m_bestSize;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_bestSize, m_stats.numBoundPrunes, elapsed);
    return m_bestClique;
}

/* ---- Clique size accessor ---- */

int MaximumClique6::cliqueSize() const { return m_bestSize; }

/* ---- Reset ---- */

void MaximumClique6::resetStatistics()
{
    m_adj.clear();
    m_adjMat.clear();
    m_bestClique.clear();
    m_n = 0;
    m_bestSize = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
