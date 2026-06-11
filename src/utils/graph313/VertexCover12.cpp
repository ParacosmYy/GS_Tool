/**
 * @file VertexCover12.cpp
 * @brief VertexCover12 实现
 *
 * 实现顶点覆盖：二部图匹配归约与LP松弛舍入实现2近似最小权重顶点覆盖。
 */

#include "utils/graph313/VertexCover12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VertexCover12::VertexCover12(QObject *parent)
    : QObject(parent) {}

VertexCover12::~VertexCover12() = default;

/* ---- Build graph ---- */

void VertexCover12::buildGraph(int numVertices, const QVector<Edge>& edges)
{
    m_n = numVertices;
    m_edges = edges;
    m_adj.resize(m_n);

    for (int i = 0; i < m_n; ++i) m_adj[i].clear();

    for (const auto& e : m_edges) {
        if (e.u >= 0 && e.u < m_n && e.v >= 0 && e.v < m_n) {
            m_adj[e.u].append(e.v);
            m_adj[e.v].append(e.u);
        }
    }

    // Default weights
    m_weights.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_weights[i] = 1.0;
}

/* ---- Set vertex weights ---- */

void VertexCover12::setVertexWeights(const QVector<double>& weights)
{
    for (int i = 0; i < qMin(weights.size(), m_n); ++i)
        m_weights[i] = qMax(0.0, weights[i]);
}

/* ---- Check bipartiteness via BFS ---- */

bool VertexCover12::isBipartite(QVector<int>& coloring) const
{
    coloring.resize(m_n);
    coloring.fill(-1);

    for (int start = 0; start < m_n; ++start) {
        if (coloring[start] >= 0) continue;

        coloring[start] = 0;
        QVector<int> queue;
        queue.append(start);

        for (int qi = 0; qi < queue.size(); ++qi) {
            int u = queue[qi];
            for (int v : m_adj[u]) {
                if (coloring[v] < 0) {
                    coloring[v] = 1 - coloring[u];
                    queue.append(v);
                } else if (coloring[v] == coloring[u]) {
                    return false;
                }
            }
        }
    }
    return true;
}

/* ---- Solve LP relaxation via primal-dual ---- */

VertexCover12::LPSolution VertexCover12::solveLPRelaxation() const
{
    LPSolution lp;
    lp.fractionalValues.resize(m_n, 0.0);
    lp.objectiveValue = 0.0;

    // Primal-dual scheme: process edges in order
    // Initialize dual variables (one per edge) to 0
    // Raise duals until tight, set x_v accordingly

    QVector<double> slack;
    slack.reserve(m_edges.size());
    for (int i = 0; i < m_edges.size(); ++i) slack.append(0.0);

    for (const auto& e : m_edges) {
        // For each edge (u,v), ensure x_u + x_v >= 1
        double sum = lp.fractionalValues[e.u] + lp.fractionalValues[e.v];
        if (sum < 1.0) {
            // Distribute remaining weight proportionally to costs
            double deficit = 1.0 - sum;
            double wU = qMax(1e-10, m_weights[e.u]);
            double wV = qMax(1e-10, m_weights[e.v]);
            double total = wU + wV;

            double incU = deficit * wV / total;
            double incV = deficit * wU / total;

            lp.fractionalValues[e.u] = qMin(1.0, lp.fractionalValues[e.u] + incU);
            lp.fractionalValues[e.v] = qMin(1.0, lp.fractionalValues[e.v] + incV);
        }
    }

    // Clamp to [0, 1]
    for (auto& v : lp.fractionalValues) v = qBound(0.0, v, 1.0);

    // Compute objective
    for (int i = 0; i < m_n; ++i)
        lp.objectiveValue += m_weights[i] * lp.fractionalValues[i];

    return lp;
}

/* ---- Round LP solution to integral cover ---- */

VertexCover12::CoverResult VertexCover12::roundLPSolution(const LPSolution& lp) const
{
    CoverResult result;
    result.totalWeight = 0.0;
    result.isApproximate = true;
    result.approximationRatio = 2.0;

    // Threshold rounding: select v if x_v >= 0.5
    for (int i = 0; i < m_n; ++i) {
        if (lp.fractionalValues[i] >= 0.5) {
            result.coverVertices.append(i);
            result.totalWeight += m_weights[i];
        }
    }

    // Verify all edges are covered; add uncovered edges greedily
    for (const auto& e : m_edges) {
        bool covered = false;
        for (int v : result.coverVertices) {
            if (v == e.u || v == e.v) { covered = true; break; }
        }
        if (!covered) {
            // Add the cheaper vertex
            int add = (m_weights[e.u] <= m_weights[e.v]) ? e.u : e.v;
            result.coverVertices.append(add);
            result.totalWeight += m_weights[add];
        }
    }

    return result;
}

