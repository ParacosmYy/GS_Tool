/**
 * @file GraphAnalyzer.cpp
 * @brief 图/网络分析器实现
 */

#include "utils/graph/GraphAnalyzer.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>
#include <queue>
#include <stack>
#include <set>
#include <algorithm>

GraphAnalyzer::GraphAnalyzer(QObject* parent)
    : QObject(parent)
    , m_edgeCount(0)
    , m_timeSum(0.0)
{}

void GraphAnalyzer::addNode(NodeId id)
{
    if (!m_adjacency.contains(id)) {
        m_adjacency[id] = QVector<Edge>();
    }
}

void GraphAnalyzer::addEdge(NodeId from, NodeId to, double weight)
{
    addNode(from);
    addNode(to);
    m_adjacency[from].append({to, weight});
    ++m_edgeCount;
}

void GraphAnalyzer::addUndirectedEdge(NodeId a, NodeId b, double weight)
{
    addEdge(a, b, weight);
    addEdge(b, a, weight);
    /* 无向边只算一条 */
    --m_edgeCount;
}

void GraphAnalyzer::clear()
{
    m_adjacency.clear();
    m_edgeCount = 0;
}

int GraphAnalyzer::nodeCount() const
{
    return m_adjacency.size();
}

int GraphAnalyzer::edgeCount() const
{
    return m_edgeCount;
}

QVector<GraphAnalyzer::NodeId> GraphAnalyzer::bfs(NodeId start)
{
    QElapsedTimer timer;
    timer.start();

    QVector<NodeId> result;
    if (!m_adjacency.contains(start)) return result;

    QSet<NodeId> visited;
    std::queue<NodeId> q;
    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        NodeId current = q.front();
        q.pop();
        result.append(current);

        for (const Edge& edge : m_adjacency.value(current)) {
            if (!visited.contains(edge.target)) {
                visited.insert(edge.target);
                q.push(edge.target);
            }
        }
    }

    /* 更新统计 */
    ++m_stats.totalAnalyses;
    m_stats.totalNodesProcessed += result.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTime = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(QStringLiteral("BFS"));
    return result;
}

QVector<GraphAnalyzer::NodeId> GraphAnalyzer::dfs(NodeId start)
{
    QElapsedTimer timer;
    timer.start();

    QVector<NodeId> result;
    if (!m_adjacency.contains(start)) return result;

    QSet<NodeId> visited;
    std::stack<NodeId> s;
    s.push(start);

    while (!s.empty()) {
        NodeId current = s.top();
        s.pop();

        if (visited.contains(current)) continue;
        visited.insert(current);
        result.append(current);

        /* 逆序压栈以保持正向遍历顺序 */
        const QVector<Edge>& edges = m_adjacency.value(current);
        for (int i = edges.size() - 1; i >= 0; --i) {
            if (!visited.contains(edges[i].target)) {
                s.push(edges[i].target);
            }
        }
    }

    /* 更新统计 */
    ++m_stats.totalAnalyses;
    m_stats.totalNodesProcessed += result.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTime = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(QStringLiteral("DFS"));
    return result;
}

GraphAnalyzer::PathResult GraphAnalyzer::dijkstra(NodeId start, NodeId end)
{
    QElapsedTimer timer;
    timer.start();

    PathResult result;
    result.found = false;

    if (!m_adjacency.contains(start) || !m_adjacency.contains(end)) {
        emit pathFound(result);
        return result;
    }

    /* 距离表 */
    QMap<NodeId, double> dist;
    QMap<NodeId, NodeId> prev;
    for (auto it = m_adjacency.begin(); it != m_adjacency.end(); ++it) {
        dist[it.key()] = std::numeric_limits<double>::max();
    }
    dist[start] = 0.0;

    /* 优先队列: (距离, 节点) */
    using PQEntry = QPair<double, NodeId>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> pq;
    pq.push({0.0, start});

    QSet<NodeId> processed;

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (processed.contains(u)) continue;
        processed.insert(u);

        if (u == end) break;

        for (const Edge& edge : m_adjacency.value(u)) {
            double alt = d + edge.weight;
            if (alt < dist.value(edge.target, std::numeric_limits<double>::max())) {
                dist[edge.target] = alt;
                prev[edge.target] = u;
                pq.push({alt, edge.target});
            }
        }
    }

    /* 回溯路径 */
    if (dist.value(end, std::numeric_limits<double>::max()) < std::numeric_limits<double>::max()) {
        result.found = true;
        result.totalWeight = dist[end];
        NodeId cur = end;
        while (prev.contains(cur)) {
            result.path.prepend(cur);
            cur = prev[cur];
        }
        result.path.prepend(start);
    }

    /* 更新统计 */
    ++m_stats.totalAnalyses;
    m_stats.totalNodesProcessed += processed.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTime = m_timeSum / m_stats.totalAnalyses;

    emit pathFound(result);
    emit analysisCompleted(QStringLiteral("Dijkstra"));
    return result;
}

GraphAnalyzer::ComponentResult GraphAnalyzer::connectedComponents()
{
    QElapsedTimer timer;
    timer.start();

    ComponentResult result;
    QSet<NodeId> visited;

    for (auto it = m_adjacency.begin(); it != m_adjacency.end(); ++it) {
        NodeId start = it.key();
        if (visited.contains(start)) continue;

        /* BFS搜索连通分量 */
        QVector<NodeId> component;
        std::queue<NodeId> q;
        q.push(start);
        visited.insert(start);

        while (!q.empty()) {
            NodeId current = q.front();
            q.pop();
            component.append(current);

            for (const Edge& edge : m_adjacency.value(current)) {
                if (!visited.contains(edge.target)) {
                    visited.insert(edge.target);
                    q.push(edge.target);
                }
            }
        }

        result.components.append(component);
        if (component.size() > result.largestComponentSize) {
            result.largestComponentSize = component.size();
        }
    }

    result.componentCount = result.components.size();

    /* 更新统计 */
    ++m_stats.totalAnalyses;
    m_stats.totalNodesProcessed += visited.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTime = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(QStringLiteral("ConnectedComponents"));
    return result;
}

void GraphAnalyzer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
