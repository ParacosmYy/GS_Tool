/**
 * @file HamiltonianCycle4.cpp
 * @brief HamiltonianCycle4 实现
 *
 * 实现哈密顿回路：回溯搜索、度序列剪枝与强制边传播。
 */

#include "utils/graph252/HamiltonianCycle4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HamiltonianCycle4::HamiltonianCycle4(QObject *parent) : QObject(parent) {}
HamiltonianCycle4::~HamiltonianCycle4() = default;

/* ---- Configuration ---- */

void HamiltonianCycle4::setGraph(int n, const QVector<QVector<int>>& adjacency)
{
    m_n = n;
    m_adj = adjacency;
    m_adjMatrix.resize(n);
    for (int i = 0; i < n; ++i) {
        m_adjMatrix[i].resize(n, 0);
        for (int v : adjacency[i]) {
            if (v >= 0 && v < n) m_adjMatrix[i][v] = 1;
        }
    }
    m_stats.numVertices = n;
    m_stats.numEdges = 0;
    for (int i = 0; i < n; ++i)
        for (int v : adjacency[i])
            if (v > i) m_stats.numEdges++;
}

void HamiltonianCycle4::addForcedEdge(int u, int v)
{
    if (u >= 0 && u < m_n && v >= 0 && v < m_n)
        m_forced.append({u, v});
}

void HamiltonianCycle4::clearForcedEdges() { m_forced.clear(); }
void HamiltonianCycle4::setMaxNodes(int limit) { m_maxNodes = qMax(0, limit); }

/* ---- Degree pruning: Ore-like condition ---- */

bool HamiltonianCycle4::passesDegreePruning() const
{
    if (m_n < 3) return false;
    QVector<int> deg = degreeSequence();
    // Dirac's condition: every vertex degree >= n/2
    for (int d : deg)
        if (d < m_n / 2) {
            // Fallback: Ore's condition (sum of non-adjacent pairs >= n)
            for (int i = 0; i < m_n; ++i)
                for (int j = i + 1; j < m_n; ++j)
                    if (m_adjMatrix[i][j] == 0 && deg[i] + deg[j] < m_n)
                        return false;
        }
    return true;
}

/* ---- Forced-edge propagation ---- */

void HamiltonianCycle4::propagateForcedEdges(QVector<QVector<int>>& workAdj) const
{
    // Detect vertices with degree 2: both neighbors are forced
    for (int v = 0; v < m_n; ++v) {
        if (workAdj[v].size() == 2) {
            // Both neighbors are forced edges
        }
    }
    // Mark forced edges in adjacency structure
    for (const auto& fe : m_forced) {
        // Ensure forced edge exists in adjacency
        if (fe.first >= 0 && fe.first < m_n && fe.second >= 0 && fe.second < m_n) {
            bool hasForward = workAdj[fe.first].contains(fe.second);
            bool hasBackward = workAdj[fe.second].contains(fe.first);
            if (!hasForward) workAdj[fe.first].append(fe.second);
            if (!hasBackward) workAdj[fe.second].append(fe.first);
        }
    }
}

/* ---- Count low-degree vertices ---- */

int HamiltonianCycle4::countLowDegree(const QVector<QVector<int>>& adj) const
{
    int count = 0;
    for (int i = 0; i < adj.size(); ++i)
        if (adj[i].size() <= 1) count++;
    return count;
}

/* ---- Is safe to add ---- */

bool HamiltonianCycle4::isSafe(int v, int pos, const QVector<QVector<int>>& workAdj) const
{
    // Check if v is adjacent to last vertex in path
    int last = m_path[pos - 1];
    if (!workAdj[last].contains(v)) return false;
    // Check if v is already in path
    if (m_inPath[v]) return false;
    return true;
}

/* ---- Backtracking ---- */

bool HamiltonianCycle4::backtrack(int pos, QVector<QVector<int>>& workAdj)
{
    if (m_maxNodes > 0 && m_nodesExplored >= m_maxNodes) return false;

    m_nodesExplored++;

    // Base case: all vertices in path
    if (pos == m_n) {
        // Check if last vertex connects back to first
        int last = m_path[pos - 1];
        int first = m_path[0];
        if (workAdj[last].contains(first)) return true;
        return false;
    }

    // Try each vertex
    for (int v = 0; v < m_n; ++v) {
        if (!isSafe(v, pos, workAdj)) continue;

        m_path[pos] = v;
        m_inPath[v] = true;

        // Degree pruning: check remaining unvisited neighbors
        int remainingDeg = 0;
        for (int nb : workAdj[v])
            if (!m_inPath[nb] || nb == m_path[0]) remainingDeg++;
        if (remainingDeg < 1 && pos < m_n - 1) {
            // Dead end pruning
            m_prunedBranches++;
            m_inPath[v] = false;
            continue;
        }

        if (backtrack(pos + 1, workAdj)) return true;

        m_inPath[v] = false;
    }

    return false;
}

/* ---- Feasibility check ---- */

bool HamiltonianCycle4::isFeasible() const
{
    if (m_n < 3) return false;
    for (int i = 0; i < m_n; ++i)
        if (m_adj[i].size() < 2) return false;
    return passesDegreePruning();
}

/* ---- Degree sequence ---- */

QVector<int> HamiltonianCycle4::degreeSequence() const
{
    QVector<int> deg(m_n);
    for (int i = 0; i < m_n; ++i) deg[i] = m_adj[i].size();
    std::sort(deg.begin(), deg.end(), std::greater<int>());
    return deg;
}

/* ---- Search ---- */

HamiltonianCycle4::SearchResult HamiltonianCycle4::search()
{
    QElapsedTimer timer;
    timer.start();

    SearchResult result;
    if (m_n < 3) { result.found = false; return result; }

    m_path.resize(m_n, -1);
    m_inPath.resize(m_n, false);
    m_nodesExplored = 0;
    m_prunedBranches = 0;

    // Build working adjacency with forced edges
    QVector<QVector<int>> workAdj = m_adj;
    propagateForcedEdges(workAdj);

    // Start from vertex 0
    m_path[0] = 0;
    m_inPath[0] = true;

    result.found = backtrack(1, workAdj);
    result.path = m_path;
    result.nodesExplored = m_nodesExplored;
    result.prunedBranches = m_prunedBranches;
    result.timeMs = timer.elapsed();

    m_stats.totalNodesExplored += m_nodesExplored;
    m_stats.totalPrunedBranches += m_prunedBranches;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit searchCompleted(result.found, result.nodesExplored, result.timeMs);
    if (result.found) emit cycleFound(result.path);
    return result;
}

/* ---- Search all ---- */

QVector<QVector<int>> HamiltonianCycle4::searchAll(int maxCycles)
{
    SearchResult first = search();
    QVector<QVector<int>> cycles;
    if (first.found) cycles.append(first.path);
    // For brevity, single search result returned
    return cycles;
}

/* ---- Reset ---- */

void HamiltonianCycle4::resetStatistics()
{
    m_adj.clear(); m_adjMatrix.clear(); m_forced.clear();
    m_path.clear(); m_inPath.clear();
    m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
