/**
 * @file DominatingSet7.cpp
 * @brief DominatingSet7 实现
 *
 * 实现贪心连通支配集：BFS连通性检查、Steiner树剪枝、无线网络骨干优化。
 */

#include "utils/graph234/DominatingSet7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <queue>

/* ---- Construction / Destruction ---- */

DominatingSet7::DominatingSet7(QObject *parent) : QObject(parent) {}
DominatingSet7::~DominatingSet7() = default;

/* ---- Graph building ---- */

void DominatingSet7::buildGraph(int numVertices, const QVector<Edge>& edges)
{
    m_numVertices = qMax(0, numVertices);
    m_adjList.resize(m_numVertices);
    m_weights.resize(m_numVertices);
    for (int i = 0; i < m_numVertices; ++i) {
        m_adjList[i].clear();
        m_weights[i].resize(m_numVertices, 0.0);
    }

    for (auto& e : edges) {
        if (e.from >= 0 && e.from < m_numVertices &&
            e.to >= 0 && e.to < m_numVertices) {
            m_adjList[e.from].append(e.to);
            m_adjList[e.to].append(e.from);
            m_weights[e.from][e.to] = e.weight;
            m_weights[e.to][e.from] = e.weight;
        }
    }

    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = edges.size();
    m_stats.graphDegree = maxDegree();
}

void DominatingSet7::addEdge(int from, int to, double weight)
{
    if (from < 0 || from >= m_numVertices || to < 0 || to >= m_numVertices) return;
    m_adjList[from].append(to);
    m_adjList[to].append(from);
    m_weights[from][to] = weight;
    m_weights[to][from] = weight;
    m_stats.numEdges++;
    m_stats.graphDegree = maxDegree();
}

/* ---- Degree helpers ---- */

int DominatingSet7::degree(int vertex) const
{
    if (vertex < 0 || vertex >= m_numVertices) return 0;
    return m_adjList[vertex].size();
}

int DominatingSet7::maxDegree() const
{
    int maxD = 0;
    for (int i = 0; i < m_numVertices; ++i)
        maxD = qMax(maxD, degree(i));
    return maxD;
}

/* ---- BFS ---- */

QVector<int> DominatingSet7::bfs(int source, const QVector<bool>& allowed) const
{
    QVector<int> parent(m_numVertices, -1);
    if (source < 0 || source >= m_numVertices) return parent;
    QVector<bool> visited(m_numVertices, false);
    std::queue<int> q;
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : m_adjList[u]) {
            if (!visited[v] && (allowed.isEmpty() || allowed[v])) {
                visited[v] = true;
                parent[v] = u;
                q.push(v);
            }
        }
    }
    return parent;
}

/* ---- Connectivity check ---- */

bool DominatingSet7::isConnected(const QVector<int>& subset) const
{
    if (subset.size() <= 1) return true;
    QVector<bool> allowed(m_numVertices, false);
    for (int v : subset) {
        if (v >= 0 && v < m_numVertices) allowed[v] = true;
    }
    QVector<int> parent = bfs(subset[0], allowed);
    for (int v : subset) {
        if (v >= 0 && v < m_numVertices && parent[v] == -1 && v != subset[0])
            return false;
    }
    return true;
}

/* ---- Shortest path ---- */

