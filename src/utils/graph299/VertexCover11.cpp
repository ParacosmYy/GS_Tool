/**
 * @file VertexCover11.cpp
 * @brief VertexCover11 实现
 *
 * 实现顶点覆盖：原始对偶方案与半积分松弛的2近似加权顶点覆盖。
 */

#include "utils/graph299/VertexCover11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VertexCover11::VertexCover11(QObject *parent)
    : QObject(parent) {}

VertexCover11::~VertexCover11() = default;

/* ---- Build graph ---- */

void VertexCover11::buildGraph(int numVertices, const QVector<Edge>& edges,
                                const QVector<double>& vertexWeights)
{
    m_n = numVertices;
    m_edges = edges;

    m_vertexWeights.resize(m_n, 1.0);
    for (int i = 0; i < qMin(vertexWeights.size(), m_n); ++i)
        m_vertexWeights[i] = qMax(0.0, vertexWeights[i]);

    buildAdjacency();
}

/* ---- Build adjacency list ---- */

void VertexCover11::buildAdjacency()
{
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_adj[i].clear();

    for (int e = 0; e < m_edges.size(); ++e) {
        int u = m_edges[e].u;
        int v = m_edges[e].v;
        if (u >= 0 && u < m_n && v >= 0 && v < m_n) {
            m_adj[u].append(e);
            m_adj[v].append(e);
        }
    }
}

/* ---- Primal-dual 2-approximation ---- */

VertexCover11::CoverResult VertexCover11::runPrimalDual()
{
    CoverResult result;
    result.totalEdges = m_edges.size();

    // Dual variables y_e for each edge, initially 0
    QVector<double> dual(m_edges.size(), 0.0);
    // Tight flags per vertex
    QVector<bool> tight(m_n, false);
    // Remaining uncovered edges
    QVector<bool> covered(m_edges.size(), false);

    // Raise dual variables until all edges have at least one tight endpoint
    for (int e = 0; e < m_edges.size(); ++e) {
        if (covered[e]) continue;

        int u = m_edges[e].u;
        int v = m_edges[e].v;

        if (tight[u] || tight[v]) {
            covered[e] = true;
            continue;
        }

        // Compute how much dual can increase
        double slackU = m_vertexWeights[u];
        double slackV = m_vertexWeights[v];
        for (int ei : m_adj[u]) if (!covered[ei]) slackU -= dual[ei];
        for (int ei : m_adj[v]) if (!covered[ei]) slackV -= dual[ei];

        double delta = qMin(qMax(slackU, 0.0), qMax(slackV, 0.0));

        // Raise dual for this edge
        dual[e] += delta;

        // Check tightness
        double sumU = 0.0, sumV = 0.0;
        for (int ei : m_adj[u]) sumU += dual[ei];
        for (int ei : m_adj[v]) sumV += dual[ei];

        if (sumU >= m_vertexWeights[u] - 1e-10 && !tight[u]) {
            tight[u] = true;
            result.coverVertices.append(u);
            result.totalWeight += m_vertexWeights[u];
            // Cover all edges incident to u
            for (int ei : m_adj[u]) covered[ei] = true;
        }
        if (sumV >= m_vertexWeights[v] - 1e-10 && !tight[v]) {
            tight[v] = true;
            result.coverVertices.append(v);
            result.totalWeight += m_vertexWeights[v];
            for (int ei : m_adj[v]) covered[ei] = true;
        }
    }

    result.numCovered = result.coverVertices.size();
    return result;
}

/* ---- LP rounding with half-integrality ---- */

VertexCover11::CoverResult VertexCover11::runLPRounding()
{
    // Solve LP relaxation
    LPSolution lp = solveLP();

    CoverResult result;
    result.totalEdges = m_edges.size();

    // Round x_v >= 0.5 to 1 (half-integrality theorem guarantees x_v in {0, 0.5, 1})
    for (int v = 0; v < m_n; ++v) {
        if (lp.x[v] >= 0.5) {
            result.coverVertices.append(v);
            result.totalWeight += m_vertexWeights[v];
        }
    }
    result.numCovered = result.coverVertices.size();
    return result;
}

/* ---- Solve LP relaxation ---- */

VertexCover11::LPSolution VertexCover11::solveLP() const
{
    LPSolution lp;
    lp.x.resize(m_n, 0.0);
    lp.halfIntegral = true;

    // Iterative rounding: start with LP solution via simple greedy
    // For each edge, ensure x_u + x_v >= 1
    // Initialize x_v = degree-based lower bound
    for (int v = 0; v < m_n; ++v) {
        if (m_adj[v].isEmpty()) continue;
        double slack = 1.0;
        for (int e : m_adj[v]) {
            int u = m_edges[e].u + m_edges[e].v - v;
            slack = qMin(slack, 1.0 - lp.x[u]);
        }
        lp.x[v] = qMax(lp.x[v], slack);
        if (lp.x[v] < 0.5) lp.x[v] = 0.0;
        else if (lp.x[v] > 0.5) lp.x[v] = 1.0;
        else lp.x[v] = 0.5;
    }

    // Ensure constraint satisfaction: x_u + x_v >= 1
    for (const auto& e : m_edges) {
        if (lp.x[e.u] + lp.x[e.v] < 1.0 - 1e-10) {
            // Raise the smaller one
            if (m_vertexWeights[e.u] <= m_vertexWeights[e.v])
                lp.x[e.u] = 1.0 - lp.x[e.v];
            else
                lp.x[e.v] = 1.0 - lp.x[e.u];
        }
    }

    // Verify half-integrality
    for (int v = 0; v < m_n; ++v) {
        if (qAbs(lp.x[v] - 0.0) > 0.01 && qAbs(lp.x[v] - 0.5) > 0.01
            && qAbs(lp.x[v] - 1.0) > 0.01) {
            lp.halfIntegral = false;
        }
    }

    lp.lpObjective = 0.0;
    for (int v = 0; v < m_n; ++v)
        lp.lpObjective += m_vertexWeights[v] * lp.x[v];

    return lp;
}

/* ---- Solve primal-dual ---- */

VertexCover11::CoverResult VertexCover11::solvePrimalDual()
{
    QElapsedTimer timer;
    timer.start();

    CoverResult result = runPrimalDual();

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(m_n, m_edges.size(), result.totalWeight, elapsed);

    return result;
}

/* ---- Solve LP rounding ---- */

VertexCover11::CoverResult VertexCover11::solveLPRounding()
{
    QElapsedTimer timer;
    timer.start();

    CoverResult result = runLPRounding();

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(m_n, m_edges.size(), result.totalWeight, elapsed);

    return result;
}

/* ---- Verify cover ---- */

bool VertexCover11::verifyCover(const QVector<int>& cover) const
{
    QSet<int> coverSet;
    for (int v : cover) coverSet.insert(v);

    for (const auto& e : m_edges) {
        if (!coverSet.contains(e.u) && !coverSet.contains(e.v))
            return false;
    }
    return true;
}

/* ---- Reset ---- */

void VertexCover11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_n = 0;
    m_edges.clear();
    m_vertexWeights.clear();
    m_adj.clear();
}
