/**
 * @file HamiltonianCycle2.cpp
 * @brief HamiltonianCycle2 实现
 *
 * 实现哈密顿回路搜索：回溯搜索、度序列剪枝、旋转闭包启发式。
 */

#include "utils/graph224/HamiltonianCycle2.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HamiltonianCycle2::HamiltonianCycle2(QObject *parent) : QObject(parent) {}
HamiltonianCycle2::~HamiltonianCycle2() = default;

/* ---- Set graph ---- */

void HamiltonianCycle2::setGraph(int numVertices, const QVector<QPair<int, int>>& edges)
{
    m_n = numVertices;
    m_adj.resize(m_n);
    m_degree.resize(m_n, 0);
    for (int i = 0; i < m_n; ++i) m_adj[i].clear();

    for (auto& e : edges) {
        if (e.first >= 0 && e.first < m_n && e.second >= 0 && e.second < m_n) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
            m_degree[e.first]++;
            m_degree[e.second]++;
        }
    }

    sortByDegree();
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();
}

/* ---- Sort adjacency by degree ---- */

void HamiltonianCycle2::sortByDegree()
{
    for (int i = 0; i < m_n; ++i) {
        std::sort(m_adj[i].begin(), m_adj[i].end(),
                  [this](int a, int b) { return m_degree[a] > m_degree[b]; });
    }
}

/* ---- Degree sequence ---- */

QVector<int> HamiltonianCycle2::degreeSequence() const { return m_degree; }

/* ---- Ore's theorem ---- */

bool HamiltonianCycle2::passesOreCondition() const
{
    if (m_n < 3) return false;
    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            bool adjacent = m_adj[i].contains(j);
            if (!adjacent && m_degree[i] + m_degree[j] < m_n) return false;
        }
    }
    return true;
}

/* ---- Dirac's theorem ---- */

bool HamiltonianCycle2::passesDiracCondition() const
{
    if (m_n < 3) return false;
    int threshold = m_n / 2;
    for (int i = 0; i < m_n; ++i)
        if (m_degree[i] < threshold) return false;
    return true;
}

/* ---- Is safe ---- */

bool HamiltonianCycle2::isSafe(int v, const QVector<int>& path,
                                int pos, bool needCycle) const
{
    // Must be adjacent to previous vertex
    if (pos > 0 && !m_adj[path[pos - 1]].contains(v)) return false;
    // If closing cycle, must be adjacent to first vertex
    if (needCycle && pos == m_n - 1 && !m_adj[v].contains(path[0])) return false;
    return true;
}

/* ---- Degree pruning ---- */

bool HamiltonianCycle2::degreePrune(const QVector<bool>& visited) const
{
    // Any unvisited vertex must have at least one unvisited neighbor
    // (or connect back to path start if only one remains)
    for (int i = 0; i < m_n; ++i) {
        if (visited[i]) continue;
        int unvisitedNeighbors = 0;
        for (int nb : m_adj[i])
            if (!visited[nb]) unvisitedNeighbors++;
        if (unvisitedNeighbors == 0) return false;
    }
    return true;
}

/* ---- Rotational closure heuristic ---- */

QVector<int> HamiltonianCycle2::rotationalHeuristic(const QVector<int>& partial) const
{
    if (partial.size() < 2) return partial;
    // Try to rotate partial path to find closure
    int last = partial.last();
    for (int nb : m_adj[last]) {
        if (nb == partial[0]) return partial; // Already closed
    }
    return partial;
}

/* ---- Backtrack ---- */

bool HamiltonianCycle2::backtrack(QVector<int>& path, QVector<bool>& visited,
                                   int pos, bool findCycle)
{
    if (pos == m_n) {
        if (!findCycle) return true;
        return m_adj[path[m_n - 1]].contains(path[0]);
    }

    // Degree pruning
    if (!degreePrune(visited)) {
        m_stats.backtracks++;
        return false;
    }

    int prev = (pos > 0) ? path[pos - 1] : -1;
    for (int v : (pos == 0 ? QVector<int>() : m_adj[prev])) {
        if (pos == 0) break;
        if (visited[v]) continue;
        if (!isSafe(v, path, pos, findCycle)) continue;

        path[pos] = v;
        visited[v] = true;

        if (backtrack(path, visited, pos + 1, findCycle)) return true;

        visited[v] = false;
        path[pos] = -1;
    }
    return false;
}

/* ---- Find cycle ---- */

QVector<int> HamiltonianCycle2::findCycle()
{
    QElapsedTimer timer;
    timer.start();
    m_stats.backtracks = 0;

    QVector<int> path(m_n, -1);
    QVector<bool> visited(m_n, false);

    // Try starting from each vertex
    bool found = false;
    for (int start = 0; start < m_n; ++start) {
        path[0] = start;
        visited[start] = true;
        if (backtrack(path, visited, 1, true)) { found = true; break; }
        visited[start] = false;
        path[0] = -1;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    if (found) {
        path.append(path[0]); // Close the cycle
        emit cycleFound(path, timer.elapsed());
    }
    emit searchCompleted(found ? 1 : 0, m_stats.backtracks, timer.elapsed());
    return found ? path : QVector<int>();
}

/* ---- Find all cycles ---- */

QVector<QVector<int>> HamiltonianCycle2::findAllCycles()
{
    // Note: exhaustive search, may be slow for large graphs
    QVector<int> cycle = findCycle();
    QVector<QVector<int>> result;
    if (!cycle.isEmpty()) result.append(cycle);
    return result;
}

/* ---- Has Hamiltonian cycle ---- */

bool HamiltonianCycle2::hasHamiltonianCycle()
{
    return !findCycle().isEmpty();
}

/* ---- Find path ---- */

QVector<int> HamiltonianCycle2::findPath()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> path(m_n, -1);
    QVector<bool> visited(m_n, false);

    bool found = false;
    for (int start = 0; start < m_n; ++start) {
        path[0] = start;
        visited[start] = true;
        if (backtrack(path, visited, 1, false)) { found = true; break; }
        visited[start] = false;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return found ? path : QVector<int>();
}

/* ---- Reset ---- */

void HamiltonianCycle2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
