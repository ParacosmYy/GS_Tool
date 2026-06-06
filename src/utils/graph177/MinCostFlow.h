/**
 * @file MinCostFlow.h
 * @brief 最小费用最大流(势函数最短增广路) — Min-Cost Max-Flow via Successive Shortest Paths with Potentials
 *
 * 功能: 实现基于势函数Johnson改权的连续最短路最小费用最大流算法。
 *       使用Bellman-Ford初始化势函数，Dijkstra寻找增广路。
 *       支持查询流量分配和总费用。
 *
 * 协作: MaxFlow(最大流) / ShortestPath(最短路) / NetworkSimplex(网络单纯形)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最小费用最大流求解器
 */
class MinCostFlow : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalAugmentations = 0;     ///< 累计增广次数
        double totalCost = 0.0;             ///< 最小总费用
        double totalFlow = 0.0;             ///< 最大流
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit MinCostFlow(QObject* parent = nullptr);
    ~MinCostFlow() override;

    /**
     * @brief 设置图的节点数
     * @param n 节点数(0-indexed)
     */
    void setNodeCount(int n);

    /**
     * @brief 添加有向边
     * @param from 起点
     * @param to 终点
     * @param capacity 容量
     * @param cost 单位费用
     */
    void addEdge(int from, int to, double capacity, double cost);

    /**
     * @brief 求解最小费用最大流
     * @param source 源点
     * @param sink 汇点
     * @return QPair<最大流, 最小费用>
     */
    QPair<double, double> solve(int source, int sink);

    /**
     * @brief 获取边的实际流量
     * @return 每条原始边的流量
     */
    QVector<QPair<QPair<int, int>, double>> flowEdges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

    /** @brief 清空图 */
    void clear();

signals:
    /** @brief 求解完成 @param flow 最大流 @param cost 最小费用 */
    void solveCompleted(double flow, double cost);

private:
    /** @brief 边结构(邻接表) */
    struct Edge {
        int to;             ///< 终点
        double capacity;    ///< 残余容量
        double cost;        ///< 单位费用
        int rev;            ///< 反向边在邻接表中的索引
        double flow;        ///< 已流过的流量
    };

    /** @brief Dijkstra找最短增广路(使用势函数) */
    bool dijkstra(int source, int sink, QVector<int>& parent, QVector<int>& parentEdge,
                  const QVector<double>& potential);

    /** @brief Bellman-Ford初始化势函数(处理负权边) */
    QVector<double> bellmanFord(int source) const;

    int m_nodeCount = 0;
    QVector<QVector<Edge>> m_graph;      ///< 邻接表
    int m_originalEdgeCount = 0;         ///< 原始边数(不含反向边)

    Stats m_stats;
    double m_timeSum = 0.0;
};
