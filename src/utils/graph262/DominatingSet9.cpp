/**
 * @file DominatingSet9.cpp
 * @brief DominatingSet9 实现
 *
 * 实现支配集：连通变体生成树强制与最小Steiner顶点增广。
 */

#include "utils/graph262/DominatingSet9.h"

#include <QElapsedTimer>
#include <QQueue>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DominatingSet9::DominatingSet9(QObject *parent) : QObject(parent) {}
DominatingSet9::~DominatingSet9() = default;

/* ---- Configuration ---- */

void DominatingSet9::setConnected(bool connected) { m_connected = connected; }

/* ---- Count undominated neighbors ---- */

int DominatingSet9::countUndominated(const QVector<QVector<int>>& adj,
                                       const QVector<bool>& dominated, int v) const
{
    int count = 0;
    if (!dominated[v]) count++;
    for (int u : adj[v]) {
        if (u >= 0 && u < dominated.size() && !dominated[u]) count++;
    }
    return count;
}

/* ---- Greedy dominating set ---- */

QVector<int> DominatingSet9::greedyDominatingSet(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    if (n == 0) return {};

    QVector<bool> dominated(n, false);
    QVector<int> domSet;

    while (true) {
        // Check if all vertices are dominated
        bool allDominated = true;
        for (bool d : dominated) { if (!d) { allDominated = false; break; } }
        if (allDominated) break;

        // Find vertex that dominates the most undominated vertices
        int best = -1;
        int bestCount = 0;
        for (int v = 0; v < n; ++v) {
            if (dominated[v] && !adj[v].isEmpty()) {
                // Prefer already-dominated if it adds new coverage
            }
            int cnt = countUndominated(adj, dominated, v);
            if (cnt > bestCount) { bestCount = cnt; best = v; }
        }
        if (best < 0 || bestCount == 0) break;

        domSet.append(best);
        dominated[best] = true;
        for (int u : adj[best]) {
            if (u >= 0 && u < n) dominated[u] = true;
        }
    }
    return domSet;
}

/* ---- BFS shortest path ---- */

QVector<int> DominatingSet9::bfsPath(const QVector<QVector<int>>& adj,
                                       int src, int dst) const
{
    int n = adj.size();
    if (src < 0 || src >= n || dst < 0 || dst >= n) return {};

    QVector<int> dist(n, -1);
    QVector<int> parent(n, -1);
    QQueue<int> queue;

    dist[src] = 0;
    queue.enqueue(src);

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        if (u == dst) break;
        for (int v : adj[u]) {
            if (v >= 0 && v < n && dist[v] < 0) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                queue.enqueue(v);
            }
        }
    }

    if (dist[dst] < 0) return {}; // No path

    // Reconstruct path
    QVector<int> path;
    int cur = dst;
    while (cur >= 0) {
        path.append(cur);
        cur = parent[cur];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

/* ---- Check connectivity ---- */

bool DominatingSet9::isConnected(const QVector<QVector<int>>& adj,
                                   const QVector<int>& vertices) const
{
    if (vertices.size() <= 1) return true;
    QSet<int> vset;
    for (int v : vertices) vset.insert(v);

    // BFS from first vertex through dominating set subgraph
    QSet<int> visited;
    QQueue<int> queue;
    queue.enqueue(vertices[0]);
    visited.insert(vertices[0]);

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        for (int v : adj[u]) {
            if (vset.contains(v) && !visited.contains(v)) {
                visited.insert(v);
                queue.enqueue(v);
                if (visited.size() == vset.size()) return true;
            }
        }
    }
    return visited.size() == vset.size();
}

/* ---- Enforce spanning tree ---- */

QVector<int> DominatingSet9::enforceSpanningTree(const QVector<QVector<int>>& adj,
                                                    QVector<int>& domSet) const
{
    if (domSet.size() <= 1) return {};

    QSet<int> inSet;
    for (int v : domSet) inSet.insert(v);

    QVector<int> steinerVertices;
    QSet<int> connected;
    connected.insert(domSet[0]);

    // Iteratively connect the closest unconnected dominating vertex
    while (connected.size() < inSet.size()) {
        int bestSrc = -1, bestDst = -1;
        QVector<int> bestPath;
        int bestLen = INT_MAX;

        for (int v : connected) {
            for (int target : domSet) {
                if (connected.contains(target)) continue;
                auto path = bfsPath(adj, v, target);
                if (!path.isEmpty() && path.size() < bestLen) {
                    bestLen = path.size();
                    bestPath = path;
                    bestSrc = v;
                    bestDst = target;
                }
            }
        }

        if (bestPath.isEmpty()) break;

        // Add intermediate vertices as Steiner vertices
        for (int v : bestPath) {
            if (!inSet.contains(v)) {
                steinerVertices.append(v);
                domSet.append(v);
                inSet.insert(v);
            }
            connected.insert(v);
        }
    }
    return steinerVertices;
}

/* ---- Steiner augmentation ---- */

QVector<int> DominatingSet9::steinerAugment(const QVector<QVector<int>>& adjacency,
                                              const QVector<int>& terminals) const
{
    if (terminals.size() <= 1) return {};
    QVector<int> mutableSet = terminals;
    return enforceSpanningTree(adjacency, mutableSet);
}

/* ---- Verify domination ---- */

bool DominatingSet9::verifyDomination(const QVector<QVector<int>>& adjacency,
                                        const QVector<int>& candidate) const
{
    int n = adjacency.size();
    QVector<bool> dominated(n, false);
    for (int v : candidate) {
        if (v >= 0 && v < n) dominated[v] = true;
        for (int u : adjacency[v]) {
            if (u >= 0 && u < n) dominated[u] = true;
        }
    }
    for (bool d : dominated)
        if (!d) return false;
    return true;
}

/* ---- Main compute ---- */

DominatingSet9::DomResult DominatingSet9::compute(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    DomResult result;
    result.dominatingSet = greedyDominatingSet(adjacency);

    if (m_connected && result.dominatingSet.size() > 1) {
        result.isConnected = isConnected(adjacency, result.dominatingSet);
        if (!result.isConnected) {
            auto steiner = enforceSpanningTree(adjacency, result.dominatingSet);
            result.numSteinerAdded = steiner.size();
            result.isConnected = isConnected(adjacency, result.dominatingSet);
        }
    } else {
        result.isConnected = (result.dominatingSet.size() <= 1);
    }

    result.dominationRatio = (n > 0)
        ? static_cast<double>(result.dominatingSet.size()) / n : 0.0;

    m_stats.numVertices = n;
    m_stats.numEdges = 0;
    for (const auto& row : adjacency) m_stats.numEdges += row.size();
    m_stats.numEdges /= 2;
    m_stats.dominatingSetSize = result.dominatingSet.size();
    m_stats.numSteinerVertices = result.numSteinerAdded;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit computationCompleted(result.dominatingSet.size(),
                               result.numSteinerAdded, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DominatingSet9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
