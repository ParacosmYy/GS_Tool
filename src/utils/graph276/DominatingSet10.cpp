/**
 * @file DominatingSet10.cpp
 * @brief DominatingSet10 实现
 *
 * 实现支配集：贪心加权选择剪枝优化最小权连通支配。
 */

#include "utils/graph276/DominatingSet10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DominatingSet10::DominatingSet10(QObject *parent)
    : QObject(parent) {}
DominatingSet10::~DominatingSet10() = default;

/* ---- Build graph ---- */

void DominatingSet10::buildGraph(int numVertices, const QVector<Edge>& edges,
                                   const QVector<double>& vertexWeights)
{
    m_n = numVertices;
    m_edges = edges;
    m_adj.resize(m_n);
    m_adjSet.resize(m_n);
    for (auto& list : m_adj) list.clear();
    for (auto& set : m_adjSet) set.clear();

    for (const auto& e : edges) {
        if (e.from >= 0 && e.from < m_n && e.to >= 0 && e.to < m_n) {
            m_adj[e.from].append(e.to);
            m_adj[e.to].append(e.from);
            // Track unique neighbors
            if (!m_adjSet[e.from].contains(e.to))
                m_adjSet[e.from].append(e.to);
            if (!m_adjSet[e.to].contains(e.from))
                m_adjSet[e.to].append(e.from);
        }
    }

    m_weights.resize(m_n);
    if (vertexWeights.size() == m_n) {
        m_weights = vertexWeights;
    } else {
        m_weights.fill(1.0);
    }
}

/* ---- Greedy selection helper ---- */

int DominatingSet10::uncoveredCount(const QVector<bool>& dominated) const
{
    int count = 0;
    for (int i = 0; i < m_n; ++i)
        if (!dominated[i]) count++;
    return count;
}

int DominatingSet10::greedySelect(const QVector<bool>& inSet,
                                    const QVector<bool>& dominated) const
{
    int bestVertex = -1;
    double bestRatio = std::numeric_limits<double>::max();

    for (int v = 0; v < m_n; ++v) {
        if (inSet[v]) continue;

        // Count how many new vertices v would dominate
        int newCovered = 0;
        if (!dominated[v]) newCovered++;
        for (int neighbor : m_adjSet[v]) {
            if (!dominated[neighbor]) newCovered++;
        }

        if (newCovered == 0) continue;

        // Weighted greedy: minimize weight / coverage
        double ratio = m_weights[v] / newCovered;
        if (ratio < bestRatio) {
            bestRatio = ratio;
            bestVertex = v;
        }
    }
    return bestVertex;
}

/* ---- Compute dominating set ---- */

QVector<int> DominatingSet10::computeDominatingSet()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    QVector<bool> inSet(m_n, false);
    QVector<bool> dominated(m_n, false);
    QVector<int> ds;

    // Greedy weighted selection
    while (uncoveredCount(dominated) > 0) {
        int v = greedySelect(inSet, dominated);
        if (v < 0) break;  // No more vertices can help

        inSet[v] = true;
        dominated[v] = true;
        for (int neighbor : m_adjSet[v])
            dominated[neighbor] = true;
        ds.append(v);
    }

    // Prune redundant vertices
    ds = pruneRedundant(ds);

    double elapsed = timer.elapsed();
    double totalW = 0.0;
    for (int v : ds) totalW += m_weights[v];
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.dominatingSetSize = ds.size();
    m_stats.totalWeight = totalW;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit dominatingSetFound(ds.size(), totalW, elapsed);
    return ds;
}

/* ---- Prune redundant ---- */

QVector<int> DominatingSet10::pruneRedundant(const QVector<int>& ds)
{
    QVector<int> result = ds;

    // Try removing each vertex; if still dominating, keep it removed
    for (int i = result.size() - 1; i >= 0; --i) {
        QVector<int> reduced = result;
        reduced.removeAt(i);
        if (isDominating(reduced))
            result = reduced;
    }
    return result;
}

/* ---- Connected dominating set ---- */

