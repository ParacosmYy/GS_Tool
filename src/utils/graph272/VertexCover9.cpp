/**
 * @file VertexCover9.cpp
 * @brief VertexCover9 实现
 *
 * 实现顶点覆盖：LP松弛舍入与皇冠分解核化固定参数可解。
 */

#include "utils/graph272/VertexCover9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VertexCover9::VertexCover9(QObject *parent)
    : QObject(parent) {}
VertexCover9::~VertexCover9() = default;

/* ---- Configuration ---- */

void VertexCover9::setParameterK(int k) { m_paramK = qMax(1, k); }

void VertexCover9::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_n = numVertices;
    m_edges = edges;
    m_cover.clear();
    buildAdjList();
}

/* ---- Build adjacency list ---- */

void VertexCover9::buildAdjList()
{
    m_adj.resize(m_n);
    for (auto& list : m_adj) list.clear();
    for (const Edge& e : m_edges) {
        if (e.u >= 0 && e.u < m_n && e.v >= 0 && e.v < m_n) {
            m_adj[e.u].append(e.v);
            m_adj[e.v].append(e.u);
        }
    }
}

/* ---- Maximal matching ---- */

QVector<VertexCover9::Edge> VertexCover9::maximalMatching() const
{
    QVector<Edge> matching;
    QVector<bool> matched(m_n, false);

    for (const Edge& e : m_edges) {
        if (!matched[e.u] && !matched[e.v]) {
            matching.append(e);
            matched[e.u] = true;
            matched[e.v] = true;
        }
    }
    return matching;
}

/* ---- Augmenting path (for bipartite matching in crown) ---- */

bool VertexCover9::augment(int u, QVector<int>& matchTo, QVector<bool>& visited) const
{
    for (int v : m_adj[u]) {
        if (visited[v] || v >= matchTo.size()) continue;
        visited[v] = true;
        if (matchTo[v] < 0 || augment(matchTo[v], matchTo, visited)) {
            matchTo[v] = u;
            return true;
        }
    }
    return false;
}

/* ---- Find crown decomposition ---- */

bool VertexCover9::findCrown(QVector<int>& crown, QVector<int>& head,
                              QVector<int>& body) const
{
    crown.clear();
    head.clear();
    body.clear();

    // Compute maximal matching
    auto matching = maximalMatching();
    QVector<bool> inMatching(m_n, false);
    for (const Edge& e : matching) {
        inMatching[e.u] = true;
        inMatching[e.v] = true;
    }

    // I = independent set (unmatched vertices)
    QVector<int> independentSet;
    for (int i = 0; i < m_n; ++i)
        if (!inMatching[i]) independentSet.append(i);

    if (independentSet.isEmpty()) return false;

    // Find crown: I must have neighbor set H where |N(I)| <= |I|
    // Build bipartite graph I <-> N(I)
    QSet<int> neighborSet;
    for (int v : independentSet)
        for (int nb : m_adj[v])
            if (!inMatching[nb]) neighborSet.insert(nb);

    // Simple crown: use first independent vertex and its unmatched neighbors
    // For a proper crown, find maximum matching in I-N(I) bipartite graph
    QVector<int> matchTo(m_n, -1);
    int matchCount = 0;
    for (int v : independentSet) {
        QVector<bool> visited(m_n, false);
        if (augment(v, matchTo, visited)) matchCount++;
    }

    // Crown head = matched neighbors of I
    for (int v : independentSet) {
        for (int nb : m_adj[v]) {
            if (matchTo[nb] == v) {
                crown.append(v);
                head.append(nb);
            }
        }
    }

    return !crown.isEmpty();
}

/* ---- LP bound ---- */

double VertexCover9::computeLPBound()
{
    // LP relaxation: min sum(x_v) s.t. x_u + x_v >= 1, 0 <= x_v <= 1
    // Dual = maximum matching (half-integral LP)
    auto matching = maximalMatching();
    m_lpBound = static_cast<double>(matching.size());
    return m_lpBound;
}

/* ---- LP relaxation + rounding ---- */

