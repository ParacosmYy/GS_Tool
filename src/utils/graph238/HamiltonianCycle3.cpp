/**
 * @file HamiltonianCycle3.cpp
 * @brief HamiltonianCycle3 实现
 *
 * 实现哈密顿回路：约束传播、强制边检测、路径合并剪枝。
 */

#include "utils/graph238/HamiltonianCycle3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HamiltonianCycle3::HamiltonianCycle3(QObject *parent) : QObject(parent) {}
HamiltonianCycle3::~HamiltonianCycle3() = default;

/* ---- Set graph ---- */

void HamiltonianCycle3::setGraph(const QVector<QVector<int>>& adjacencyMatrix)
{
    m_n = adjacencyMatrix.size();
    m_adj = adjacencyMatrix;
    m_weights.clear();
    m_weights.resize(m_n, QVector<int>(m_n, 0));
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            m_weights[i][j] = adjacencyMatrix[i][j];
    m_stats.numVertices = m_n;
    int edges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (m_adj[i][j] > 0) edges++;
    m_stats.numEdges = edges;
}

void HamiltonianCycle3::setWeightedGraph(const QVector<QVector<int>>& weights)
{
    m_n = weights.size();
    m_weights = weights;
    m_adj.clear();
    m_adj.resize(m_n, QVector<int>(m_n, 0));
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            m_adj[i][j] = (weights[i][j] > 0) ? 1 : 0;
    m_stats.numVertices = m_n;
    int edges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (m_adj[i][j] > 0) edges++;
    m_stats.numEdges = edges;
}

/* ---- Detect forced edges (degree-2 vertices) ---- */

QVector<QPair<int, int>> HamiltonianCycle3::detectForcedEdges() const
{
    QVector<QPair<int, int>> forced;
    for (int v = 0; v < m_n; ++v) {
        QVector<int> neighbors;
        for (int u = 0; u < m_n; ++u)
            if (m_adj[v][u] > 0) neighbors.append(u);
        // If vertex has degree 2, both edges are forced
        if (neighbors.size() == 2) {
            forced.append(qMakePair(qMin(v, neighbors[0]), qMax(v, neighbors[0])));
            forced.append(qMakePair(qMin(v, neighbors[1]), qMax(v, neighbors[1])));
        }
    }
    return forced;
}

/* ---- Degree validation ---- */

bool HamiltonianCycle3::degreeValid(int u, int v, const QVector<int>& degree) const
{
    // In a Hamiltonian cycle each vertex has exactly degree 2
    return degree[u] < 2 && degree[v] < 2;
}

/* ---- Constraint propagation ---- */

bool HamiltonianCycle3::propagateConstraints(QVector<int>& degree,
                                               QVector<bool>& inPath)
{
    bool changed = true;
    while (changed) {
        changed = false;
        for (int v = 0; v < m_n; ++v) {
            if (inPath[v]) continue;
            QVector<int> available;
            for (int u = 0; u < m_n; ++u) {
                if (m_adj[v][u] > 0 && degree[v] < 2 && degree[u] < 2)
                    available.append(u);
            }
            // If vertex has 0 available neighbors and not in path -> infeasible
            if (available.isEmpty() && !inPath[v]) return false;
            // If degree already 2, vertex is complete
            if (degree[v] == 2) { inPath[v] = true; changed = true; }
        }
    }
    return true;
}

/* ---- Merge forced paths ---- */

