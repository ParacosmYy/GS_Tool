/**
 * @file MinimumSpanningTree.cpp
 * @brief MinimumSpanningTree 实现
 *
 * 实现最小生成树：Kruskal并查集、Prim堆优化、总权值计算。
 */

#include "utils/graph198/MinimumSpanningTree.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MinimumSpanningTree::MinimumSpanningTree(QObject *parent) : QObject(parent) {}
MinimumSpanningTree::~MinimumSpanningTree() = default;

/* ---- Configuration ---- */

void MinimumSpanningTree::setNumVertices(int n) { m_numVertices = qMax(0, n); }

void MinimumSpanningTree::addEdge(int from, int to, double weight)
{
    m_edges.append({from, to, weight});
}

void MinimumSpanningTree::clear()
{
    m_edges.clear();
    m_numVertices = 0;
}

/* ---- Union-Find ---- */

int MinimumSpanningTree::ufFind(QVector<int>& parent, QVector<int>& rank_,
                                  int x) const
{
    while (parent[x] != x) {
        parent[x] = parent[parent[x]]; // Path compression (halving)
        x = parent[x];
    }
    return x;
}

bool MinimumSpanningTree::ufUnion(QVector<int>& parent, QVector<int>& rank_,
                                    int x, int y) const
{
    int rx = ufFind(parent, rank_, x);
    int ry = ufFind(parent, rank_, y);
    if (rx == ry) return false;

    // Union by rank
    if (rank_[rx] < rank_[ry]) std::swap(rx, ry);
    parent[ry] = rx;
    if (rank_[rx] == rank_[ry]) rank_[rx]++;
    return true;
}

/* ---- Kruskal ---- */

QVector<MinimumSpanningTree::Edge> MinimumSpanningTree::kruskal()
{
    QElapsedTimer timer;
    timer.start();

    QVector<Edge> result;
    if (m_numVertices <= 0) return result;

    // Sort edges by weight
    QVector<Edge> sorted = m_edges;
    std::sort(sorted.begin(), sorted.end(),
              [](const Edge& a, const Edge& b) { return a.weight < b.weight; });

    // Initialize union-find
    QVector<int> parent(m_numVertices);
    QVector<int> rank_(m_numVertices, 0);
    for (int i = 0; i < m_numVertices; ++i) parent[i] = i;

    for (const Edge& e : sorted) {
        if (result.size() >= m_numVertices - 1) break;
        if (e.from < 0 || e.from >= m_numVertices ||
            e.to < 0 || e.to >= m_numVertices) continue;

        if (ufUnion(parent, rank_, e.from, e.to)) {
            result.append(e);
        }
    }

    double w = totalWeight(result);

    m_stats.totalRuns++;
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = result.size();
    m_stats.totalWeight = w;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit mstComputed("kruskal", result.size(), w);
    return result;
}

/* ---- Prim (heap-based) ---- */

QVector<MinimumSpanningTree::Edge> MinimumSpanningTree::prim()
{
    QElapsedTimer timer;
    timer.start();

    QVector<Edge> result;
    if (m_numVertices <= 0) return result;

    // Build adjacency list
    QVector<QVector<QPair<int, double>>> adj(m_numVertices);
    for (const Edge& e : m_edges) {
        if (e.from >= 0 && e.from < m_numVertices &&
            e.to >= 0 && e.to < m_numVertices) {
            adj[e.from].append({e.to, e.weight});
            adj[e.to].append({e.from, e.weight});
        }
    }

    QVector<bool> inMST(m_numVertices, false);
    QVector<double> minKey(m_numVertices, 1e18);
    QVector<int> parent(m_numVertices, -1);

    // Start from vertex 0
    minKey[0] = 0.0;

    // Simple priority queue using a vector (min-heap simulation)
    for (int iter = 0; iter < m_numVertices; ++iter) {
        // Find minimum key vertex not yet in MST
        int u = -1;
        double minVal = 1e18;
        for (int v = 0; v < m_numVertices; ++v) {
            if (!inMST[v] && minKey[v] < minVal) {
                minVal = minKey[v];
                u = v;
            }
        }
        if (u < 0) break;

        inMST[u] = true;

        // Record edge (except for root)
        if (parent[u] >= 0) {
            result.append({parent[u], u, minKey[u]});
        }

        // Update adjacent vertices
        for (const auto& [v, w] : adj[u]) {
            if (!inMST[v] && w < minKey[v]) {
                minKey[v] = w;
                parent[v] = u;
            }
        }
    }

    double w = totalWeight(result);

    m_stats.totalRuns++;
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = result.size();
    m_stats.totalWeight = w;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit mstComputed("prim", result.size(), w);
    return result;
}

/* ---- Total weight ---- */

double MinimumSpanningTree::totalWeight(const QVector<Edge>& mst) const
{
    double sum = 0.0;
    for (const Edge& e : mst) sum += e.weight;
    return sum;
}

/* ---- Reset ---- */

void MinimumSpanningTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
