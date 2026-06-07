/**
 * @file DominatingSet5.cpp
 * @brief DominatingSet5 实现
 *
 * 实现支配集：贪心启发式、LP舍入界限、迭代关键顶点包含。
 */

#include "utils/graph206/DominatingSet5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DominatingSet5::DominatingSet5(QObject *parent) : QObject(parent) {}
DominatingSet5::~DominatingSet5() = default;

/* ---- Build graph from adjacency list ---- */

void DominatingSet5::setGraph(const QVector<QVector<int>>& adjList)
{
    m_n = adjList.size();
    m_adj = adjList;
    m_degree.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_degree[i] = m_adj[i].size();
        m_stats.numEdges += m_adj[i].size();
    }
    m_stats.numEdges /= 2;
    m_stats.numVertices = m_n;
}

/* ---- Build graph from edge list ---- */

void DominatingSet5::setEdges(const QVector<QPair<int, int>>& edges,
                                int vertexCount)
{
    m_n = vertexCount;
    m_adj.resize(m_n);
    m_degree.resize(m_n, 0);

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_n && e.second >= 0 && e.second < m_n) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
        }
    }

    for (int i = 0; i < m_n; ++i)
        m_degree[i] = m_adj[i].size();

    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();
}

/* ---- Greedy dominating set ---- */

QVector<int> DominatingSet5::greedyDominatingSet()
{
    QElapsedTimer timer;
    timer.start();

    QVector<bool> dominated(m_n, false);
    QVector<int> result;

    // Greedy: pick vertex covering most undominated vertices
    while (true) {
        int best = -1;
        int bestGain = 0;

        for (int v = 0; v < m_n; ++v) {
            if (dominated[v] && !m_adj[v].isEmpty()) {
                // Count undominated neighbors + self
                int gain = 0;
                if (!dominated[v]) gain++;
                for (int u : m_adj[v])
                    if (!dominated[u]) gain++;
                if (gain > bestGain) { bestGain = gain; best = v; }
            } else {
                int gain = 1;
                for (int u : m_adj[v])
                    if (!dominated[u]) gain++;
                if (gain > bestGain) { bestGain = gain; best = v; }
            }
        }

        if (bestGain == 0) break;
        result.append(best);
        dominated[best] = true;
        for (int u : m_adj[best])
            dominated[u] = true;
    }

    m_stats.totalRuns++;
    m_stats.dominatingSize = result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalRuns);

    emit dominatingSetFound(result.size(), 0.0, timer.elapsed());
    return result;
}

/* ---- LP relaxation lower bound ---- */

double DominatingSet5::lpRelaxationBound()
{
    QElapsedTimer timer;
    timer.start();

    // LP: minimize sum(x_v) subject to x_v + sum(x_u for u in N(v)) >= 1
    // Solve with greedy fractional relaxation
    QVector<double> x(m_n, 0.0);
    QVector<double> coverage(m_n, 0.0);

    // Iteratively raise fractional variables until all covered
    for (int iter = 0; iter < m_n * 2; ++iter) {
        // Find least-covered vertex
        int worst = -1;
        double worstCov = 2.0;
        for (int v = 0; v < m_n; ++v) {
            double cov = x[v];
            for (int u : m_adj[v]) cov += x[u];
            if (cov < 1.0 - 1e-9 && cov < worstCov) {
                worstCov = cov;
                worst = v;
            }
        }
        if (worst == -1) break;

        // Distribute coverage among v and its neighbors
        double deficit = 1.0 - worstCov;
        int count = 1 + m_adj[worst].size();
        double inc = deficit / count;
        x[worst] += inc;
        for (int u : m_adj[worst])
            x[u] += inc;
    }

    double bound = 0.0;
    for (int v = 0; v < m_n; ++v)
        bound += x[v];

    m_stats.lpBound = bound;
    m_stats.totalRuns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalRuns);

    return bound;
}

/* ---- Find critical vertices ---- */

QVector<int> DominatingSet5::findCriticalVertices() const
{
    QVector<int> critical;
    for (int v = 0; v < m_n; ++v) {
        // A vertex is critical if it has a neighbor that ONLY it can dominate
        for (int u : m_adj[v]) {
            if (m_adj[u].size() == 1) {
                critical.append(v);
                break;
            }
        }
    }
    return critical;
}

/* ---- Critical vertex dominating set ---- */

QVector<int> DominatingSet5::criticalVertexSet()
{
    QElapsedTimer timer;
    timer.start();

    QVector<bool> dominated(m_n, false);
    QVector<bool> inSet(m_n, false);
    QVector<int> result;

    // Phase 1: Include all critical vertices
    QVector<int> critical = findCriticalVertices();
    for (int v : critical) {
        if (!inSet[v]) {
            result.append(v);
            inSet[v] = true;
            dominated[v] = true;
            for (int u : m_adj[v])
                dominated[u] = true;
        }
    }

    // Phase 2: Greedy fill for remaining
    while (true) {
        int best = -1;
        int bestGain = 0;
        for (int v = 0; v < m_n; ++v) {
            if (inSet[v]) continue;
            int gain = dominated[v] ? 0 : 1;
            for (int u : m_adj[v])
                if (!dominated[u]) gain++;
            if (gain > bestGain) { bestGain = gain; best = v; }
        }
        if (bestGain == 0) break;
        result.append(best);
        inSet[best] = true;
        dominated[best] = true;
        for (int u : m_adj[best])
            dominated[u] = true;
    }

    m_stats.dominatingSize = result.size();
    m_stats.totalRuns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalRuns);

    emit dominatingSetFound(result.size(), m_stats.lpBound, timer.elapsed());
    return result;
}

/* ---- Verify dominating set ---- */

bool DominatingSet5::isDominating(const QVector<int>& candidate) const
{
    QVector<bool> dominated(m_n, false);
    for (int v : candidate) {
        if (v >= 0 && v < m_n) {
            dominated[v] = true;
            for (int u : m_adj[v])
                dominated[u] = true;
        }
    }
    for (int i = 0; i < m_n; ++i)
        if (!dominated[i]) return false;
    return true;
}

/* ---- Reset ---- */

void DominatingSet5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_degree.clear();
    m_n = 0;
}
