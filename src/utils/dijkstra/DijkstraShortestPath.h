/**
 * @file DijkstraShortestPath.h
 * @brief Dijkstra最短路径 — 基于优先队列的单源最短路径算法
 *
 * 功能: 实现带权有向图/无向图的单源最短路径计算，
 *       支持路径重建、多源最短路径、距离矩阵计算。
 *
 * 协作: DataCorrelator(相关性分析) / StateTracker(状态图)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

#include <vector>
#include <limits>

/**
 * @brief Dijkstra最短路径 — 优先队列实现
 */
class DijkstraShortestPath : public QObject {
    Q_OBJECT

public:
    /** @brief 边权重类型 */
    using Weight = double;

    /** @brief 边: (目标节点, 权重) */
    using Edge = QPair<int, Weight>;

    /** @brief 邻接表类型 */
    using AdjList = QVector<QVector<Edge>>;

    /** @brief 路径结果 */
    struct PathResult {
        QVector<Weight>  distances;    ///< 源点到各节点距离
        QVector<int>     predecessors; ///< 前驱节点(-1=无)
        Weight           totalDistance; ///< 目标最短距离
        QVector<int>     path;         ///< 源到目标的最短路径
        bool             reachable;    ///< 目标是否可达
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries         = 0;   ///< 累计查询次数
        quint64 totalNodesRelaxed    = 0;   ///< 累计松弛节点数
        quint64 totalEdgesProcessed  = 0;   ///< 累计处理边数
        double  avgProcessingTimeMs  = 0.0; ///< 平均处理时间(ms)
        quint64 totalGraphSize       = 0;   ///< 累计图节点数
    };

    explicit DijkstraShortestPath(QObject* parent = nullptr);

    /**
     * @brief 设置图(邻接表)
     * @param numNodes 节点数
     * @param directed 是否有向图
     */
    void setGraph(int numNodes, bool directed = true);

    /**
     * @brief 添加边
     * @param from 起点
     * @param to 终点
     * @param weight 权重(必须≥0)
     */
    void addEdge(int from, int to, Weight weight);

    /**
     * @brief 计算单源最短路径
     * @param source 源节点
     * @param target 目标节点(-1=计算到所有节点)
     * @return 路径结果
     */
    PathResult shortestPath(int source, int target = -1);

    /**
     * @brief 计算所有节点对的最短路径
     * @return 距离矩阵 [i][j] = i到j的距离
     */
    QVector<QVector<Weight>> allPairsShortest();

    /**
     * @brief 获取节点数
     * @return 节点数
     */
    int nodeCount() const { return m_numNodes; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最短路径计算完成 @param result 路径结果 */
    void pathComputed(const PathResult& result);

private:
    /**
     * @brief 从前驱数组重建路径
     * @param predecessors 前驱数组
     * @param target 目标节点
     * @return 路径节点序列
     */
    QVector<int> reconstructPath(const QVector<int>& predecessors, int target) const;

    int     m_numNodes;       ///< 节点数
    bool    m_directed;       ///< 是否有向图
    AdjList m_adjList;        ///< 邻接表

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;

    static constexpr Weight INF = std::numeric_limits<Weight>::infinity();
};
