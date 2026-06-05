/**
 * @file ArticulationPoint.cpp
 * @brief 关节点与桥检测器实现
 */

#include "ArticulationPoint.h"
#include <QElapsedTimer>
#include <algorithm>

ArticulationPoint::ArticulationPoint(QObject* parent)
    : QObject(parent)
    , m_time(0)
    , m_timeSum(0.0)
{
}

void ArticulationPoint::addEdge(int u, int v)
{
    ensureGraph(qMax(u, v) + 1);
    m_adj[u].append(v);
    m_adj[v].append(u);
}

QVector<int> ArticulationPoint::findArticulationPoints()
{
    findAll();
    QVector<int> result;
    for (int i = 0; i < m_isArticulation.size(); ++i)
        if (m_isArticulation[i]) result.append(i);
    return result;
}

QVector<QPair<int, int>> ArticulationPoint::findBridges()
{
    findAll();
    return m_bridges;
}

void ArticulationPoint::findAll()
{
    QElapsedTimer timer;
    timer.start();

    int n = m_adj.size();
    if (n == 0) return;

    m_disc.assign(n, -1);
    m_low.assign(n, 0);
    m_visited.assign(n, false);
    m_isArticulation.assign(n, false);
    m_bridges.clear();
    m_time = 0;

    for (int i = 0; i < n; ++i) {
        if (!m_visited[i]) dfs(i, -1);
    }

    m_stats.totalSearches++;
    int apCount = 0;
    for (bool b : m_isArticulation) if (b) apCount++;
    m_stats.totalArticulations += apCount;
    m_stats.totalBridges += m_bridges.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(apCount, m_bridges.size());
}

QVector<int> ArticulationPoint::articulationPoints() const
{
    QVector<int> result;
    for (int i = 0; i < m_isArticulation.size(); ++i)
        if (m_isArticulation[i]) result.append(i);
    return result;
}

QVector<QPair<int, int>> ArticulationPoint::bridges() const
{
    return m_bridges;
}

void ArticulationPoint::reset()
{
    m_adj.clear();
    m_disc.clear();
    m_low.clear();
    m_visited.clear();
    m_isArticulation.clear();
    m_bridges.clear();
    m_time = 0;
}

void ArticulationPoint::dfs(int u, int parent)
{
    m_visited[u] = true;
    m_disc[u] = m_low[u] = m_time++;
    int children = 0;

    for (int v : m_adj[u]) {
        if (v == parent) continue;

        if (m_visited[v]) {
            m_low[u] = qMin(m_low[u], m_disc[v]);
        } else {
            dfs(v, u);
            m_low[u] = qMin(m_low[u], m_low[v]);
            children++;

            /* 关节点判定 */
            if (parent == -1 && children > 1)
                m_isArticulation[u] = true;
            if (parent != -1 && m_low[v] >= m_disc[u])
                m_isArticulation[u] = true;

            /* 桥判定 */
            if (m_low[v] > m_disc[u])
                m_bridges.append({qMin(u, v), qMax(u, v)});
        }
    }
}

void ArticulationPoint::ensureGraph(int n)
{
    if (m_adj.size() < n) m_adj.resize(n);
}

ArticulationPoint::Stats ArticulationPoint::stats() const { return m_stats; }

void ArticulationPoint::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
