/**
 * @file DominatingSet8.cpp
 * @brief DominatingSet8 实现
 *
 * 实现支配集：贪心近似与闭邻域基数剪枝。
 */

#include "utils/graph248/DominatingSet8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DominatingSet8::DominatingSet8(QObject *parent) : QObject(parent) {}
DominatingSet8::~DominatingSet8() = default;

/* ---- Set graph ---- */

void DominatingSet8::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_numVertices = adjacency.size();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = 0;
    for (const auto& neighbors : m_adj)
        m_stats.numEdges += neighbors.size();
    m_stats.numEdges /= 2;  // Undirected
}

void DominatingSet8::setEdges(int numVertices, const QVector<QPair<int, int>>& edges)
{
    m_numVertices = numVertices;
    m_adj.resize(numVertices);
    for (int i = 0; i < numVertices; ++i) m_adj[i].clear();

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < numVertices &&
            e.second >= 0 && e.second < numVertices) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
        }
    }
    m_stats.numVertices = numVertices;
    m_stats.numEdges = edges.size();
}

/* ---- Closed neighborhood ---- */

QVector<int> DominatingSet8::closedNeighborhood(int vertex) const
{
    QVector<int> cn;
    if (vertex < 0 || vertex >= m_numVertices) return cn;
    cn = m_adj[vertex];
    cn.append(vertex);
    // Remove duplicates
    std::sort(cn.begin(), cn.end());
    cn.erase(std::unique(cn.begin(), cn.end()), cn.end());
    return cn;
}

int DominatingSet8::closedNeighborhoodSize(int vertex) const
{
    if (vertex < 0 || vertex >= m_numVertices) return 0;
    int size = 1;  // Include self
    size += m_adj[vertex].size();
    return size;
}

/* ---- Uncovered count ---- */

int DominatingSet8::uncoveredCount(int vertex, const QVector<bool>& covered) const
{
    int count = 0;
    if (vertex < 0 || vertex >= m_numVertices) return 0;
    if (!covered[vertex]) count++;
    for (int nb : m_adj[vertex]) {
        if (!covered[nb]) count++;
    }
    return count;
}

/* ---- Prune candidates ---- */

QVector<int> DominatingSet8::pruneCandidates(const QVector<int>& candidates,
                                               int remainingUndominated) const
{
    if (candidates.size() <= 1) return candidates;

    // Compute max closed neighborhood size among candidates
    int maxCN = 0;
    for (int v : candidates)
        maxCN = qMax(maxCN, closedNeighborhoodSize(v));

    // Prune: if a candidate's CN size < ceil(remaining / (numCandidates)),
    // and it's strictly less than maxCN, it can be pruned
    int threshold = qMax(1, static_cast<int>(qCeil(
        static_cast<double>(remainingUndominated) / candidates.size())));

    QVector<int> pruned;
    int numPruned = 0;
    for (int v : candidates) {
        int cnSize = closedNeighborhoodSize(v);
        if (cnSize >= threshold || cnSize == maxCN) {
            pruned.append(v);
        } else {
            numPruned++;
        }
    }
    const_cast<DominatingSet8*>(this)->m_stats.numPruned += numPruned;
    return pruned.isEmpty() ? candidates : pruned;
}

/* ---- Upper bound ---- */

int DominatingSet8::upperBound(int remaining) const
{
    if (remaining <= 0) return 0;
    int maxCN = 1;
    for (int v = 0; v < m_numVertices; ++v)
        maxCN = qMax(maxCN, closedNeighborhoodSize(v));
    return qCeil(static_cast<double>(remaining) / maxCN);
}

/* ---- Solve (greedy approximation) ---- */

QVector<int> DominatingSet8::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numVertices == 0) return {};

    QVector<bool> covered(m_numVertices, false);
    QVector<int> dominatingSet;
    int totalCovered = 0;

    while (totalCovered < m_numVertices) {
        // Build candidate list: uncovered vertices and their neighbors
        QVector<int> candidates;
        for (int v = 0; v < m_numVertices; ++v) {
            if (!covered[v] || uncoveredCount(v, covered) > 0)
                candidates.append(v);
        }

        int remaining = m_numVertices - totalCovered;
        candidates = pruneCandidates(candidates, remaining);

        // Greedy: pick vertex that covers most uncovered vertices
        int bestVertex = -1;
        int bestCover = -1;
        for (int v : candidates) {
            int cnt = uncoveredCount(v, covered);
            if (cnt > bestCover) {
                bestCover = cnt;
                bestVertex = v;
            }
        }

        if (bestVertex < 0) break;

        // Mark covered
        dominatingSet.append(bestVertex);
        if (!covered[bestVertex]) { covered[bestVertex] = true; totalCovered++; }
        for (int nb : m_adj[bestVertex]) {
            if (!covered[nb]) { covered[nb] = true; totalCovered++; }
        }

        emit vertexAdded(bestVertex, bestCover);
    }

    m_stats.dominatingSetSize = dominatingSet.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(dominatingSet.size(), timer.elapsed());
    return dominatingSet;
}

/* ---- Validation ---- */

bool DominatingSet8::isValidDominatingSet(const QVector<int>& candidateSet) const
{
    QVector<bool> dominated(m_numVertices, false);
    for (int v : candidateSet) {
        if (v < 0 || v >= m_numVertices) return false;
        dominated[v] = true;
        for (int nb : m_adj[v])
            dominated[nb] = true;
    }
    for (bool d : dominated)
        if (!d) return false;
    return true;
}

/* ---- Accessor ---- */

QVector<QVector<int>> DominatingSet8::graph() const { return m_adj; }

/* ---- Reset ---- */

void DominatingSet8::resetStatistics()
{
    m_adj.clear();
    m_numVertices = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