QVector<int> DominatingSet10::computeConnectedDominatingSet()
{
    QVector<int> ds = computeDominatingSet();
    if (ds.size() <= 1) return ds;

    // Connect components via shortest paths
    QVector<int> connected = connectComponents(ds);

    // Prune again after adding connector vertices
    connected = pruneRedundant(connected);

    double totalW = 0.0;
    for (int v : connected) totalW += m_weights[v];
    m_stats.dominatingSetSize = connected.size();
    m_stats.totalWeight = totalW;
    return connected;
}

/* ---- Union-Find ---- */

int DominatingSet10::findRoot(QVector<int>& parent, int x) const
{
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];  // path compression
        x = parent[x];
    }
    return x;
}

/* ---- Get components ---- */

QVector<QVector<int>> DominatingSet10::getComponents(const QVector<int>& ds) const
{
    QVector<int> parent(m_n);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    QVector<bool> inDs(m_n, false);
    for (int v : ds) inDs[v] = true;

    // Union DS vertices connected by edges
    for (int v : ds) {
        for (int neighbor : m_adjSet[v]) {
            if (inDs[neighbor]) {
                int rv = findRoot(parent, v);
                int rn = findRoot(parent, neighbor);
                if (rv != rn) parent[rv] = rn;
            }
        }
    }

    // Group by root
    QMap<int, QVector<int>> components;
    for (int v : ds) {
        int root = findRoot(parent, v);
        components[root].append(v);
    }
    return components.values().toVector();
}

/* ---- Shortest path (BFS) ---- */

QVector<int> DominatingSet10::shortestPath(int from, int to,
                                             const QVector<bool>& inSet) const
{
    if (from == to) return {from};

    QVector<int> dist(m_n, -1);
    QVector<int> prev(m_n, -1);
    QVector<int> queue;
    queue.append(from);
    dist[from] = 0;

    int head = 0;
    while (head < queue.size()) {
        int u = queue[head++];
        if (u == to) break;
        for (int v : m_adjSet[u]) {
            if (dist[v] < 0) {
                dist[v] = dist[u] + 1;
                prev[v] = u;
                queue.append(v);
            }
        }
    }

    QVector<int> path;
    if (dist[to] < 0) return path;
    for (int cur = to; cur >= 0; cur = prev[cur])
        path.prepend(cur);
    return path;
}

/* ---- Connect components ---- */

QVector<int> DominatingSet10::connectComponents(QVector<int> ds) const
{
    auto components = getComponents(ds);
    if (components.size() <= 1) return ds;

    QSet<int> dsSet;
    for (int v : ds) dsSet.insert(v);

    // Greedily connect components via shortest paths
    while (components.size() > 1) {
        int bestI = 0, bestJ = 1;
        int bestLen = std::numeric_limits<int>::max();
        QVector<int> bestPath;

        for (int i = 0; i < components.size(); ++i) {
            for (int j = i + 1; j < components.size(); ++j) {
                for (int vi : components[i]) {
                    for (int vj : components[j]) {
                        QVector<bool> dummy(m_n, false);
                        auto path = shortestPath(vi, vj, dummy);
                        if (path.size() > 0 && path.size() < bestLen) {
                            bestLen = path.size();
                            bestI = i; bestJ = j;
                            bestPath = path;
                        }
                    }
                }
            }
        }

        // Add path vertices to DS
        for (int v : bestPath) {
            if (!dsSet.contains(v)) {
                ds.append(v);
                dsSet.insert(v);
            }
        }
        components = getComponents(ds);
    }
    return ds;
}

/* ---- Check domination ---- */

bool DominatingSet10::isDominating(const QVector<int>& set) const
{
    QVector<bool> dominated(m_n, false);
    for (int v : set) {
        dominated[v] = true;
        for (int neighbor : m_adjSet[v])
            dominated[neighbor] = true;
    }
    for (int i = 0; i < m_n; ++i)
        if (!dominated[i]) return false;
    return true;
}

/* ---- Accessor ---- */

QVector<QVector<int>> DominatingSet10::adjacencyList() const { return m_adj; }

/* ---- Reset ---- */

void DominatingSet10::resetStatistics()
{
    m_adj.clear();
    m_adjSet.clear();
    m_weights.clear();
    m_edges.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