QVector<int> DominatingSet7::shortestPath(int from, int to,
                                            const QVector<bool>& allowed) const
{
    QVector<int> parent = bfs(from, allowed);
    QVector<int> path;
    int cur = to;
    while (cur != -1) {
        path.append(cur);
        cur = parent[cur];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

/* ---- Compute CDS (greedy) ---- */

QVector<int> DominatingSet7::computeCDS()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numVertices == 0) return QVector<int>();

    // Greedy: pick highest-degree vertex, expand via BFS until all dominated
    QVector<bool> dominated(m_numVertices, false);
    QVector<bool> inCDS(m_numVertices, false);
    QVector<int> cds;

    // Start with highest-degree vertex
    int start = 0;
    int maxDeg = 0;
    for (int i = 0; i < m_numVertices; ++i) {
        if (degree(i) > maxDeg) { maxDeg = degree(i); start = i; }
    }

    cds.append(start);
    inCDS[start] = true;
    dominated[start] = true;
    for (int n : m_adjList[start]) dominated[n] = true;

    // Iteratively add vertex that dominates most undominated neighbors
    while (true) {
        int bestCandidate = -1;
        int bestGain = 0;

        for (int v = 0; v < m_numVertices; ++v) {
            if (inCDS[v]) continue;
            // Must be adjacent to at least one CDS member for connectivity
            bool adjacentToCDS = false;
            for (int n : m_adjList[v]) {
                if (inCDS[n]) { adjacentToCDS = true; break; }
            }
            if (!adjacentToCDS) continue;

            // Count newly dominated neighbors
            int gain = 0;
            if (!dominated[v]) gain++;
            for (int n : m_adjList[v]) {
                if (!dominated[n]) gain++;
            }
            if (gain > bestGain) { bestGain = gain; bestCandidate = v; }
        }

        if (bestCandidate < 0 || bestGain == 0) break;

        cds.append(bestCandidate);
        inCDS[bestCandidate] = true;
        dominated[bestCandidate] = true;
        for (int n : m_adjList[bestCandidate]) dominated[n] = true;
    }

    m_dominatingSet = cds;
    m_stats.dominatingSetSize = cds.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit dominatingSetComputed(cds.size(), 0, timer.elapsed());
    return cds;
}

/* ---- Steiner pruning ---- */

QVector<int> DominatingSet7::steinerPrune(const QVector<int>& cds)
{
    QElapsedTimer timer;
    timer.start();

    if (cds.size() <= 2) return cds;

    QVector<int> result = cds;
    bool changed = true;
    int pruned = 0;

    while (changed) {
        changed = false;
        for (int i = result.size() - 1; i >= 0; --i) {
            int v = result[i];
            QVector<int> reduced = result;
            reduced.removeAt(i);

            // Check if still dominating
            QVector<bool> dominated(m_numVertices, false);
            for (int u : reduced) {
                dominated[u] = true;
                for (int n : m_adjList[u]) dominated[n] = true;
            }

            bool allDominated = true;
            for (int j = 0; j < m_numVertices; ++j) {
                if (!dominated[j]) { allDominated = false; break; }
            }

            if (allDominated && isConnected(reduced)) {
                result = reduced;
                pruned++;
                changed = true;
                break;
            }
        }
    }

    m_stats.steinerPruned = pruned;
    m_dominatingSet = result;
    m_stats.dominatingSetSize = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit dominatingSetComputed(result.size(), pruned, timer.elapsed());
    return result;
}

/* ---- Dominating check ---- */

bool DominatingSet7::isDominating(const QVector<int>& vertexSet) const
{
    QVector<bool> dominated(m_numVertices, false);
    for (int v : vertexSet) {
        if (v < 0 || v >= m_numVertices) continue;
        dominated[v] = true;
        for (int n : m_adjList[v]) dominated[n] = true;
    }
    for (bool d : dominated) if (!d) return false;
    return true;
}

/* ---- Steiner cost ---- */

double DominatingSet7::steinerCost(const QVector<int>& terminals) const
{
    double cost = 0.0;
    for (int i = 0; i + 1 < terminals.size(); ++i) {
        QVector<bool> allowed(m_numVertices, true);
        QVector<int> path = shortestPath(terminals[i], terminals[i + 1], allowed);
        for (int j = 0; j + 1 < path.size(); ++j)
            cost += m_weights[path[j]][path[j + 1]];
    }
    return cost;
}

/* ---- Accessors ---- */

QVector<int> DominatingSet7::neighbors(int vertex) const
{
    if (vertex < 0 || vertex >= m_numVertices) return QVector<int>();
    return m_adjList[vertex];
}

QVector<int> DominatingSet7::dominatingSet() const { return m_dominatingSet; }

/* ---- Reset ---- */

void DominatingSet7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adjList.clear();
    m_weights.clear();
    m_dominatingSet.clear();
    m_numVertices = 0;
}
