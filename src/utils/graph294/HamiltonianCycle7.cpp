/**
 * @file HamiltonianCycle7.cpp
 * @brief HamiltonianCycle7 实现
 *
 * 实现哈密顿回路：Ore度条件验证与Dirac定理的充分条件检查。
 */

#include "utils/graph294/HamiltonianCycle7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HamiltonianCycle7::HamiltonianCycle7(QObject *parent)
    : QObject(parent) {}

HamiltonianCycle7::~HamiltonianCycle7() = default;

/* ---- Compute degree of each vertex ---- */

QVector<int> HamiltonianCycle7::computeDegrees(const QVector<QVector<int>>& adjacency) const
{
    int n = adjacency.size();
    QVector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (adjacency[i][j] != 0) deg[i]++;
    return deg;
}

/* ---- Check Ore's condition ---- */
// Ore: for every pair of non-adjacent vertices u,v: deg(u)+deg(v) >= n

bool HamiltonianCycle7::checkOreCondition(const QVector<QVector<int>>& adjacency) const
{
    int n = adjacency.size();
    if (n < 3) return false;
    auto deg = computeDegrees(adjacency);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (adjacency[i][j] == 0) {  // Non-adjacent pair
                if (deg[i] + deg[j] < n) return false;
            }
        }
    }
    return true;
}

/* ---- Check Dirac's condition ---- */
// Dirac: every vertex has degree >= n/2

bool HamiltonianCycle7::checkDiracCondition(const QVector<QVector<int>>& adjacency) const
{
    int n = adjacency.size();
    if (n < 3) return false;
    auto deg = computeDegrees(adjacency);
    int threshold = n / 2;

    for (int i = 0; i < n; ++i) {
        if (deg[i] < threshold) return false;
    }
    return true;
}

/* ---- Check if Hamiltonian cycle is likely (heuristic) ---- */

bool HamiltonianCycle7::isHamiltonianLikely(const QVector<QVector<int>>& adjacency) const
{
    int n = adjacency.size();
    if (n < 3) return false;
    // If either sufficient condition holds, definitely exists
    if (checkOreCondition(adjacency) || checkDiracCondition(adjacency)) return true;
    // Heuristic: average degree > n/3
    auto deg = computeDegrees(adjacency);
    double avgDeg = 0.0;
    for (int d : deg) avgDeg += d;
    avgDeg /= n;
    return avgDeg > n / 3.0;
}

/* ---- Check if vertex v is safe to add at position pos ---- */

bool HamiltonianCycle7::isSafe(int v, const QVector<QVector<int>>& adj,
                                const QVector<int>& path, int pos) const
{
    // Must be adjacent to previous vertex
    if (adj[path[pos - 1]][v] == 0) return false;
    // Must not already be in path
    for (int i = 0; i < pos; ++i)
        if (path[i] == v) return false;
    return true;
}

/* ---- Recursive backtracking ---- */

bool HamiltonianCycle7::backtrack(const QVector<QVector<int>>& adj,
                                   QVector<int>& path, QVector<bool>& visited,
                                   int pos, int& backtracks)
{
    int n = adj.size();

    // Base: all vertices visited
    if (pos == n) {
        // Check if last vertex connects to first (cycle closure)
        return (adj[path[pos - 1]][path[0]] != 0);
    }

    // Try each vertex as next candidate, sorted by degree (Wañszajn heuristic)
    QVector<int> candidates;
    for (int v = 1; v < n; ++v) {
        if (!visited[v] && isSafe(v, adj, path, pos))
            candidates.append(v);
    }

    for (int v : candidates) {
        path[pos] = v;
        visited[v] = true;

        if (backtrack(adj, path, visited, pos + 1, backtracks))
            return true;

        // Backtrack
        visited[v] = false;
        path[pos] = -1;
        backtracks++;
    }

    return false;
}

/* ---- Find Hamiltonian cycle ---- */

HamiltonianCycle7::CycleResult HamiltonianCycle7::findCycle(
    const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    CycleResult result;
    int n = adjacency.size();
    if (n < 3) return result;

    // Check sufficient conditions
    result.oreCondition = checkOreCondition(adjacency);
    result.diracCondition = checkDiracCondition(adjacency);

    emit conditionChecked(QStringLiteral("Ore"), result.oreCondition);
    emit conditionChecked(QStringLiteral("Dirac"), result.diracCondition);

    // Initialize path
    QVector<int> path(n, -1);
    QVector<bool> visited(n, false);
    path[0] = 0;
    visited[0] = true;
    int backtracks = 0;

    result.hasCycle = backtrack(adjacency, path, visited, 1, backtracks);
    result.numBacktracks = backtracks;

    if (result.hasCycle) {
        path.append(path[0]);  // Close the cycle
        result.cycle = path;
    }

    double elapsed = timer.elapsed();
    result.searchTimeMs = elapsed;

    // Count edges
    int edges = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (adjacency[i][j] != 0) edges++;

    m_stats.numVertices = n;
    m_stats.numEdges = edges;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit searchDone(n, result.hasCycle, elapsed);

    return result;
}

/* ---- Reset ---- */

void HamiltonianCycle7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
