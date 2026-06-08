/**
 * @file VertexCover7.cpp
 * @brief VertexCover7 实现
 *
 * 实现顶点覆盖：半整数LP松弛与冠分解核化参数化约减。
 */

#include "utils/graph244/VertexCover7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VertexCover7::VertexCover7(QObject *parent) : QObject(parent) {}
VertexCover7::~VertexCover7() = default;

/* ---- Set graph ---- */

void VertexCover7::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_numVertices = numVertices;
    m_edges = edges;
    buildAdjacency();
}

/* ---- Build adjacency list ---- */

void VertexCover7::buildAdjacency()
{
    m_adj.resize(m_numVertices);
    for (auto& list : m_adj) list.clear();

    for (const auto& e : m_edges) {
        if (e.u >= 0 && e.u < m_numVertices && e.v >= 0 && e.v < m_numVertices) {
            m_adj[e.u].append(e.v);
            m_adj[e.v].append(e.u);
        }
    }
}

/* ---- Solve half-integrality LP relaxation (Nemhauser-Trotter) ---- */

VertexCover7::LPResult VertexCover7::solveHalfIntegralLP() const
{
    // Nemhauser-Trotter theorem: construct bipartite graph (v, v') for each vertex
    // and find maximum matching; vertices matched from both sides get x=0.5
    int n = m_numVertices;
    LPResult result;
    result.fractional.resize(n, 0.5);
    result.optimal = true;

    // Build bipartite graph: left = original vertices, right = copies
    // Edge (u, v) becomes edges (u, v+n) and (v, u+n) in bipartite graph
    QVector<QVector<int>> bipAdj(n);
    for (const auto& e : m_edges) {
        bipAdj[e.u].append(e.v);
        bipAdj[e.v].append(e.u);
    }

    // Find maximum matching using augmenting paths
    QVector<int> match = maxMatching(bipAdj, n, n);

    // Classify vertices by matching status
    QVector<int> matchedLeft(n, -1), matchedRight(n, -1);
    for (int i = 0; i < n; ++i) {
        if (match[i] >= 0) {
            matchedLeft[i] = match[i];
            matchedRight[match[i]] = i;
        }
    }

    // Find vertices reachable from unmatched left vertices via alternating paths
    QVector<bool> visited(n, false);
    QVector<int> queue;
    for (int i = 0; i < n; ++i) {
        if (matchedLeft[i] < 0) {
            queue.append(i);
            visited[i] = true;
        }
    }

    while (!queue.isEmpty()) {
        int u = queue.takeFirst();
        for (int v : bipAdj[u]) {
            if (!visited[v] || matchedRight[v] == u) continue;
            if (matchedRight[v] >= 0) {
                visited[v] = true;
                queue.append(v);
            }
        }
    }

    // Nemhauser-Trotter classification
    double obj = 0.0;
    for (int i = 0; i < n; ++i) {
        if (!visited[i] && matchedLeft[i] >= 0)
            result.fractional[i] = 1.0;
        else if (visited[i] && matchedLeft[i] < 0)
            result.fractional[i] = 0.0;
        else
            result.fractional[i] = 0.5;
        obj += result.fractional[i];
    }
    result.objective = obj;
    return result;
}

/* ---- Maximum matching via augmenting paths ---- */

QVector<int> VertexCover7::maxMatching(const QVector<QVector<int>>& bipAdj,
                                          int leftSize, int rightSize) const
{
    QVector<int> matchLeft(leftSize, -1);
    QVector<int> matchRight(rightSize, -1);
    QVector<bool> seen(rightSize);

    // DFS for augmenting path
    std::function<bool(int)> augment = [&](int u) -> bool {
        for (int v : bipAdj[u]) {
            if (v >= rightSize || seen[v]) continue;
            seen[v] = true;
            if (matchRight[v] < 0 || augment(matchRight[v])) {
                matchRight[v] = u;
                matchLeft[u] = v;
                return true;
            }
        }
        return false;
    };

    for (int u = 0; u < leftSize; ++u) {
        seen.fill(false);
        augment(u);
    }
    return matchLeft;
}

/* ---- Find crown decomposition ---- */

bool VertexCover7::findCrown(QVector<int>& crown, QVector<int>& head) const
{
    int n = m_numVertices;

    // Find maximum independent set in the complement via bipartite matching
    // Crown = I \ {isolated vertices}, Head = N(I)
    QVector<bool> inCover;
    inCover.resize(n, false);

    // Use LP relaxation to identify candidate independent set
    LPResult lp = solveHalfIntegralLP();

    QVector<int> independentSet;
    for (int i = 0; i < n; ++i) {
        if (lp.fractional[i] == 0.0)
            independentSet.append(i);
    }

    if (independentSet.isEmpty()) return false;

    // Crown = independent set, Head = neighbors of crown
    crown = independentSet;
    QSet<int> headSet;
    for (int v : crown) {
        for (int nb : m_adj[v])
            headSet.insert(nb);
    }
    // Remove crown vertices from head
    for (int v : crown)
        headSet.remove(v);

    head.clear();
    for (int v : headSet)
        head.append(v);

    // Check crown condition: |head| >= |crown| requires matching from crown to head
    if (head.size() < crown.size()) return false;

    return !crown.isEmpty();
}

