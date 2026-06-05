/**
 * @file DijkstraShortestPath.h
 * @brief Dijkstra最短路径 — 加权图单源最短路径
 *
 * 功能: Dijkstra算法求解单源最短路径，支持路径重建、
 *       多目标查询，统计查询次数/路径总长/耗时。
 */
#ifndef DIJKSTRASHORTESTPATH_H
#define DIJKSTRASHORTESTPATH_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPair>

class DijkstraShortestPath : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalQueries = 0;
        quint64 totalNodesVisited = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit DijkstraShortestPath(QObject* parent = nullptr);

    /** @brief 添加带权边 @param from 起点 @param to 终点 @param weight 权重 */
    void addEdge(int from, int to, double weight);

    /** @brief 添加双向边 @param u 端点1 @param v 端点2 @param weight 权重 */
    void addBidirectionalEdge(int u, int v, double weight);

    /** @brief 计算最短路径 @param source 起点 @param target 终点 @return {路径, 总距离} */
    QPair<QVector<int>, double> shortestPath(int source, int target);

    /** @brief 计算到所有节点的距离 @param source 起点 @return node→distance */
    QMap<int, double> shortestDistances(int source);

    /** @brief 检查路径是否存在 @param source 起点 @param target 终点 @return 是否可达 */
    bool hasPath(int source, int target);

    void clear();
    int nodeCount() const { return m_adj.size(); }
    int edgeCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pathFound(int source, int target, double distance, int hops);

private:
    QMap<int, QVector<QPair<int, double>>> m_adj; ///< 邻接表
    Stats m_stats;
    double m_timeSum;
};

#endif // DIJKSTRASHORTESTPATH_H
