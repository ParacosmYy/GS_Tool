/**
 * @file StronglyConnected.cpp
 * @brief 强连通分量(Tarjan)实现
 */

#include "StronglyConnected.h"
#include <QElapsedTimer>

StronglyConnected::StronglyConnected(QObject* parent)
    : QObject(parent)
    , m_numNodes(0)
    , m_currentIndex(0)
    , m_compCount(0)
    , m_timeSum(0.0)
{
}

void StronglyConnected::setGraph(const QVector<QVector<int>>& adj, int numNodes)
{
    m_adj = adj;
    m_numNodes = numNodes;
    m_index.fill(-1, numNodes);
    m_lowlink.fill(0, numNodes);
    m_onStack.fill(false, numNodes);
    m_compId.fill(-1, numNodes);
    m_stack.clear();
    m_components.clear();
    m_currentIndex = 0;
    m_compCount = 0;
}

void StronglyConnected::tarjanDFS(int v)
{
    m_index[v] = m_currentIndex;
    m_lowlink[v] = m_currentIndex;
    m_currentIndex++;
    m_stack.append(v);
    m_onStack[v] = true;

    for (int w : m_adj[v]) {
        if (w < 0 || w >= m_numNodes) continue;

        if (m_index[w] == -1) {
            tarjanDFS(w);
            m_lowlink[v] = qMin(m_lowlink[v], m_lowlink[w]);
        } else if (m_onStack[w]) {
            m_lowlink[v] = qMin(m_lowlink[v], m_index[w]);
        }
    }

    /* 如果v是SCC的根 */
    if (m_lowlink[v] == m_index[v]) {
        QVector<int> component;
        int w;
        do {
            w = m_stack.last();
            m_stack.removeLast();
            m_onStack[w] = false;
            m_compId[w] = m_compCount;
            component.append(w);
        } while (w != v);

        m_components.append(component);
        m_compCount++;
    }
}

QVector<QVector<int>> StronglyConnected::findComponents()
{
    QElapsedTimer timer;
    timer.start();

    m_index.fill(-1, m_numNodes);
    m_lowlink.fill(0, m_numNodes);
    m_onStack.fill(false, m_numNodes);
    m_compId.fill(-1, m_numNodes);
    m_stack.clear();
    m_components.clear();
    m_currentIndex = 0;
    m_compCount = 0;

    for (int v = 0; v < m_numNodes; ++v) {
        if (m_index[v] == -1)
            tarjanDFS(v);
    }

    m_stats.totalSearches++;
    m_stats.totalComponentsFound += m_compCount;
    m_timeSum += timer.elapsed();
    if (m_stats.totalSearches > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(m_compCount);
    return m_components;
}

bool StronglyConnected::isStronglyConnected(int u, int v) const
{
    if (u < 0 || u >= m_numNodes || v < 0 || v >= m_numNodes) return false;
    return m_compId[u] != -1 && m_compId[u] == m_compId[v];
}

int StronglyConnected::componentId(int node) const
{
    if (node < 0 || node >= m_numNodes) return -1;
    return m_compId[node];
}

int StronglyConnected::componentCount() const { return m_compCount; }

QVector<QVector<int>> StronglyConnected::condensationDAG() const
{
    QVector<QVector<int>> dag(m_compCount);

    for (int u = 0; u < m_numNodes; ++u) {
        int cu = m_compId[u];
        if (cu < 0) continue;
        for (int v : m_adj[u]) {
            if (v < 0 || v >= m_numNodes) continue;
            int cv = m_compId[v];
            if (cv < 0 || cv == cu) continue;

            /* 去重 */
            if (!dag[cu].contains(cv))
                dag[cu].append(cv);
        }
    }
    return dag;
}

StronglyConnected::Stats StronglyConnected::stats() const { return m_stats; }

void StronglyConnected::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
