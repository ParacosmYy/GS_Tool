/**
 * @file DominatingSet12.cpp
 * @brief DominatingSet12 实现
 *
 * 实现支配集：贪心度数选择与冗余节点消除实现连通最小支配集近似。
 */

#include "utils/graph304/DominatingSet12.h"

#include <QElapsedTimer>
#include <QSet>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DominatingSet12::DominatingSet12(QObject *parent)
    : QObject(parent) {}

DominatingSet12::~DominatingSet12() = default;

/* ---- Configuration ---- */

void DominatingSet12::setGraph(const Graph& graph)
{
    m_graph = graph;
    m_stats.numVertices = graph.numVertices;
    m_stats.numEdges = 0;
    for (const auto& neighbors : graph.adj)
        m_stats.numEdges += neighbors.size();
    m_stats.numEdges /= 2;
}

void DominatingSet12::setGraphFromEdges(int n, const QVector<QPair<int,int>>& edges)
{
    m_graph.numVertices = n;
    m_graph.adj.resize(n);
    for (auto& a : m_graph.adj) a.clear();

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n) {
            m_graph.adj[e.first].append(e.second);
            m_graph.adj[e.second].append(e.first);
        }
    }
    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
}

/* ---- Compute vertex degrees ---- */

QVector<int> DominatingSet12::computeDegrees() const
{
    QVector<int> deg(m_graph.numVertices, 0);
    for (int v = 0; v < m_graph.numVertices; ++v)
        deg[v] = m_graph.adj[v].size();
    return deg;
}

/* ---- Check if set dominates the graph ---- */

bool DominatingSet12::isDominated(const QVector<int>& domSet) const
{
    QSet<int> covered;
    for (int v : domSet) {
        covered.insert(v);
        for (int nb : m_graph.adj[v])
            covered.insert(nb);
    }
    return covered.size() >= m_graph.numVertices;
}

/* ---- Eliminate redundant vertices ---- */

QVector<int> DominatingSet12::eliminateRedundant(const QVector<int>& domSet) const
{
    QSet<int> remaining = QSet<int>(domSet.begin(), domSet.end());
    // Try removing each vertex in reverse order of addition
    QVector<int> candidates = domSet;
    // Sort by ascending degree (remove low-degree first, less useful)
    QVector<int> deg = computeDegrees();
    std::sort(candidates.begin(), candidates.end(),
              [&](int a, int b) { return deg[a] < deg[b]; });

    for (int v : candidates) {
        remaining.remove(v);
        // Check if still dominating
        QVector<int> testSet = remaining.values().toVector();
        if (!isDominated(testSet))
            remaining.insert(v);  // Put back
    }
    return remaining.values().toVector();
}

/* ---- Check if vertex set induces connected subgraph ---- */

bool DominatingSet12::isConnectedSubgraph(const QVector<int>& vertices) const
{
    if (vertices.size() <= 1) return true;
    QSet<int> vset(vertices.begin(), vertices.end());
    QSet<int> visited;
    QVector<int> queue;
    queue.append(vertices[0]);
    visited.insert(vertices[0]);

    while (!queue.isEmpty()) {
        int cur = queue.takeFirst();
        for (int nb : m_graph.adj[cur]) {
            if (vset.contains(nb) && !visited.contains(nb)) {
                visited.insert(nb);
                queue.append(nb);
            }
        }
    }
    return visited.size() == vset.size();
}

/* ---- Shortest path via BFS ---- */

QVector<int> DominatingSet12::shortestPath(int src, int dst,
                                             const QSet<int>& avoid) const
{
    if (src == dst) return {src};
    QVector<int> parent(m_graph.numVertices, -1);
    QSet<int> visited;
    QVector<int> queue;
    queue.append(src);
    visited.insert(src);

    while (!queue.isEmpty()) {
        int cur = queue.takeFirst();
        for (int nb : m_graph.adj[cur]) {
            if (visited.contains(nb)) continue;
            if (avoid.contains(nb) && nb != dst) continue;
            parent[nb] = cur;
            if (nb == dst) {
                // Reconstruct path
                QVector<int> path;
                int p = dst;
                while (p != -1) { path.append(p); p = parent[p]; }
                std::reverse(path.begin(), path.end());
                return path;
            }
            visited.insert(nb);
            queue.append(nb);
        }
    }
    return {};  // No path found
}

