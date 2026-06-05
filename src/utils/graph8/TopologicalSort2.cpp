/**
 * @file TopologicalSort2.cpp
 * @brief Kahn拓扑排序实现
 */

#include "TopologicalSort2.h"
#include <QElapsedTimer>
#include <QQueue>

TopologicalSort2::TopologicalSort2(QObject* parent)
    : QObject(parent)
    , m_numNodes(0)
    , m_lastHadCycle(false)
    , m_timeSum(0.0)
{
}

void TopologicalSort2::setGraph(const QVector<QVector<int>>& adj, int numNodes)
{
    m_adj = adj;
    m_numNodes = numNodes;
}

void TopologicalSort2::addEdge(int from, int to)
{
    int maxNode = qMax(from, to) + 1;
    if (maxNode > m_numNodes) {
        m_adj.resize(maxNode);
        m_numNodes = maxNode;
    }
    m_adj[from].append(to);
}

QVector<int> TopologicalSort2::sort()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> inDegree(m_numNodes, 0);
    for (int u = 0; u < m_numNodes; ++u) {
        for (int v : m_adj[u])
            inDegree[v]++;
    }

    QQueue<int> queue;
    for (int i = 0; i < m_numNodes; ++i) {
        if (inDegree[i] == 0)
            queue.enqueue(i);
    }

    QVector<int> result;
    result.reserve(m_numNodes);

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        result.append(u);

        for (int v : m_adj[u]) {
            inDegree[v]--;
            if (inDegree[v] == 0)
                queue.enqueue(v);
        }
    }

    m_lastHadCycle = (result.size() != m_numNodes);

    m_stats.totalSorts++;
    if (m_lastHadCycle) m_stats.cyclesDetected++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(result.size(), m_lastHadCycle);
    return result;
}

bool TopologicalSort2::hasCycle() const { return m_lastHadCycle; }

TopologicalSort2::Stats TopologicalSort2::stats() const { return m_stats; }

void TopologicalSort2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
