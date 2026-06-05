/**
 * @file TopologicalSorter.cpp
 * @brief 拓扑排序器实现
 */

#include "utils/graph2/TopologicalSorter.h"

#include <QElapsedTimer>
#include <QQueue>
#include <QSet>

TopologicalSorter::TopologicalSorter(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

void TopologicalSorter::addEdge(int from, int to)
{
    m_nodes.insert(from);
    m_nodes.insert(to);
    m_adj[from].append(to);
    m_inDegree[to]++;
    if (!m_inDegree.contains(from)) m_inDegree[from] = 0;
}

void TopologicalSorter::addNode(int node)
{
    m_nodes.insert(node);
    if (!m_inDegree.contains(node)) m_inDegree[node] = 0;
}

TopologicalSorter::SortResult TopologicalSorter::sort()
{
    QElapsedTimer timer;
    timer.start();

    SortResult result;

    /* 复制入度表 */
    QMap<int, int> inDeg = m_inDegree;

    /* 入度为0的节点入队 */
    QQueue<int> queue;
    for (int node : m_nodes) {
        if (inDeg.value(node, 0) == 0) {
            queue.enqueue(node);
        }
    }

    /* BFS: Kahn算法 */
    while (!queue.isEmpty()) {
        int node = queue.dequeue();
        result.order.append(node);

        for (int neighbor : m_adj.value(node, QVector<int>())) {
            inDeg[neighbor]--;
            if (inDeg[neighbor] == 0) {
                queue.enqueue(neighbor);
            }
        }
    }

    /* 检测环 */
    if (result.order.size() < m_nodes.size()) {
        result.hasCycle = true;
        QSet<int> visited;
        for (int node : m_nodes) {
            if (!visited.contains(node)) {
                /* 找环: DFS回溯 */
                QSet<int> onStack;
                QVector<int> path;
                QMap<int, int> tempDeg = m_inDegree;

                /* 未入序的节点即在环中 */
                QSet<int> inOrder;
                for (int n : result.order) inOrder.insert(n);
                for (int n : m_nodes) {
                    if (!inOrder.contains(n)) result.cycleNodes.append(n);
                }
                break;
            }
        }
        m_stats.totalCyclesDetected++;
        emit cycleDetected(result.cycleNodes);
    }

    m_stats.totalSorts++;
    m_stats.totalNodes = m_nodes.size();
    m_stats.totalEdges = edgeCount();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSorts, 1ULL);

    emit sortCompleted(m_nodes.size(), result.hasCycle);
    return result;
}

bool TopologicalSorter::hasCycle()
{
    return sort().hasCycle;
}

QVector<QVector<int>> TopologicalSorter::computeLevels() const
{
    QVector<QVector<int>> levels;
    QMap<int, int> inDeg = m_inDegree;
    QSet<int> remaining = m_nodes;

    while (!remaining.isEmpty()) {
        QVector<int> currentLevel;
        for (int node : remaining) {
            if (inDeg.value(node, 0) == 0) {
                currentLevel.append(node);
            }
        }

        if (currentLevel.isEmpty()) break;

        for (int node : currentLevel) {
            remaining.remove(node);
            for (int neighbor : m_adj.value(node, QVector<int>())) {
                inDeg[neighbor]--;
            }
        }

        levels.append(currentLevel);
    }

    return levels;
}

void TopologicalSorter::clear()
{
    m_nodes.clear();
    m_adj.clear();
    m_inDegree.clear();
}

int TopologicalSorter::edgeCount() const
{
    int count = 0;
    for (auto it = m_adj.constBegin(); it != m_adj.constEnd(); ++it) {
        count += it.value().size();
    }
    return count;
}

void TopologicalSorter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
