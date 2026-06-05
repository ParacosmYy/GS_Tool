/**
 * @file DijkstraShortestPath.cpp
 * @brief Dijkstra最短路径实现 — 优先队列
 */

#include "utils/dijkstra/DijkstraShortestPath.h"

#include <QElapsedTimer>

#include <queue>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DijkstraShortestPath::DijkstraShortestPath(QObject* parent)
    : QObject(parent)
    , m_numNodes(0)
    , m_directed(true)
{
}

/** @brief 设置图 @param numNodes 节点数 @param directed 是否有向 */
void DijkstraShortestPath::setGraph(int numNodes, bool directed)
{
    m_numNodes = std::max(0, numNodes);
    m_directed = directed;
    m_adjList.clear();
    m_adjList.resize(m_numNodes);
}

/** @brief 添加边 @param from 起点 @param to 终点 @param weight 权重 */
void DijkstraShortestPath::addEdge(int from, int to, Weight weight)
{
    if (from < 0 || from >= m_numNodes || to < 0 || to >= m_numNodes) return;
    if (weight < 0) return;

    m_adjList[from].append({to, weight});
    if (!m_directed) {
        m_adjList[to].append({from, weight});
    }
}

/** @brief 重建路径 @param predecessors 前驱数组 @param target 目标 @return 路径 */
QVector<int> DijkstraShortestPath::reconstructPath(
    const QVector<int>& predecessors, int target) const
{
    QVector<int> path;
    if (target < 0 || target >= m_numNodes) return path;
    if (predecessors[target] == -1 && target != predecessors.indexOf(0)) {
        /* 检查是否是源节点自身 */
        return path;
    }

    int cur = target;
    while (cur != -1) {
        path.prepend(cur);
        cur = predecessors[cur];
    }
    return path;
}

/** @brief 单源最短路径 @param source 源 @param target 目标 @return 路径结果 */
DijkstraShortestPath::PathResult DijkstraShortestPath::shortestPath(
    int source, int target)
{
    PathResult result;
    if (m_numNodes == 0 || source < 0 || source >= m_numNodes) {
        result.reachable = false;
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    /* 初始化距离和前驱 */
    result.distances.resize(m_numNodes, INF);
    result.predecessors.resize(m_numNodes, -1);
    result.distances[source] = 0;

    /* 优先队列: (距离, 节点) */
    using PQEntry = QPair<Weight, int>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> pq;
    pq.push({0.0, source});

    QVector<bool> visited(m_numNodes, false);
    quint64 edgesProcessed = 0;
    quint64 nodesRelaxed = 0;

    while (!pq.empty()) {
        auto [dist, u] = pq.top();
        pq.pop();

        /* 如果到达目标，提前退出 */
        if (u == target) break;

        if (visited[u]) continue;
        visited[u] = true;
        ++nodesRelaxed;

        /* 松弛所有出边 */
        for (const auto& [v, w] : m_adjList[u]) {
            ++edgesProcessed;
            Weight newDist = dist + w;
            if (newDist < result.distances[v]) {
                result.distances[v] = newDist;
                result.predecessors[v] = u;
                pq.push({newDist, v});
            }
        }
    }

    /* 构建结果 */
    if (target >= 0 && target < m_numNodes) {
        result.totalDistance = result.distances[target];
        result.reachable = (result.distances[target] < INF);
        result.path = reconstructPath(result.predecessors, target);
    } else {
        result.totalDistance = 0;
        result.reachable = true;
        result.path.clear();
    }

    /* 更新统计 */
    ++m_stats.totalQueries;
    m_stats.totalNodesRelaxed += nodesRelaxed;
    m_stats.totalEdgesProcessed += edgesProcessed;
    m_stats.totalGraphSize += static_cast<quint64>(m_numNodes);
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalQueries);

    emit pathComputed(result);
    return result;
}

/** @brief 所有节点对最短路径 @return 距离矩阵 */
QVector<QVector<DijkstraShortestPath::Weight>> DijkstraShortestPath::allPairsShortest()
{
    QVector<QVector<Weight>> distMatrix;
    distMatrix.reserve(m_numNodes);

    for (int s = 0; s < m_numNodes; ++s) {
        PathResult r = shortestPath(s);
        distMatrix.append(r.distances);
    }

    return distMatrix;
}

/** @brief 重置统计 */
void DijkstraShortestPath::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
