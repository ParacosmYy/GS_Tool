/**
 * @file GraphAnalyzer.h
 * @brief 图/网络分析器 — 遍历、最短路径与连通分量
 *
 * 功能: 基于邻接表表示的图结构，支持BFS/DFS遍历、
 *       Dijkstra最短路径、连通分量检测，用于网络拓扑分析。
 *
 * 协作: DataPipeline(依赖分析) / QueueSimulator(网络流量)
 */
#ifndef GRAPHANALYZER_H
#define GRAPHANALYZER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @class GraphAnalyzer
 * @brief 图/网络分析工具
 */
class GraphAnalyzer : public QObject {
    Q_OBJECT

public:
    /** 分析统计 */
    struct Stats {
        quint64 totalAnalyses = 0;             ///< 总分析次数
        quint64 totalNodesProcessed = 0;       ///< 总处理节点数
        double  avgProcessingTime = 0.0;       ///< 平均处理时间(ms)
    };

    /** 图节点ID类型 */
    using NodeId = int;

    /** 带权边 */
    struct Edge {
        NodeId target = -1;                    ///< 目标节点
        double weight = 1.0;                   ///< 边权重
    };

    /** 路径结果 */
    struct PathResult {
        QVector<NodeId> path;                  ///< 路径节点序列
        double totalWeight = 0.0;              ///< 路径总权重
        bool found = false;                    ///< 是否找到路径
    };

    /** 连通分量结果 */
    struct ComponentResult {
        QVector<QVector<NodeId>> components;   ///< 各连通分量
        int largestComponentSize = 0;          ///< 最大分量大小
        int componentCount = 0;                ///< 分量总数
    };

    explicit GraphAnalyzer(QObject* parent = nullptr);

    /** @brief 添加节点 @param id 节点ID */
    void addNode(NodeId id);

    /** @brief 添加有向边 @param from 起点 @param to 终点 @param weight 权重 */
    void addEdge(NodeId from, NodeId to, double weight = 1.0);

    /** @brief 添加无向边 @param a 端点A @param b 端点B @param weight 权重 */
    void addUndirectedEdge(NodeId a, NodeId b, double weight = 1.0);

    /** @brief 清空图 */
    void clear();

    /** @brief 获取节点数 @return 节点总数 */
    int nodeCount() const;

    /** @brief 获取边数 @return 边总数 */
    int edgeCount() const;

    /** @brief 广度优先搜索 @param start 起始节点 @return 遍历序列 */
    QVector<NodeId> bfs(NodeId start);

    /** @brief 深度优先搜索 @param start 起始节点 @return 遍历序列 */
    QVector<NodeId> dfs(NodeId start);

    /** @brief Dijkstra最短路径 @param start 起点 @param end 终点 @return 路径结果 */
    PathResult dijkstra(NodeId start, NodeId end);

    /** @brief 检测连通分量 @return 分量结果 */
    ComponentResult connectedComponents();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分析完成 @param analysisType 分析类型 */
    void analysisCompleted(const QString& analysisType);

    /** @brief 路径计算完成 @param result 路径结果 */
    void pathFound(const PathResult& result);

private:
    QMap<NodeId, QVector<Edge>> m_adjacency;   ///< 邻接表
    int m_edgeCount;                           ///< 边计数

    Stats m_stats;
    double m_timeSum;                          ///< 处理时间累计
};

#endif // GRAPHANALYZER_H
