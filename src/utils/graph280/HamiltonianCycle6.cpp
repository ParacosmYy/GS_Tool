/**
 * @file HamiltonianCycle6.cpp
 * @brief HamiltonianCycle6 实现
 *
 * 实现哈密顿回路：回溯度序列剪枝与前向检查约束路径扩展。
 */

#include "utils/graph280/HamiltonianCycle6.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HamiltonianCycle6::HamiltonianCycle6(QObject *parent)
    : QObject(parent) {}

HamiltonianCycle6::~HamiltonianCycle6() = default;

/* ---- Configuration ---- */

void HamiltonianCycle6::setGraph(const QVector<QVector<int>>& adjacencyMatrix)
{
    m_adj = adjacencyMatrix;
    m_n = adjacencyMatrix.size();
    m_stats.numVertices = m_n;
    int edges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (i < m_adj.size() && j < m_adj[i].size() && m_adj[i][j])
                edges++;
    m_stats.numEdges = edges;
}

/* ---- Degree computation ---- */

int HamiltonianCycle6::degree(int v) const
{
    int deg = 0;
    if (v >= m_adj.size()) return 0;
    for (int j = 0; j < m_n; ++j)
        if (j < m_adj[v].size() && m_adj[v][j]) deg++;
    return deg;
}

/* ---- Degree-sequence pruning ---- */

bool HamiltonianCycle6::degreePruning(int currentVertex) const
{
    // Prune if any unvisited vertex has degree < 2
    // (Hamiltonian cycle requires each vertex to have at least 2 neighbors)
    for (int v = 0; v < m_n; ++v) {
        if (!m_visited[v] && degree(v) < 2)
            return false;
    }
    return true;
}

/* ---- Forward-checking ---- */

bool HamiltonianCycle6::forwardCheck(int pos) const
{
    if (pos >= m_n - 1) return true;

    // Count unvisited vertices adjacent to each unvisited vertex
    // If any unvisited vertex has no unvisited neighbors left (and we're
    // not at the end), the path cannot be extended
    int unvisitedCount = 0;
    for (int v = 0; v < m_n; ++v)
        if (!m_visited[v]) unvisitedCount++;

    if (unvisitedCount == 0) return true;

    for (int v = 0; v < m_n; ++v) {
        if (m_visited[v]) continue;
        // Count unvisited neighbors of v
        int unvisitedNeighbors = 0;
        for (int u = 0; u < m_n; ++u) {
            if (!m_visited[u] && v < m_adj.size() && u < m_adj[v].size()
                && m_adj[v][u])
                unvisitedNeighbors++;
        }
        // If last unvisited vertex has no path back to start, fail
        if (unvisitedCount == 1 && pos == m_n - 1) {
            // Check if v connects to path[0]
            int start = m_path[0];
            if (v < m_adj.size() && start < m_adj[v].size() && m_adj[v][start])
                return true;
            return false;
        }
        // If vertex has zero unvisited neighbors and path isn't near complete
        if (unvisitedNeighbors == 0 && unvisitedCount > 1)
            return false;
    }
    return true;
}

/* ---- Backtracking search ---- */

bool HamiltonianCycle6::backtrack(int pos)
{
    // Base case: all vertices visited
    if (pos == m_n) {
        // Check if last vertex connects back to first
        int last = m_path[pos - 1];
        int first = m_path[0];
        if (last < m_adj.size() && first < m_adj[last].size()
            && m_adj[last][first]) {
            return true;
        }
        m_backtrackCount++;
        return false;
    }

    int prev = m_path[pos - 1];

    for (int v = 0; v < m_n; ++v) {
        if (m_visited[v]) continue;
        if (prev >= m_adj.size() || v >= m_adj[prev].size() || !m_adj[prev][v])
            continue;

        // Apply pruning
        if (!degreePruning(v)) {
            m_backtrackCount++;
            continue;
        }

        // Try vertex v
        m_path[pos] = v;
        m_visited[v] = true;

        // Forward-check
        if (forwardCheck(pos)) {
            if (backtrack(pos + 1))
                return true;
        }

        // Undo
        m_visited[v] = false;
        m_backtrackCount++;
    }

    return false;
}

/* ---- Find one cycle ---- */

QVector<int> HamiltonianCycle6::findCycle()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n < 3) return {};

    m_path.resize(m_n);
    m_visited.resize(m_n);
    m_visited.fill(false);
    m_backtrackCount = 0;

    // Start from vertex 0
    m_path[0] = 0;
    m_visited[0] = true;

    bool found = backtrack(1);

    double elapsed = timer.elapsed();
    m_stats.cycleFound = found;
    m_stats.backtrackCount = m_backtrackCount;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cycleFound(m_n, m_backtrackCount, elapsed);

    return found ? m_path : QVector<int>();
}

/* ---- Find all cycles ---- */

QVector<QVector<int>> HamiltonianCycle6::findAllCycles()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> cycles;
    if (m_n < 3) return cycles;

    // For finding all: use modified backtracking that continues after finding each
    QVector<int> path(m_n);
    QVector<bool> visited(m_n, false);
    int btCount = 0;

    path[0] = 0;
    visited[0] = true;

    // Recursive lambda for all cycles
    // Manual iterative approach to avoid deep recursion
    QVector<int> candidates(m_n);
    for (int i = 0; i < m_n; ++i) candidates[i] = i;

    // Use findCycle approach but keep searching
    for (int start = 0; start < m_n; ++start) {
        m_path.resize(m_n);
        m_visited.resize(m_n);
        m_visited.fill(false);
        m_path[0] = start;
        m_visited[start] = true;

        if (backtrack(1)) {
            QVector<int> cycle = m_path;
            // Avoid duplicates (rotation)
            bool isDupe = false;
            for (const auto& existing : cycles) {
                if (existing.size() == cycle.size()) {
                    int rotOffset = existing.indexOf(cycle[0]);
                    if (rotOffset >= 0) {
                        bool match = true;
                        for (int i = 0; i < cycle.size(); ++i) {
                            if (existing[(rotOffset + i) % cycle.size()] != cycle[i]) {
                                match = false;
                                break;
                            }
                        }
                        if (match) { isDupe = true; break; }
                    }
                }
            }
            if (!isDupe) cycles.append(cycle);
        }
        m_visited[start] = false;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return cycles;
}

/* ---- Check existence ---- */

bool HamiltonianCycle6::hasHamiltonianCycle()
{
    auto cycle = findCycle();
    return !cycle.isEmpty();
}

/* ---- Reset ---- */

void HamiltonianCycle6::resetStatistics()
{
    m_adj.clear();
    m_path.clear();
    m_visited.clear();
    m_n = 0;
    m_backtrackCount = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
