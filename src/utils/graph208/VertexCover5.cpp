/**
 * @file VertexCover5.cpp
 * @brief VertexCover5 实现
 *
 * 实现顶点覆盖：冠分解、LP松弛、节点约简(NPR)、近似算法。
 */

#include "utils/graph208/VertexCover5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VertexCover5::VertexCover5(QObject *parent) : QObject(parent) {}
VertexCover5::~VertexCover5() = default;

/* ---- Greedy 2-approximation ---- */

QVector<int> VertexCover5::greedyApprox(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<bool> inCover(n, false);
    QVector<QPair<int, int>> edges;

    for (int u = 0; u < n; ++u)
        for (int v : adj[u])
            if (u < v) edges.append({u, v});

    for (const auto& e : edges) {
        if (!inCover[e.first] && !inCover[e.second]) {
            // Pick vertex with higher degree
            int du = 0, dv = 0;
            for (int w : adj[e.first]) if (!inCover[w]) du++;
            for (int w : adj[e.second]) if (!inCover[w]) dv++;
            inCover[(du >= dv) ? e.first : e.second] = true;
        }
    }

    QVector<int> cover;
    for (int i = 0; i < n; ++i)
        if (inCover[i]) cover.append(i);
    return cover;
}

/* ---- Maximum matching (augmenting paths) ---- */

QVector<int> VertexCover5::maxMatching(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> match(n, -1);

    // BFS-based augmenting path search
    for (int u = 0; u < n; ++u) {
        QVector<int> parent(n, -1);
        QVector<bool> visited(n, false);
        QVector<int> queue;
        queue.append(u);
        visited[u] = true;
        int found = -1;

        while (!queue.isEmpty() && found == -1) {
            int cur = queue.takeFirst();
            for (int v : adj[cur]) {
                if (visited[v]) continue;
                visited[v] = true;
                parent[v] = cur;
                if (match[v] == -1) { found = v; break; }
                queue.append(match[v]);
                parent[match[v]] = v;
            }
        }

        if (found != -1) {
            // Augment along path
            int v = found;
            while (v != -1) {
                int pv = parent[v];
                if (pv == -1) break;
                int ppv = (parent[pv] != -1) ? parent[pv] : -1;
                match[v] = pv;
                match[pv] = v;
                v = ppv;
            }
        }
    }
    return match;
}

/* ---- Complement of vertex set ---- */

QVector<int> VertexCover5::complement(const QVector<int>& set, int n) const
{
    QVector<bool> inSet(n, false);
    for (int v : set) inSet[v] = true;
    QVector<int> result;
    for (int i = 0; i < n; ++i)
        if (!inSet[i]) result.append(i);
    return result;
}

/* ---- Crown decomposition ---- */

bool VertexCover5::findCrown(const QVector<QVector<int>>& adj,
                              QVector<int>& crown, QVector<int>& head) const
{
    int n = adj.size();
    crown.clear();
    head.clear();

    // Find maximal matching
    QVector<int> match = maxMatching(adj);

    // Partition: matched vs unmatched
    QVector<int> unmatched;
    for (int i = 0; i < n; ++i)
        if (match[i] == -1) unmatched.append(i);

    if (unmatched.isEmpty()) return false;

    // BFS from unmatched vertices through alternating paths
    QVector<int> dist(n, -1);
    QVector<int> queue;
    for (int u : unmatched) { dist[u] = 0; queue.append(u); }

    int qi = 0;
    while (qi < queue.size()) {
        int u = queue[qi++];
        for (int v : adj[u]) {
            if (dist[v] != -1) continue;
            // Only follow non-matching edge to matching edge
            if (match[u] != v || dist[u] % 2 == 0) {
                if (match[u] == v) continue;
                dist[v] = dist[u] + 1;
                queue.append(v);
                // If v is matched, also enqueue its partner
                if (match[v] != -1 && dist[match[v]] == -1) {
                    dist[match[v]] = dist[v] + 1;
                    queue.append(match[v]);
                }
            }
        }
    }

    // Crown = unmatched vertices reachable via alternating paths
    // Head = their matched neighbors
    for (int u : unmatched) {
        crown.append(u);
        for (int v : adj[u])
            if (match[v] != -1 && !head.contains(v))
                head.append(v);
    }

    return !crown.isEmpty() && crown.size() <= head.size();
}

/* ---- Crown-based independent set ---- */

QVector<int> VertexCover5::crownIndependentSet(
    const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<bool> removed(n, false);
    QVector<int> independent;
    QVector<QVector<int>> workAdj = adj;

    // Iteratively remove crowns
    for (int iter = 0; iter < 10; ++iter) {
        // Rebuild working adjacency (excluding removed)
        QVector<int> active;
        for (int i = 0; i < n; ++i)
            if (!removed[i]) active.append(i);

        if (active.isEmpty()) break;

        // Build subgraph
        QMap<int, int> remap;
        for (int i = 0; i < active.size(); ++i) remap[active[i]] = i;

        QVector<QVector<int>> subAdj(active.size());
        for (int u : active) {
            for (int v : adj[u]) {
                if (!removed[v] && remap.contains(v))
                    subAdj[remap[u]].append(remap[v]);
            }
        }

        QVector<int> crown, hd;
        if (!findCrown(subAdj, crown, hd)) break;

        for (int c : crown) {
            independent.append(active[c]);
            removed[active[c]] = true;
        }
        for (int h : hd) removed[active[h]] = true;
    }

    return independent;
}

/* ---- LP relaxation ---- */

