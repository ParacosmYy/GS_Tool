/**
 * @file HamiltonianCycle.cpp
 * @brief HamiltonianCycle 实现
 *
 * 实现哈密顿回路检测：回溯搜索、Warnsdorff启发式排序、度数剪枝。
 */

#include "utils/graph204/HamiltonianCycle.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HamiltonianCycle::HamiltonianCycle(QObject *parent) : QObject(parent) {}
HamiltonianCycle::~HamiltonianCycle() = default;

/* ---- Graph operations ---- */

void HamiltonianCycle::setGraph(const QVector<QVector<int>>& adjList)
{
    m_adj = adjList;
    m_numVertices = adjList.size();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = 0;
    for (const auto& neighbors : m_adj)
        m_stats.numEdges += neighbors.size();
    m_stats.numEdges /= 2; // undirected
}

void HamiltonianCycle::addEdge(int u, int v)
{
    int maxV = qMax(u, v) + 1;
    if (maxV > m_numVertices) {
        m_adj.resize(maxV);
        m_numVertices = maxV;
    }
    m_adj[u].append(v);
    m_adj[v].append(u);
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges++;
}

void HamiltonianCycle::clearGraph()
{
    m_adj.clear();
    m_numVertices = 0;
}

/* ---- Degree-based pruning ---- */

bool HamiltonianCycle::degreePruning() const
{
    // Necessary condition: all vertices must have degree >= 2
    for (int v = 0; v < m_numVertices; ++v) {
        if (m_adj[v].size() < 2) return false;
    }
    return true;
}

/* ---- Warnsdorff ordering ---- */

QVector<int> HamiltonianCycle::warnsdorffOrder(int vertex,
                                                const QVector<bool>& visited) const
{
    // Sort unvisited neighbors by ascending degree (Warnsdorff's heuristic)
    QVector<QPair<int, int>> candidates; // (degree, neighbor)
    for (int nb : m_adj[vertex]) {
        if (!visited[nb]) {
            int degree = 0;
            for (int nn : m_adj[nb]) {
                if (!visited[nn]) degree++;
            }
            candidates.append({degree, nb});
        }
    }
    std::sort(candidates.begin(), candidates.end());

    QVector<int> result;
    for (const auto& c : candidates)
        result.append(c.second);
    return result;
}

/* ---- Basic backtracking ---- */

bool HamiltonianCycle::backtrack(QVector<int>& path, QVector<bool>& visited, int pos)
{
    m_stats.nodesExplored++;

    if (pos == m_numVertices) {
        // Check if last vertex connects back to first
        for (int nb : m_adj[path[pos - 1]]) {
            if (nb == path[0]) return true;
        }
        return false;
    }

    int current = path[pos - 1];
    for (int nb : m_adj[current]) {
        if (!visited[nb]) {
            visited[nb] = true;
            path[pos] = nb;

            if (backtrack(path, visited, pos + 1))
                return true;

            visited[nb] = false;
            path[pos] = -1;
        }
    }
    return false;
}

/* ---- Backtracking with Warnsdorff ---- */

bool HamiltonianCycle::backtrackWarnsdorff(QVector<int>& path,
                                            QVector<bool>& visited, int pos)
{
    m_stats.nodesExplored++;

    if (pos == m_numVertices) {
        for (int nb : m_adj[path[pos - 1]]) {
            if (nb == path[0]) return true;
        }
        return false;
    }

    int current = path[pos - 1];
    auto ordered = warnsdorffOrder(current, visited);

    for (int nb : ordered) {
        visited[nb] = true;
        path[pos] = nb;

        if (backtrackWarnsdorff(path, visited, pos + 1))
            return true;

        visited[nb] = false;
        path[pos] = -1;
    }
    return false;
}

/* ---- Find one cycle ---- */

QVector<int> HamiltonianCycle::findCycle()
{
    QElapsedTimer timer;
    timer.start();
    m_stats.nodesExplored = 0;
    m_stats.pruningCuts = 0;

    if (m_numVertices < 3) return {};

    // Degree pruning
    if (!degreePruning()) {
        m_stats.pruningCuts++;
        emit searchCompleted(m_numVertices, false, timer.elapsed());
        return {};
    }

    QVector<int> path(m_numVertices, -1);
    QVector<bool> visited(m_numVertices, false);

    // Start from vertex 0
    path[0] = 0;
    visited[0] = true;

    // Use Warnsdorff heuristic for faster search
    bool found = backtrackWarnsdorff(path, visited, 1);

    m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    QVector<int> result;
    if (found) {
        result = path;
        result.append(path[0]); // Close the cycle
    }

    emit searchCompleted(m_numVertices, found, timer.elapsed());
    return result;
}

/* ---- Check existence ---- */

bool HamiltonianCycle::hasCycle()
{
    return !findCycle().isEmpty();
}

/* ---- Multi-cycle backtracking ---- */

void HamiltonianCycle::backtrackAll(QVector<int>& path, QVector<bool>& visited,
                                     int pos, QVector<QVector<int>>& results)
{
    m_stats.nodesExplored++;

    if (static_cast<int>(results.size()) >= m_maxCount)
        return;

    if (pos == m_numVertices) {
        for (int nb : m_adj[path[pos - 1]]) {
            if (nb == path[0]) {
                auto cycle = path;
                cycle.append(path[0]);
                results.append(cycle);
                break;
            }
        }
        return;
    }

    int current = path[pos - 1];
    auto ordered = warnsdorffOrder(current, visited);

    for (int nb : ordered) {
        if (static_cast<int>(results.size()) >= m_maxCount) return;
        visited[nb] = true;
        path[pos] = nb;
        backtrackAll(path, visited, pos + 1, results);
        visited[nb] = false;
        path[pos] = -1;
    }
}

/* ---- Find all cycles ---- */

QVector<QVector<int>> HamiltonianCycle::findAllCycles(int maxCount)
{
    QElapsedTimer timer;
    timer.start();
    m_maxCount = maxCount;
    m_stats.nodesExplored = 0;

    if (m_numVertices < 3) return {};

    if (!degreePruning()) return {};

    QVector<QVector<int>> results;
    QVector<int> path(m_numVertices, -1);
    QVector<bool> visited(m_numVertices, false);

    path[0] = 0;
    visited[0] = true;
    backtrackAll(path, visited, 1, results);

    m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(m_numVertices, !results.isEmpty(), timer.elapsed());
    return results;
}

/* ---- Reset ---- */

void HamiltonianCycle::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