QVector<QVector<int>> HamiltonianCycle3::mergePaths(
    const QVector<QPair<int, int>>& forcedEdges) const
{
    QVector<int> parent(m_n);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    // Union-Find for path merging
    auto find = [&](int x) -> int {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    Q_UNUSED(forcedEdges);
    QVector<QVector<int>> chains;
    // Simple chain extraction: follow forced edges
    QVector<bool> visited(m_n, false);
    for (int v = 0; v < m_n; ++v) {
        if (visited[v]) continue;
        QVector<int> chain;
        chain.append(v);
        visited[v] = true;
        // Extend chain forward
        int cur = v;
        bool extended = true;
        while (extended) {
            extended = false;
            for (int u = 0; u < m_n; ++u) {
                if (!visited[u] && m_adj[cur][u] > 0) {
                    // Check if this edge is forced (degree constraint)
                    int deg = 0;
                    for (int k = 0; k < m_n; ++k)
                        if (m_adj[u][k] > 0 && !visited[k]) deg++;
                    if (deg <= 1) {
                        chain.append(u);
                        visited[u] = true;
                        cur = u;
                        extended = true;
                        break;
                    }
                }
            }
        }
        if (chain.size() > 1) chains.append(chain);
    }
    return chains;
}

/* ---- Feasibility check ---- */

bool HamiltonianCycle3::feasibilityCheck(const QVector<bool>& visited) const
{
    int unvisited = 0;
    for (int i = 0; i < m_n; ++i)
        if (!visited[i]) unvisited++;
    // Check if any unvisited vertex is isolated from other unvisited vertices
    for (int v = 0; v < m_n; ++v) {
        if (visited[v]) continue;
        bool hasUnvisitedNeighbor = false;
        for (int u = 0; u < m_n; ++u) {
            if (!visited[u] && m_adj[v][u] > 0) { hasUnvisitedNeighbor = true; break; }
        }
        if (!hasUnvisitedNeighbor && unvisited > 1) return false;
    }
    return true;
}

/* ---- DFS search with constraint propagation ---- */

bool HamiltonianCycle3::dfsSearch(int current, int depth, QVector<int>& path,
                                    QVector<bool>& visited, QVector<int>& degree,
                                    CycleResult& result)
{
    result.nodesExplored++;

    if (depth == m_n) {
        // Check if we can close the cycle back to start
        if (m_adj[current][path[0]] > 0) {
            path.append(path[0]);
            return true;
        }
        return false;
    }

    // Prune: check forced edges for early termination
    QVector<QPair<int, int>> forced = detectForcedEdges();
    auto chains = mergePaths(forced);
    // If forced chains create a premature cycle, prune
    for (const auto& chain : chains) {
        if (chain.size() < m_n) {
            int first = chain.first();
            int last = chain.last();
            if (first != last && m_adj[first][last] > 0 && chain.size() < m_n - depth) {
                result.pruneCount++;
                return false;
            }
        }
    }

    // Try each unvisited neighbor
    for (int next = 0; next < m_n; ++next) {
        if (visited[next] || m_adj[current][next] <= 0) continue;
        if (!degreeValid(current, next, degree)) continue;

        visited[next] = true;
        path.append(next);
        degree[current]++;
        degree[next]++;

        // Constraint propagation
        QVector<bool> inPath = visited;
        QVector<int> degCopy = degree;
        if (propagateConstraints(degCopy, inPath)) {
            if (feasibilityCheck(visited)) {
                if (dfsSearch(next, depth + 1, path, visited, degree, result))
                    return true;
            } else {
                result.pruneCount++;
            }
        } else {
            result.pruneCount++;
        }

        // Backtrack
        path.removeLast();
        visited[next] = false;
        degree[current]--;
        degree[next]--;
    }
    return false;
}

/* ---- Find cycle ---- */

HamiltonianCycle3::CycleResult HamiltonianCycle3::findCycle()
{
    QElapsedTimer timer;
    timer.start();

    CycleResult result;
    if (m_n < 3) { emit searchCompleted(false, 0, timer.elapsed()); return result; }

    QVector<int> path;
    path.append(0);
    QVector<bool> visited(m_n, false);
    visited[0] = true;
    QVector<int> degree(m_n, 0);

    result.found = dfsSearch(0, 1, path, visited, degree, result);
    result.path = path;

    m_stats.totalNodesExplored += result.nodesExplored;
    m_stats.totalPrunes += result.pruneCount;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit searchCompleted(result.found, path.size(), timer.elapsed());
    return result;
}

/* ---- DFS for minimum cost cycle ---- */

void HamiltonianCycle3::dfsMinCycle(int current, int depth, int cost,
                                      QVector<int>& path, QVector<bool>& visited,
                                      CycleResult& best)
{
    if (cost >= best.totalCost) return; // Prune if already worse

    if (depth == m_n) {
        if (m_weights[current][path[0]] > 0) {
            int totalCost = cost + m_weights[current][path[0]];
            if (totalCost < best.totalCost) {
                best.path = path;
                best.path.append(path[0]);
                best.totalCost = totalCost;
                best.found = true;
            }
        }
        return;
    }

    for (int next = 0; next < m_n; ++next) {
        if (visited[next] || m_adj[current][next] <= 0) continue;
        int edgeCost = (m_weights[current][next] > 0) ? m_weights[current][next] : 1;

        visited[next] = true;
        path.append(next);
        dfsMinCycle(next, depth + 1, cost + edgeCost, path, visited, best);
        path.removeLast();
        visited[next] = false;
    }
}

/* ---- Find minimum cost cycle ---- */

HamiltonianCycle3::CycleResult HamiltonianCycle3::findMinCycle()
{
    QElapsedTimer timer;
    timer.start();

    CycleResult best;
    best.totalCost = std::numeric_limits<int>::max();

    if (m_n < 3) { emit searchCompleted(false, 0, timer.elapsed()); return best; }

    QVector<int> path;
    path.append(0);
    QVector<bool> visited(m_n, false);
    visited[0] = true;

    dfsMinCycle(0, 1, 0, path, visited, best);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit searchCompleted(best.found, best.path.size(), timer.elapsed());
    return best;
}

/* ---- Reset ---- */

void HamiltonianCycle3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_weights.clear();
    m_n = 0;
}