QVector<int> VertexCover9::solveLP()
{
    QElapsedTimer timer;
    timer.start();

    // Solve LP relaxation (fractional values via greedy)
    QVector<double> fractional(m_n, 0.0);
    for (const Edge& e : m_edges) {
        double need = 1.0;
        double curU = fractional[e.u];
        double curV = fractional[e.v];
        if (curU + curV < need) {
            double remaining = need - curU - curV;
            fractional[e.u] += remaining / 2.0;
            fractional[e.v] += remaining / 2.0;
        }
    }

    // Round: include vertex if x_v >= 0.5
    m_cover.clear();
    for (int i = 0; i < m_n; ++i)
        if (fractional[i] >= 0.5) m_cover.append(i);

    // Verify and fix any uncovered edges
    for (const Edge& e : m_edges) {
        bool covered = false;
        for (int v : m_cover) {
            if (v == e.u || v == e.v) { covered = true; break; }
        }
        if (!covered) {
            // Add vertex with higher fractional value
            m_cover.append(fractional[e.u] >= fractional[e.v] ? e.u : e.v);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.coverSize = m_cover.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coverFound(m_cover.size(), m_n, elapsed);
    return m_cover;
}

/* ---- Bounded search tree for FPT ---- */

bool VertexCover9::boundedSearch(QVector<bool>& inCover, int remaining, int edgeIdx)
{
    // Find first uncovered edge
    int targetEdge = -1;
    for (int i = edgeIdx; i < m_edges.size(); ++i) {
        if (!inCover[m_edges[i].u] && !inCover[m_edges[i].v]) {
            targetEdge = i;
            break;
        }
    }
    if (targetEdge < 0) return true; // All edges covered
    if (remaining <= 0) return false;

    const Edge& e = m_edges[targetEdge];

    // Branch 1: include u
    inCover[e.u] = true;
    if (boundedSearch(inCover, remaining - 1, targetEdge + 1)) return true;
    inCover[e.u] = false;

    // Branch 2: include v
    inCover[e.v] = true;
    if (boundedSearch(inCover, remaining - 1, targetEdge + 1)) return true;
    inCover[e.v] = false;

    return false;
}

/* ---- Kernelized solve ---- */

QVector<int> VertexCover9::solveKernelized()
{
    QElapsedTimer timer;
    timer.start();

    int kernelN = m_n;

    // Crown decomposition kernelization
    QVector<int> crown, head, body;
    while (findCrown(crown, head, body)) {
        // Crown rule: add head to cover, remove crown and head from graph
        for (int h : head) {
            if (!m_cover.contains(h)) m_cover.append(h);
        }
        // Remove crown and head vertices
        QSet<int> toRemove;
        for (int c : crown) toRemove.insert(c);
        for (int h : head) toRemove.insert(h);

        QVector<Edge> newEdges;
        for (const Edge& e : m_edges) {
            if (!toRemove.contains(e.u) && !toRemove.contains(e.v))
                newEdges.append(e);
        }
        m_edges = newEdges;

        // Remap vertices
        kernelN = 0;
        QSet<int> remaining;
        for (const Edge& e : m_edges) { remaining.insert(e.u); remaining.insert(e.v); }
        kernelN = remaining.size();
        buildAdjList();
    }

    // LP bound check: if matching > k, no solution of size k exists
    double bound = computeLPBound();

    // Bounded search tree on kernel
    QVector<bool> inCover(m_n, false);
    // Mark already selected vertices
    for (int v : m_cover) inCover[v] = true;

    int remaining = m_paramK - m_cover.size();
    if (remaining >= 0 && boundedSearch(inCover, remaining, 0)) {
        for (int i = 0; i < m_n; ++i)
            if (inCover[i] && !m_cover.contains(i)) m_cover.append(i);
    }

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.coverSize = m_cover.size();
    m_stats.kernelSize = kernelN;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coverFound(m_cover.size(), kernelN, elapsed);
    return m_cover;
}

/* ---- Accessors ---- */

int VertexCover9::coverSize() const { return m_cover.size(); }

bool VertexCover9::verifyCover(const QVector<int>& cover) const
{
    QSet<int> coverSet(cover.begin(), cover.end());
    for (const Edge& e : m_edges) {
        if (!coverSet.contains(e.u) && !coverSet.contains(e.v))
            return false;
    }
    return true;
}

/* ---- Reset ---- */

void VertexCover9::resetStatistics()
{
    m_n = 0;
    m_edges.clear();
    m_adj.clear();
    m_cover.clear();
    m_lpBound = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
