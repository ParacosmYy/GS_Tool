/**
 * @file MinimumSpanningTree2.cpp
 * @brief MinimumSpanningTree2 实现
 *
 * 实现Borůvka最小生成树：并行分量收缩、Union-Find、边权重排序。
 */

#include "utils/graph214/MinimumSpanningTree2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MinimumSpanningTree2::MinimumSpanningTree2(QObject *parent) : QObject(parent) {}
MinimumSpanningTree2::~MinimumSpanningTree2() = default;

/* ---- Configuration ---- */

void MinimumSpanningTree2::setNumVertices(int n) { m_n = qMax(0, n); }
void MinimumSpanningTree2::setEdges(const QVector<Edge>& edges) { m_edges = edges; }

/* ---- Union-Find ---- */

int MinimumSpanningTree2::ufFind(QVector<int>& parent, int x) const
{
    while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
    return x;
}

bool MinimumSpanningTree2::ufUnion(QVector<int>& parent, QVector<int>& rank_, int a, int b) const
{
    a = ufFind(parent, a); b = ufFind(parent, b);
    if (a == b) return false;
    if (rank_[a] < rank_[b]) qSwap(a, b);
    parent[b] = a;
    if (rank_[a] == rank_[b]) rank_[a]++;
    return true;
}

/* ---- Borůvka's algorithm ---- */

QVector<MinimumSpanningTree2::Edge> MinimumSpanningTree2::boruvka()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 1 || m_edges.isEmpty()) return {};

    QVector<int> parent(m_n), rank_(m_n, 0);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    QVector<Edge> mst;
    int components = m_n;

    while (components > 1 && mst.size() < m_n - 1) {
        // Find cheapest edge for each component
        QVector<Edge> cheapest(m_n, {-1, -1, std::numeric_limits<double>::max()});

        for (const Edge& e : m_edges) {
            int ca = ufFind(parent, e.from);
            int cb = ufFind(parent, e.to);
            if (ca == cb) continue;
            if (e.weight < cheapest[ca].weight) cheapest[ca] = e;
            if (e.weight < cheapest[cb].weight) cheapest[cb] = e;
        }

        // Merge components via cheapest edges
        for (int i = 0; i < m_n; ++i) {
            if (cheapest[i].from < 0) continue;
            if (ufUnion(parent, rank_, cheapest[i].from, cheapest[i].to)) {
                mst.append(cheapest[i]);
                components--;
            }
        }

        // Safety: if no edges merged, break
        bool merged = false;
        for (int i = 0; i < m_n; ++i) {
            if (cheapest[i].from >= 0) { merged = true; break; }
        }
        if (!merged) break;
    }

    m_stats.totalRuns++;
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.totalWeight = totalWeight(mst);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit mstCompleted(m_n, mst.size(), totalWeight(mst), timer.elapsed());
    return mst;
}

/* ---- Borůvka with parallel component contraction ---- */

QVector<MinimumSpanningTree2::Edge> MinimumSpanningTree2::boruvkaParallel()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 1 || m_edges.isEmpty()) return {};

    QVector<Edge> currentEdges = m_edges;
    int currentN = m_n;
    QVector<Edge> mst;

    while (currentN > 1 && !currentEdges.isEmpty()) {
        QVector<int> parent(currentN), rank_(currentN, 0);
        for (int i = 0; i < currentN; ++i) parent[i] = i;

        // Find cheapest outgoing edge per component (parallel step)
        QVector<Edge> cheapest(currentN, {-1, -1, std::numeric_limits<double>::max()});
        for (const Edge& e : currentEdges) {
            if (e.from >= currentN || e.to >= currentN) continue;
            int ca = ufFind(parent, e.from);
            int cb = ufFind(parent, e.to);
            if (ca == cb) continue;
            if (e.weight < cheapest[ca].weight) cheapest[ca] = e;
            if (e.weight < cheapest[cb].weight) cheapest[cb] = e;
        }

        // Merge
        for (int i = 0; i < currentN; ++i) {
            if (cheapest[i].from >= 0 && ufUnion(parent, rank_, cheapest[i].from, cheapest[i].to))
                mst.append(cheapest[i]);
        }

        // Component contraction: remap vertices
        QVector<int> component(currentN);
        int numComp = 0;
        QVector<int> remap(currentN, -1);
        for (int i = 0; i < currentN; ++i) {
            int root = ufFind(parent, i);
            if (remap[root] < 0) remap[root] = numComp++;
            component[i] = remap[root];
        }

        if (numComp == currentN) break; // No merges

        // Contract edges
        currentEdges = contractEdges(component, numComp);
        currentN = numComp;
    }

    m_stats.totalRuns++;
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.totalWeight = totalWeight(mst);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit mstCompleted(m_n, mst.size(), totalWeight(mst), timer.elapsed());
    return mst;
}

/* ---- Contract edges ---- */

QVector<MinimumSpanningTree2::Edge> MinimumSpanningTree2::contractEdges(
    const QVector<int>& component, int numComponents) const
{
    // Keep minimum-weight edge between each pair of components
    QMap<quint64, Edge> bestEdge;
    for (const Edge& e : m_edges) {
        if (e.from >= component.size() || e.to >= component.size()) continue;
        int ca = component[e.from], cb = component[e.to];
        if (ca == cb) continue;
        int lo = qMin(ca, cb), hi = qMax(ca, cb);
        quint64 key = static_cast<quint64>(lo) * numComponents + hi;
        if (!bestEdge.contains(key) || e.weight < bestEdge[key].weight)
            bestEdge[key] = Edge{lo, hi, e.weight};
    }

    QVector<Edge> result;
    for (auto it = bestEdge.constBegin(); it != bestEdge.constEnd(); ++it)
        result.append(it.value());
    return result;
}

/* ---- Total weight ---- */

double MinimumSpanningTree2::totalWeight(const QVector<Edge>& edges)
{
    double w = 0.0;
    for (const Edge& e : edges) w += e.weight;
    return w;
}

/* ---- Validate MST ---- */

bool MinimumSpanningTree2::isValidMST(const QVector<Edge>& mstEdges) const
{
    if (mstEdges.size() != m_n - 1) return false;
    QVector<int> parent(m_n), rank_(m_n, 0);
    for (int i = 0; i < m_n; ++i) parent[i] = i;
    for (const Edge& e : mstEdges) {
        if (!ufUnion(parent, rank_, e.from, e.to)) return false; // Cycle
    }
    return true;
}

/* ---- Adjacency list ---- */

QVector<QVector<QPair<int, double>>> MinimumSpanningTree2::adjacencyList() const
{
    QVector<QVector<QPair<int, double>>> adj(m_n);
    for (const Edge& e : m_edges) {
        if (e.from < m_n && e.to < m_n) {
            adj[e.from].append({e.to, e.weight});
            adj[e.to].append({e.from, e.weight});
        }
    }
    return adj;
}

/* ---- Reset ---- */

void MinimumSpanningTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
