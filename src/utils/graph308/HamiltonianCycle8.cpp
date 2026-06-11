/**
 * @file HamiltonianCycle8.cpp
 * @brief HamiltonianCycle8 实现
 *
 * 实现哈密顿回路检测：度序列剪枝与旋转邻接序回溯搜索实现提前终止的HC检测。
 */

#include "utils/graph308/HamiltonianCycle8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HamiltonianCycle8::HamiltonianCycle8(QObject *parent)
    : QObject(parent) {}

HamiltonianCycle8::~HamiltonianCycle8() = default;

/* ---- Set graph (adjacency matrix) ---- */

void HamiltonianCycle8::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_n = adjacency.size();
    m_degree.resize(m_n, 0);

    for (int i = 0; i < m_n; ++i) {
        m_degree[i] = 0;
        for (int j = 0; j < m_n; ++j)
            if (m_adj[i][j] != 0) m_degree[i]++;
    }

    m_stats.numVertices = m_n;
    int edges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (m_adj[i][j] != 0) edges++;
    m_stats.numEdges = edges;

    buildAdjacencyOrder();
}

/* ---- Set weighted graph ---- */

void HamiltonianCycle8::setWeightedGraph(const QVector<QVector<double>>& weights,
                                           double threshold)
{
    int n = weights.size();
    QVector<QVector<int>> adj(n, QVector<int>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            adj[i][j] = (weights[i][j] > threshold) ? 1 : 0;
    setGraph(adj);
}

/* ---- Build rotated adjacency ordering ---- */

void HamiltonianCycle8::buildAdjacencyOrder()
{
    // Order neighbors by degree (ascending) for better pruning
    m_adjOrder.resize(m_n);
    for (int v = 0; v < m_n; ++v) {
        QVector<int> neighbors;
        for (int u = 0; u < m_n; ++u) {
            if (m_adj[v][u] != 0) neighbors.append(u);
        }
        // Sort by degree ascending (visit low-degree vertices first for pruning)
        std::sort(neighbors.begin(), neighbors.end(), [this](int a, int b) {
            return m_degree[a] < m_degree[b];
        });
        m_adjOrder[v] = neighbors;
    }
}

/* ---- Check necessary conditions ---- */

bool HamiltonianCycle8::necessaryConditions() const
{
    if (m_n < 3) return false;

    // Dirac's theorem: if min degree >= n/2, Hamiltonian cycle exists
    // Ore's theorem: if deg(u)+deg(v) >= n for all non-adjacent pairs
    // We check weaker necessary condition: min degree >= 2
    for (int i = 0; i < m_n; ++i) {
        if (m_degree[i] < 2) return false;
    }

    // Check connectivity via BFS
    QVector<bool> visited(m_n, false);
    QVector<int> queue;
    queue.append(0);
    visited[0] = true;
    int count = 1;
    while (!queue.isEmpty()) {
        int v = queue.takeFirst();
        for (int u = 0; u < m_n; ++u) {
            if (m_adj[v][u] != 0 && !visited[u]) {
                visited[u] = true;
                queue.append(u);
                count++;
            }
        }
    }
    return count == m_n;
}

/* ---- Check if safe to add vertex ---- */

bool HamiltonianCycle8::isSafe(int v, const QVector<int>& path, int pos) const
{
    // Must be adjacent to previous vertex
    if (m_adj[path[pos - 1]][v] == 0) return false;

    // Must not already be visited
    for (int i = 0; i < pos; ++i)
        if (path[i] == v) return false;

    return true;
}

/* ---- Backtracking search ---- */

bool HamiltonianCycle8::backtrack(QVector<int>& path, QVector<bool>& visited,
                                    int pos, CycleResult& result)
{
    // Base case: all vertices in path
    if (pos == m_n) {
        // Check if last vertex connects back to first
        if (m_adj[path[pos - 1]][path[0]] != 0) {
            result.found = true;
            result.cycle = path;
            result.cycle.append(path[0]); // Close the cycle
            return true;
        }
        return false;
    }

    // Try vertices in rotated adjacency order from the last vertex
    int lastV = path[pos - 1];
    for (int next : m_adjOrder[lastV]) {
        if (visited[next]) continue;
        if (!isSafe(next, path, pos)) continue;

        result.nodesExplored++;

        // Pruning: if remaining unvisited vertices have any with degree < 2
        // in the subgraph, no Hamiltonian cycle can exist through this path
        bool pruneOk = true;
        for (int u = 0; u < m_n && pruneOk; ++u) {
            if (visited[u] || u == next) continue;
            int availDeg = 0;
            for (int w = 0; w < m_n; ++w) {
                if (!visited[w] && w != next && m_adj[u][w] != 0) availDeg++;
            }
            // u needs at least 1 edge to the remaining set (including 'next')
            // and also needs to connect to something
            if (availDeg == 0 && m_adj[u][next] == 0) {
                pruneOk = false;
                result.pruningCuts++;
            }
        }
        if (!pruneOk) continue;

        path[pos] = next;
        visited[next] = true;

        if (backtrack(path, visited, pos + 1, result))
            return true;

        // Backtrack
        visited[next] = false;
        path[pos] = -1;
    }

    return false;
}

/* ---- Find one Hamiltonian cycle ---- */

HamiltonianCycle8::CycleResult HamiltonianCycle8::findCycle()
{
    QElapsedTimer timer;
    timer.start();

    CycleResult result;
    result.found = false;

    if (m_n < 3 || !necessaryConditions()) {
        result.timeMs = timer.elapsed();
        emit searchDone(false, 0, result.timeMs);
        return result;
    }

    QVector<int> path(m_n, -1);
    QVector<bool> visited(m_n, false);

    // Start from vertex 0
    path[0] = 0;
    visited[0] = true;

    backtrack(path, visited, 1, result);

    result.timeMs = timer.elapsed();

    m_stats.totalSearches++;
    m_nodesSum += result.nodesExplored;
    m_stats.avgNodesExplored = m_nodesSum / m_stats.totalSearches;
    m_timeSum += result.timeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
    if (result.found) m_stats.cyclesFound++;

    emit searchDone(result.found, result.cycle.size(), result.timeMs);
    return result;
}

/* ---- Find all cycles ---- */

QVector<HamiltonianCycle8::CycleResult> HamiltonianCycle8::findAllCycles(int maxCycles)
{
    QVector<CycleResult> results;
    if (m_n < 3) return results;

    // For finding all cycles, we try different starting vertices
    // and rotate the adjacency order to find different paths
    for (int start = 0; start < m_n && results.size() < maxCycles; ++start) {
        QElapsedTimer timer;
        timer.start();

        CycleResult result;

        QVector<int> path(m_n, -1);
        QVector<bool> visited(m_n, false);
        path[0] = start;
        visited[start] = true;

        backtrack(path, visited, 1, result);
        result.timeMs = timer.elapsed();

        if (result.found) {
            // Normalize: rotate cycle so smallest vertex is first
            results.append(result);
        }

        m_stats.totalSearches++;
        m_nodesSum += result.nodesExplored;
        m_stats.avgNodesExplored = m_nodesSum / m_stats.totalSearches;
        m_timeSum += result.timeMs;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        if (result.found) m_stats.cyclesFound++;
    }

    return results;
}

/* ---- Reset ---- */

void HamiltonianCycle8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodesSum = 0.0;
}