/* ---- Augment to connected via Steiner nodes ---- */

void DominatingSet12::augmentToConnected(DSResult& result) const
{
    if (result.dominatingSet.size() <= 1) {
        result.isConnected = true;
        return;
    }

    QSet<int> dsSet(result.dominatingSet.begin(), result.dominatingSet.end());

    // Build connected components of the DS
    while (!isConnectedSubgraph(result.dominatingSet)) {
        QSet<int> visited;
        QVector<int> queue = {result.dominatingSet[0]};
        visited.insert(result.dominatingSet[0]);

        while (!queue.isEmpty()) {
            int cur = queue.takeFirst();
            for (int nb : m_graph.adj[cur]) {
                if (dsSet.contains(nb) && !visited.contains(nb)) {
                    visited.insert(nb);
                    queue.append(nb);
                }
            }
        }

        // Find a DS vertex not in the visited component
        int target = -1;
        for (int v : result.dominatingSet) {
            if (!visited.contains(v)) { target = v; break; }
        }
        if (target < 0) break;

        // Find any visited vertex to connect to
        int src = *visited.begin();
        QVector<int> path = shortestPath(src, target, {});
        if (path.isEmpty()) break;

        // Add intermediate path vertices to DS
        for (int v : path) {
            if (!dsSet.contains(v)) {
                dsSet.insert(v);
                result.dominatingSet.append(v);
            }
        }
    }
    result.isConnected = isConnectedSubgraph(result.dominatingSet);
}

/* ---- Compute MDS via greedy degree selection ---- */

DominatingSet12::DSResult DominatingSet12::computeMDS()
{
    QElapsedTimer timer;
    timer.start();

    DSResult result;
    int n = m_graph.numVertices;
    if (n == 0) return result;

    QSet<int> dominated;
    QSet<int> inSet;
    QVector<int> effectiveDeg = computeDegrees();

    // Greedy: pick vertex with maximum (undominated neighbors) / degree ratio
    while (dominated.size() < n) {
        int bestV = -1;
        double bestScore = -1.0;

        for (int v = 0; v < n; ++v) {
            if (inSet.contains(v)) continue;
            // Count undominated neighbors including self
            int newCov = 0;
            if (!dominated.contains(v)) newCov++;
            for (int nb : m_graph.adj[v]) {
                if (!dominated.contains(nb)) newCov++;
            }
            if (newCov == 0) continue;
            double score = (double)newCov;
            if (score > bestScore) {
                bestScore = score;
                bestV = v;
            }
        }

        if (bestV < 0) break;

        inSet.insert(bestV);
        dominated.insert(bestV);
        for (int nb : m_graph.adj[bestV])
            dominated.insert(nb);
    }

    result.dominatingSet = inSet.values().toVector();

    // Eliminate redundant nodes
    result.dominatingSet = eliminateRedundant(result.dominatingSet);
    result.setSize = result.dominatingSet.size();
    result.isConnected = isConnectedSubgraph(result.dominatingSet);

    // Build dominatedBy map
    result.dominatedBy.resize(n);
    for (int v = 0; v < n; ++v) result.dominatedBy[v] = -1;
    for (int d : result.dominatingSet) {
        if (result.dominatedBy[d] < 0) result.dominatedBy[d] = d;
        for (int nb : m_graph.adj[d]) {
            if (result.dominatedBy[nb] < 0) result.dominatedBy[nb] = d;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit mdsComputed(result.setSize, result.isConnected, elapsed);
    return result;
}

/* ---- Compute connected MDS ---- */

DominatingSet12::DSResult DominatingSet12::computeConnectedMDS()
{
    QElapsedTimer timer;
    timer.start();

    DSResult result = computeMDS();
    augmentToConnected(result);
    result.setSize = result.dominatingSet.size();
    result.isConnected = isConnectedSubgraph(result.dominatingSet);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit mdsComputed(result.setSize, result.isConnected, elapsed);
    return result;
}

/* ---- Reset ---- */

void DominatingSet12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
