/**
 * @file MinimumCostFlow.h
 * @brief 最小费用流算法 — 逐次最短路径实现
 *
 * 功能: 在带容量和单位费用的网络中求解最小费用最大流问题，
 *       使用逐次最短增广路径算法 (Successive Shortest Paths)，
 *       支持 SPFA 负边费用处理和残量网络维护。
 *
 * 协作: 串口数据路由优化 / 协议桥调度 / 资源分配
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 最小费用流求解器
 *
 * 给定一个有向网络 (N 个节点, M 条边)，每条边有容量上限和单位费用，
 * 计算从源点 s 到汇点 t 的最大流中费用最小的方案。
 */
class MinimumCostFlow : public QObject {
    Q_OBJECT

public:
    /** @brief 边定义 */
    struct Edge {
        int to = 0;             ///< 目标节点
        int capacity = 0;       ///< 容量
        int cost = 0;           ///< 单位费用
        int rev = 0;            ///< 反向边在邻接表中的索引
    };

    /** @brief 求解结果 */
    struct FlowResult {
        int totalFlow = 0;      ///< 总流量
        int totalCost = 0;      ///< 总费用
        bool feasible = false;  ///< 是否存在可行流
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSolves = 0;            ///< 累计求解次数
        int totalAugments = 0;          ///< 累计增广次数
        int totalNodes = 0;             ///< 累计节点数
        int totalEdges = 0;             ///< 累计边数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit MinimumCostFlow(QObject* parent = nullptr);

    /**
     * @brief 初始化网络
     * @param nodeCount 节点数量 (节点编号 0 ~ nodeCount-1)
     */
    void init(int nodeCount);

    /**
     * @brief 添加有向边
     * @param from 起点
     * @param to 终点
     * @param capacity 容量
     * @param cost 单位费用
     */
    void addEdge(int from, int to, int capacity, int cost);

    /**
     * @brief 求解最小费用最大流
     * @param source 源点
     * @param sink 汇点
     * @param maxFlow 限制最大流量 (默认 INT_MAX)
     * @return 流量与费用的结果
     */
    FlowResult solve(int source, int sink, int maxFlow = 0x7FFFFFFF);

    /**
     * @brief 获取指定边的流量
     * @param from 起点
     * @param edgeIndex 边在邻接表中的索引
     * @return 当前流量
     */
    int getFlow(int from, int edgeIndex) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param flow 总流量 @param cost 总费用 */
    void solved(int flow, int cost);

private:
    bool spfa(int source, int sink, QVector<int>& dist,
              QVector<int>& parentNode, QVector<int>& parentEdge);

    QVector<QList<Edge>> m_graph;    ///< 邻接表 (残量网络)
    int m_nodeCount;                  ///< 节点数量

    Stats m_stats;                    ///< 统计信息
    double m_timeSum = 0.0;           ///< 处理时间累加器
};
