/**
 * @file StrongConnectivity4.cpp
 * @brief StrongConnectivity4 实现
 *
 * 实现Tarjan强连通分量：桥分量树构建、2SCC凝聚图生成、拓扑排序。
 */

#include "utils/graph215/StrongConnectivity4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

StrongConnectivity4::StrongConnectivity4(QObject *parent) : QObject(parent) {}
StrongConnectivity4::~StrongConnectivity4() = default;

/* ---- Build adjacency list ---- */

QVector<QVector<int>> StrongConnectivity4::buildAdj(int n, const QVector<QPair<int, int>>& edges) const
{
    QVector<QVector<int>> adj(n);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n)
            adj[e.first].append(e.second);
    }
    return adj;
}

/* ---- Tarjan DFS ---- */

void StrongConnectivity4::tarjanDFS(int v, int& index, QVector<int>& idx,
                                      QVector<int>& lowLink, QVector<bool>& onStack,
                                      QVector<int>& stack, QVector<QVector<int>>& sccs,
                                      const QVector<QVector<int>>& adj) const
{
    idx[v] = index;
    lowLink[v] = index;
    index++;
    stack.append(v);
    onStack[v] = true;

    for (int w : adj[v]) {
        if (idx[w] < 0) {
            // w has not been visited
            tarjanDFS(w, index, idx, lowLink, onStack, stack, sccs, adj);
            lowLink[v] = qMin(lowLink[v], lowLink[w]);
        } else if (onStack[w]) {
            // w is on stack, back edge
            lowLink[v] = qMin(lowLink[v], idx[w]);
        }
    }

    // If v is a root node, pop the stack to form an SCC
    if (lowLink[v] == idx[v]) {
        QVector<int> component;
        int w;
        do {
            w = stack.takeLast();
            onStack[w] = false;
            component.append(w);
        } while (w != v);
        sccs.append(component);
    }
}

/* ---- Find SCCs ---- */

QVector<QVector<int>> StrongConnectivity4::findSCCs(int n, const QVector<QPair<int, int>>& edges)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> adj = buildAdj(n, edges);
    QVector<int> idx(n, -1);
    QVector<int> lowLink(n, 0);
    QVector<bool> onStack(n, false);
    QVector<int> stack;
    QVector<QVector<int>> sccs;

    int index = 0;
    for (int v = 0; v < n; ++v) {
        if (idx[v] < 0)
            tarjanDFS(v, index, idx, lowLink, onStack, stack, sccs, adj);
    }

    m_stats.totalRuns++;
    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
    m_stats.numComponents = sccs.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit decompositionCompleted(sccs.size(), n, timer.elapsed());
    return sccs;
}

/* ---- Build condensation DAG ---- */

QVector<QPair<int, int>> StrongConnectivity4::buildCondensation(
    const QVector<QVector<int>>& sccs, const QVector<QPair<int, int>>& edges) const
{
    // Map vertex -> component index
    int n = 0;
    for (const auto& c : sccs) for (int v : c) n = qMax(n, v + 1);
    QVector<int> compId(n, -1);
    for (int i = 0; i < sccs.size(); ++i)
        for (int v : sccs[i]) compId[v] = i;

    // Collect inter-component edges (DAG edges)
    QSet<QPair<int, int>> seen;
    QVector<QPair<int, int>> dagEdges;
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n) {
            int c1 = compId[e.first], c2 = compId[e.second];
            if (c1 >= 0 && c2 >= 0 && c1 != c2) {
                QPair<int, int> edge(qMin(c1, c2), qMax(c1, c2));
                if (!seen.contains(edge)) { seen.insert(edge); dagEdges.append({c1, c2}); }
            }
        }
    }
    return dagEdges;
}

/* ---- Build bridge-component tree ---- */

QVector<QVector<int>> StrongConnectivity4::bridgeComponentTree(
    const QVector<QVector<int>>& sccs, const QVector<QPair<int, int>>& edges) const
{
    QVector<QPair<int, int>> dagEdges = buildCondensation(sccs, edges);
    int numSccs = sccs.size();

    QVector<QVector<int>> tree(numSccs);
    for (const auto& e : dagEdges) {
        tree[e.first].append(e.second);
        tree[e.second].append(e.first);  // undirected for tree
    }
    return tree;
}

/* ---- Topological sort ---- */

QVector<int> StrongConnectivity4::topoSort(int numSccs, const QVector<QPair<int, int>>& dagEdges) const
{
    QVector<int> inDeg(numSccs, 0);
    QVector<QVector<int>> adj(numSccs);
    for (const auto& e : dagEdges) {
        adj[e.first].append(e.second);
        inDeg[e.second]++;
    }

    // Kahn's algorithm
    QVector<int> queue;
    for (int i = 0; i < numSccs; ++i)
        if (inDeg[i] == 0) queue.append(i);

    QVector<int> order;
    while (!queue.isEmpty()) {
        int v = queue.takeFirst();
        order.append(v);
        for (int w : adj[v]) {
            inDeg[w]--;
            if (inDeg[w] == 0) queue.append(w);
        }
    }
    return order;
}

/* ---- Is strongly connected ---- */

bool StrongConnectivity4::isStronglyConnected(int n, const QVector<QPair<int, int>>& edges)
{
    QVector<QVector<int>> sccs = findSCCs(n, edges);
    return sccs.size() <= 1;
}

/* ---- Reset ---- */

void StrongConnectivity4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
