/**
 * @file MinCostMaxFlow.h
 * @brief 最小费用最大流 — SPFA增广/费用缩放/对偶变量
 *
 * 功能: 在流网络中同时最大化流量和最小化费用，支持SPFA残量图
 *       最短路增广、费用缩放策略、对偶变量维护。
 *
 * 协作: DataFlowMeter(流控) / DataSynchronizer(同步调度)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 最小费用最大流 — 流网络优化引擎
 */
class MinCostMaxFlow : public QObject {
    Q_OBJECT

public:
    /** @brief 边结构 */
    struct Edge {
        int to = -1;            ///< 目标顶点
        int rev = -1;           ///< 反向边索引
        double capacity = 0.0;  ///< 容量
        double cost = 0.0;      ///< 单位费用
    };

    /** @brief 流结果 */
    struct FlowResult {
        double totalFlow = 0.0;     ///< 总流量
        double totalCost = 0.0;     ///< 总费用
        int augmentations = 0;      ///< 增广次数
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalAugmentations = 0;     ///< 累计增广次数
        quint64 totalEdgesProcessed = 0;    ///< 累计处理边数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  bestCostEfficiency = 0.0;   ///< 历史最佳费用效率
    };

    explicit MinCostMaxFlow(QObject* parent = nullptr);

    /**
     * @brief 初始化图
     * @param n 顶点数
     */
    void init(int n);

    /**
     * @brief 添加边
     * @param from 源顶点
     * @param to 目标顶点
     * @param capacity 容量
     * @param cost 单位费用
     */
    void addEdge(int from, int to, double capacity, double cost);

    /**
     * @brief 依次最短路增广求解最小费用最大流
     * @param source 源点
     * @param sink 汇点
     * @param maxFlow 最大流量限制
     * @return 流结果
     */
    FlowResult successiveShortestPath(int source, int sink,
                                      double maxFlow = 1e18);

    /**
     * @brief SPFA在残量图上求最短路
     * @param source 源点
     * @param sink 汇点
     * @param n 顶点数
     * @return (距离数组, 前驱节点数组)
     */
    QPair<QVector<double>, QVector<int>> spfa(int source, int sink, int n);

    /**
     * @brief 费用缩放求解(近似)
     * @param source 源点
     * @param sink 汇点
     * @param epsilon 精度参数
     * @return 流结果
     */
    FlowResult costScaling(int source, int sink, double epsilon = 1e-6);

    /**
     * @brief 获取对偶变量(势函数)
     * @return 各顶点的势值
     */
    QVector<double> dualVariables() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param flow 总流量 @param cost 总费用 */
    void solveComplete(double flow, double cost);

    /** @brief 增广完成 @param amount 增广量 @param cost 增广费用 */
    void augmentDone(double amount, double cost);

private:
    double augmentAlongPath(int source, int sink,
                            const QVector<int>& parent);

    QVector<QVector<Edge>> m_graph;    ///< 邻接表
    QVector<double> m_potential;       ///< 对偶变量(势函数)
    int m_vertexCount;                 ///< 顶点数

    Stats m_stats;
    double m_timeSum = 0.0;     ///< 处理时间累加器
};