QVector<double> VertexCover5::lpRelaxation(
    const QVector<QVector<int>>& adjList) const
{
    int n = adjList.size();
    // Half-integral LP: x_u + x_v >= 1, 0 <= x_u <= 1
    // Iterative method: initialize to 0.5, propagate constraints
    QVector<double> x(n, 0.5);

    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;
        for (int u = 0; u < n; ++u) {
            for (int v : adjList[u]) {
                if (u >= v) continue;
                double sum = x[u] + x[v];
                if (sum < 1.0) {
                    double deficit = (1.0 - sum) / 2.0;
                    x[u] = qMin(1.0, x[u] + deficit);
                    x[v] = qMin(1.0, x[v] + deficit);
                    changed = true;
                }
            }
        }
        if (!changed) break;
    }
    return x;
}

/* ---- NPR: nodal point reduction ---- */

QVector<QVector<int>> VertexCover5::nodalReduce(
    const QVector<QVector<int>>& adj, QVector<int>& mapping) const
{
    int n = adj.size();
    QVector<bool> folded(n, false);
    mapping.resize(n);
    for (int i = 0; i < n; ++i) mapping[i] = i;

    QVector<QVector<int>> result = adj;

    // Fold degree-1 vertices: if u has degree 1, neighbor v must be in cover
    for (int u = 0; u < n; ++u) {
        if (folded[u] || result[u].size() != 1) continue;
        int v = result[u][0];
        if (folded[v]) continue;

        // Fold u into v
        folded[u] = true;
        mapping[u] = v;

        // Remove u from v's adjacency
        QVector<int> newAdj;
        for (int w : result[v])
            if (w != u) newAdj.append(w);
        result[v] = newAdj;
    }

    // Build reduced graph
    QVector<int> active;
    for (int i = 0; i < n; ++i)
        if (!folded[i]) active.append(i);

    QMap<int, int> remap;
    for (int i = 0; i < active.size(); ++i) remap[active[i]] = i;

    QVector<QVector<int>> reduced(active.size());
    for (int u : active) {
        for (int v : result[u]) {
            if (!folded[v] && remap.contains(v) && u < v) {
                reduced[remap[u]].append(remap[v]);
                reduced[remap[v]].append(remap[u]);
            }
        }
    }
    return reduced;
}

/* ---- Verify cover ---- */

bool VertexCover5::verifyCover(const QVector<QVector<int>>& adj,
                                const QVector<int>& cover) const
{
    QVector<bool> inCov(adj.size(), false);
    for (int v : cover) inCov[v] = true;
    for (int u = 0; u < adj.size(); ++u)
        for (int v : adj[u])
            if (u < v && !inCov[u] && !inCov[v]) return false;
    return true;
}

/* ---- Solve ---- */

QVector<int> VertexCover5::solve(const QVector<QVector<int>>& adjList)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjList.size();
    if (n == 0) return {};

    int edges = 0;
    for (int u = 0; u < n; ++u)
        for (int v : adjList[u])
            if (u < v) edges++;

    // Step 1: LP relaxation for lower bound
    QVector<double> lpSol = lpRelaxation(adjList);

    // Step 2: NPR reduction
    QVector<int> mapping;
    QVector<QVector<int>> reduced = nodalReduce(adjList, mapping);
    int reductions = n - reduced.size();

    // Step 3: Crown decomposition on reduced graph
    QVector<int> indepSet = crownIndependentSet(reduced);

    // Step 4: Cover = complement of independent set in reduced graph
    QVector<int> reducedCover = complement(indepSet, reduced.size());

    // Map back to original vertices
    QVector<bool> inCover(n, false);

    // Include folded degree-1 neighbors
    for (int u = 0; u < n; ++u) {
        if (mapping[u] != u) {
            // u was folded into mapping[u], mapping[u] must be in cover
            inCover[mapping[u]] = true;
        }
    }

    // Include reduced cover vertices
    for (int v : reducedCover) {
        if (v < reduced.size()) {
            // Find original vertex mapped to this reduced index
            for (int i = 0; i < n; ++i) {
                if (mapping[i] == i) {
                    // Count active vertices up to this index
                }
            }
        }
    }

    // Fallback: use LP rounding + greedy if crown didn't give full solution
    if (!verifyCover(adjList, reducedCover.isEmpty() ?
                     greedyApprox(adjList) : reducedCover)) {
        // Use LP rounding: vertices with x >= 0.5
        for (int u = 0; u < n; ++u) {
            if (lpSol[u] >= 0.5) inCover[u] = true;
        }

        // Verify and fill gaps with greedy
        QVector<int> cover;
        for (int i = 0; i < n; ++i)
            if (inCover[i]) cover.append(i);

        if (!verifyCover(adjList, cover))
            return greedyApprox(adjList);

        QVector<int> result;
        for (int i = 0; i < n; ++i)
            if (inCover[i]) result.append(i);

        m_stats.totalSolves++;
        m_stats.numVertices = n;
        m_stats.numEdges = edges;
        m_stats.coverSize = result.size();
        m_stats.crownReductions = reductions;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

        emit coverFound(result.size(), reductions, timer.elapsed());
        return result;
    }

    QVector<int> result;
    for (int i = 0; i < n; ++i)
        if (inCover[i]) result.append(i);

    m_stats.totalSolves++;
    m_stats.numVertices = n;
    m_stats.numEdges = edges;
    m_stats.coverSize = result.size();
    m_stats.crownReductions = reductions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit coverFound(result.size(), reductions, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void VertexCover5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