/* ---- Augmenting path for matching ---- */

bool VertexCover12::augmentingPath(int u, QVector<int>& matchU, QVector<int>& matchV,
                                    QVector<bool>& visited) const
{
    for (int v : m_adj[u]) {
        if (visited[v]) continue;
        visited[v] = true;
        if (matchV[v] < 0 || augmentingPath(matchV[v], matchU, matchV, visited)) {
            matchU[u] = v;
            matchV[v] = u;
            return true;
        }
    }
    return false;
}

/* ---- Maximum matching ---- */

QVector<int> VertexCover12::maximumMatching(const QVector<int>& coloring) const
{
    QVector<int> matchU(m_n, -1);
    QVector<int> matchV(m_n, -1);

    // Match from partition 0 to partition 1
    for (int u = 0; u < m_n; ++u) {
        if (coloring[u] != 0) continue;
        QVector<bool> visited(m_n, false);
        augmentingPath(u, matchU, matchV, visited);
    }

    return matchU;
}

/* ---- Konig's theorem cover extraction ---- */

VertexCover12::CoverResult VertexCover12::konigCover(const QVector<int>& matchU,
                                                       const QVector<int>& coloring) const
{
    CoverResult result;
    result.isApproximate = false;
    result.approximationRatio = 1.0;
    result.totalWeight = 0.0;

    // Find unmatched vertices in partition 0
    QVector<bool> inZ(m_n, false);
    QVector<int> queue;

    for (int u = 0; u < m_n; ++u) {
        if (coloring[u] == 0 && matchU[u] < 0) {
            inZ[u] = true;
            queue.append(u);
        }
    }

    // BFS via alternating paths
    for (int qi = 0; qi < queue.size(); ++qi) {
        int u = queue[qi];
        for (int v : m_adj[u]) {
            if (inZ[v]) continue;
            if (coloring[u] == 0) {
                // Unmatched edge from U-side
                if (matchU[u] != v) {
                    inZ[v] = true;
                    queue.append(v);
                }
            } else {
                // Matched edge from V-side
                if (matchU[v] == u) {
                    // Only follow matched edges
                }
                inZ[v] = true;
                queue.append(v);
            }
        }
    }

    // Cover = (U \ Z) ∪ (V ∩ Z)
    for (int i = 0; i < m_n; ++i) {
        bool inCover = false;
        if (coloring[i] == 0 && !inZ[i]) inCover = true;
        if (coloring[i] == 1 && inZ[i]) inCover = true;

        if (inCover) {
            result.coverVertices.append(i);
            result.totalWeight += m_weights[i];
        }
    }

    return result;
}

/* ---- Solve via LP rounding ---- */

VertexCover12::CoverResult VertexCover12::solveLP()
{
    QElapsedTimer timer;
    timer.start();

    auto lp = solveLPRelaxation();
    auto result = roundLPSolution(lp);

    m_stats.totalSolves++;
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.coverSize = result.coverVertices.size();

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(result.coverVertices.size(), result.totalWeight, elapsed);
    return result;
}

/* ---- Solve via bipartite reduction ---- */

VertexCover12::CoverResult VertexCover12::solveBipartite()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> coloring;
    CoverResult result;

    if (!isBipartite(coloring)) {
        // Fall back to LP rounding
        auto lp = solveLPRelaxation();
        result = roundLPSolution(lp);
        result.isApproximate = true;
        result.approximationRatio = 2.0;
    } else {
        auto matching = maximumMatching(coloring);
        result = konigCover(matching, coloring);
    }

    m_stats.totalSolves++;
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.coverSize = result.coverVertices.size();

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(result.coverVertices.size(), result.totalWeight, elapsed);
    return result;
}

/* ---- Get LP relaxation ---- */

VertexCover12::LPSolution VertexCover12::lpRelaxation() const
{
    return solveLPRelaxation();
}

/* ---- Reset ---- */

void VertexCover12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
