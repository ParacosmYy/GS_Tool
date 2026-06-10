/**
 * @file TransitiveClosure8.cpp
 * @brief TransitiveClosure8 实现
 *
 * 实现传递闭包：Purdom算法与强连通分量压缩的环感知可达性。
 */

#include "utils/graph296/TransitiveClosure8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransitiveClosure8::TransitiveClosure8(QObject *parent)
    : QObject(parent) {}

TransitiveClosure8::~TransitiveClosure8() = default;

/* ---- Tarjan's SCC DFS ---- */

void TransitiveClosure8::tarjanDFS(int u, const QVector<QVector<int>>& adj,
                                     QVector<int>& disc, QVector<int>& low,
                                     QVector<bool>& onStack, QVector<int>& stack,
                                     QVector<int>& sccId, int& index, int& sccCount) const
{
    disc[u] = low[u] = index++;
    stack.append(u);
    onStack[u] = true;

    for (int v : adj[u]) {
        if (v < 0 || v >= disc.size()) continue;
        if (disc[v] == -1) {
            // Unvisited: recurse
            tarjanDFS(v, adj, disc, low, onStack, stack, sccId, index, sccCount);
            low[u] = qMin(low[u], low[v]);
        } else if (onStack[v]) {
            // Back edge to ancestor on stack
            low[u] = qMin(low[u], disc[v]);
        }
    }

    // If u is root of SCC
    if (low[u] == disc[u]) {
        int w;
        do {
            w = stack.takeLast();
            onStack[w] = false;
            sccId[w] = sccCount;
        } while (w != u);
        sccCount++;
    }
}

/* ---- Find SCCs using Tarjan's algorithm ---- */

QVector<QVector<int>> TransitiveClosure8::findSCCs(const QVector<QVector<int>>& adj,
                                                      int n) const
{
    QVector<int> disc(n, -1), low(n, -1);
    QVector<bool> onStack(n, false);
    QVector<int> stack, sccId(n, -1);
    int index = 0, sccCount = 0;

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1)
            tarjanDFS(i, adj, disc, low, onStack, stack, sccId, index, sccCount);
    }

    // Group vertices by SCC
    QVector<QVector<int>> sccs(sccCount);
    for (int i = 0; i < n; ++i)
        sccs[sccId[i]].append(i);
    return sccs;
}

/* ---- Topological sort of condensation DAG ---- */

QVector<int> TransitiveClosure8::topoSort(const QVector<QVector<int>>& dagAdj,
                                             int n) const
{
    QVector<int> inDegree(n, 0);
    for (int u = 0; u < n; ++u)
        for (int v : dagAdj[u])
            if (v >= 0 && v < n) inDegree[v]++;

    QVector<int> queue;
    for (int i = 0; i < n; ++i)
        if (inDegree[i] == 0) queue.append(i);

    QVector<int> order;
    while (!queue.isEmpty()) {
        int u = queue.takeFirst();
        order.append(u);
        for (int v : dagAdj[u]) {
            if (v >= 0 && v < n) {
                inDegree[v]--;
                if (inDegree[v] == 0) queue.append(v);
            }
        }
    }
    return order;
}

/* ---- Forward DFS from source ---- */

void TransitiveClosure8::forwardReach(int src, const QVector<QVector<int>>& dagAdj,
                                        QVector<bool>& reached) const
{
    QVector<int> stack = {src};
    reached[src] = true;

    while (!stack.isEmpty()) {
        int u = stack.takeLast();
        for (int v : dagAdj[u]) {
            if (v >= 0 && v < reached.size() && !reached[v]) {
                reached[v] = true;
                stack.append(v);
            }
        }
    }
}

/* ---- Compute transitive closure via Purdom's SCC condensation ---- */

TransitiveClosure8::ClosureResult TransitiveClosure8::compute(
    const QVector<QVector<int>>& adjacency, int n)
{
    QElapsedTimer timer;
    timer.start();

    ClosureResult result;
    m_n = n;

    if (n <= 0) return result;

    // Phase 1: Find SCCs
    auto sccs = findSCCs(adjacency, n);
    int numSCCs = sccs.size();
    result.numSCCs = numSCCs;

    // Map each vertex to its SCC id
    QVector<int> sccId(n, 0);
    for (int s = 0; s < numSCCs; ++s)
        for (int v : sccs[s])
            sccId[v] = s;
    result.sccIds = sccId;

    // Phase 2: Build condensation DAG
    QVector<QVector<int>> condAdj(numSCCs);
    for (int u = 0; u < n; ++u) {
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n && sccId[u] != sccId[v]) {
                // Add edge in condensation (avoid duplicates)
                if (!condAdj[sccId[u]].contains(sccId[v]))
                    condAdj[sccId[u]].append(sccId[v]);
            }
        }
    }

    // Phase 3: Topological sort of condensation DAG
    auto topo = topoSort(condAdj, numSCCs);

    // Phase 4: Compute reachability in condensation DAG
    // Process in reverse topological order (Purdom's approach)
    QVector<QVector<bool>> sccReach(numSCCs, QVector<bool>(numSCCs, false));
    for (int i = topo.size() - 1; i >= 0; --i) {
        int s = topo[i];
        sccReach[s][s] = true;
        for (int t : condAdj[s]) {
            sccReach[s][t] = true;
            // Propagate reachability from t to s
            for (int r = 0; r < numSCCs; ++r)
                if (sccReach[t][r]) sccReach[s][r] = true;
        }
    }

    // Phase 5: Expand SCC reachability to full vertex reachability
    m_closure = QVector<QVector<bool>>(n, QVector<bool>(n, false));
    int reachablePairs = 0;

    for (int u = 0; u < n; ++u) {
        int su = sccId[u];
        for (int v = 0; v < n; ++v) {
            int sv = sccId[v];
            // u reaches v if their SCCs are reachable
            // Within same SCC, all vertices reach each other
            m_closure[u][v] = sccReach[su][sv];
            if (m_closure[u][v]) reachablePairs++;
        }
    }

    result.reachability = m_closure;
    result.numReachablePairs = reachablePairs;

    int edgeCount = 0;
    for (int i = 0; i < n; ++i) edgeCount += adjacency[i].size();
    m_stats.numVertices = n;
    m_stats.numEdges = edgeCount;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit closureDone(n, numSCCs, reachablePairs, timer.elapsed());

    return result;
}

/* ---- Check reachability ---- */

bool TransitiveClosure8::canReach(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    return m_closure[u][v];
}

/* ---- Get all vertices reachable from u ---- */

QVector<int> TransitiveClosure8::reachableFrom(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_n) return result;
    for (int v = 0; v < m_n; ++v)
        if (m_closure[u][v]) result.append(v);
    return result;
}

/* ---- Reset ---- */

void TransitiveClosure8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_closure.clear();
    m_n = 0;
}