/* ---- Crown kernelization ---- */

int VertexCover7::crownKernelize(QVector<int>& forcedVertices,
                                   QVector<int>& removedVertices)
{
    forcedVertices.clear();
    removedVertices.clear();

    LPResult lp = solveHalfIntegralLP();

    // Force vertices with x=1 into cover
    for (int i = 0; i < m_numVertices; ++i) {
        if (lp.fractional[i] == 1.0)
            forcedVertices.append(i);
        else if (lp.fractional[i] == 0.0)
            removedVertices.append(i);
    }

    QVector<int> crown, head;
    if (findCrown(crown, head)) {
        // Add head to cover, remove crown from graph
        for (int v : head)
            forcedVertices.append(v);
        for (int v : crown)
            removedVertices.append(v);
    }

    int kernelSize = m_numVertices - forcedVertices.size() - removedVertices.size();
    m_stats.kernelSize = kernelSize;
    emit kernelReduced(m_numVertices, kernelSize);
    return kernelSize;
}

/* ---- Select uncovered edge ---- */

int VertexCover7::selectUncoveredEdge(const QVector<int>& partialCover,
                                        const QVector<Edge>& edges) const
{
    QSet<int> inCover;
    for (int v : partialCover) inCover.insert(v);

    for (int i = 0; i < edges.size(); ++i) {
        if (!inCover.contains(edges[i].u) && !inCover.contains(edges[i].v))
            return i;
    }
    return -1;
}

/* ---- Bounded search tree ---- */

bool VertexCover7::boundedSearch(int k, QVector<int>& currentCover,
                                   const QVector<Edge>& remaining)
{
    if (k < 0) return false;

    int edgeIdx = selectUncoveredEdge(currentCover, remaining);
    if (edgeIdx < 0) return true;  // all edges covered

    // Branch: include u or v
    const Edge& e = remaining[edgeIdx];
    for (int branch = 0; branch < 2; ++branch) {
        int v = (branch == 0) ? e.u : e.v;
        currentCover.append(v);

        // Filter remaining edges
        QVector<Edge> newRemaining;
        for (const auto& re : remaining) {
            if (re.u != v && re.v != v)
                newRemaining.append(re);
        }

        if (boundedSearch(k - 1, currentCover, newRemaining))
            return true;

        currentCover.removeLast();
    }
    return false;
}

/* ---- Solve ---- */

QVector<int> VertexCover7::solve(int parameter)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: LP relaxation + Nemhauser-Trotter reduction
    m_lpResult = solveHalfIntegralLP();

    // Step 2: Crown kernelization
    QVector<int> forced, removed;
    int kernelSize = crownKernelize(forced, removed);

    QSet<int> removedSet;
    for (int v : removed) removedSet.insert(v);

    // Build reduced edge set
    QVector<Edge> reducedEdges;
    for (const auto& e : m_edges) {
        if (!removedSet.contains(e.u) && !removedSet.contains(e.v))
            reducedEdges.append(e);
    }

    // Step 3: Bounded search on reduced instance
    int k = (parameter < 0) ? reducedEdges.size() / 2 + 1 : parameter - forced.size();
    QVector<int> searchCover;

    if (!reducedEdges.isEmpty())
        boundedSearch(k, searchCover, reducedEdges);

    // Combine forced + search result
    m_cover = forced;
    for (int v : searchCover)
        m_cover.append(v);

    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = m_edges.size();
    m_stats.coverSize = m_cover.size();
    m_stats.kernelSize = kernelSize;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_cover.size(), kernelSize, timer.elapsed());
    return m_cover;
}

/* ---- Get LP relaxation ---- */

VertexCover7::LPResult VertexCover7::lpRelaxation() const
{
    return m_lpResult;
}

/* ---- Verify cover ---- */

bool VertexCover7::verifyCover(const QVector<int>& cover) const
{
    QSet<int> inCover;
    for (int v : cover) inCover.insert(v);

    for (const auto& e : m_edges) {
        if (!inCover.contains(e.u) && !inCover.contains(e.v))
            return false;
    }
    return true;
}

/* ---- Reset ---- */

void VertexCover7::resetStatistics()
{
    m_adj.clear();
    m_edges.clear();
    m_cover.clear();
    m_lpResult = LPResult{};
    m_stats = Stats{};
    m_timeSum = 0.0;
}
