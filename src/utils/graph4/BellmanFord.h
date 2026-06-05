/**
 * @file BellmanFord.h
 * @brief Bellman-Ford最短路径算法 — 支持负权边
 *
 * 功能: 计算单源最短路径，可处理负权边并检测负权环。
 *       时间复杂度O(VE)，比Dijkstra慢但适用范围更广。
 *
 * 协作: DijkstraShortestPath(非负权) / FloydWarshall(全源)
 */
#ifndef BELLMANFORD_H
#define BELLMANFORD_H

#include <QObject>
#include <QVector>

/**
 * @brief Bellman-Ford最短路径
 */
class BellmanFord : public QObject {
    Q_OBJECT

public:
    /** @brief 边 */
    struct Edge {
        int from = 0;     ///< 起点
        int to = 0;       ///< 终点
        double weight = 0.0; ///< 权重
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries = 0;        ///< 累计查询次数
        quint64 negativeCyclesFound = 0; ///< 发现负权环次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 查询结果 */
    struct PathResult {
        QVector<double> distances;       ///< 最短距离
        QVector<int> predecessors;       ///< 前驱节点
        bool hasNegativeCycle = false;   ///< 是否有负权环
    };

    explicit BellmanFord(QObject* parent = nullptr);

    /** @brief 计算单源最短路径
     *  @param vertexCount 顶点数
     *  @param edges 边列表
     *  @param source 源点
     *  @return 路径结果 */
    PathResult shortestPaths(int vertexCount,
                             const QVector<Edge>& edges,
                             int source);

    /** @brief 检测负权环
     *  @param vertexCount 顶点数
     *  @param edges 边列表
     *  @return 是否存在负权环 */
    bool hasNegativeCycle(int vertexCount,
                          const QVector<Edge>& edges);

    /** @brief 回溯最短路径
     *  @param result 查询结果
     *  @param target 目标点
     *  @return 路径节点序列 */
    QVector<int> reconstructPath(const PathResult& result,
                                 int target) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 查询完成 @param source 源点 @param hasNegCycle 负权环 */
    void queryCompleted(int source, bool hasNegCycle);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // BELLMANFORD_H
