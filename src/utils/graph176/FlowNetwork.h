/**
 * @file FlowNetwork.h
 * @brief 最大流网络(Edmonds-Karp) — Max-Flow via Edmonds-Karp with Residual Graph
 *
 * 功能: 实现Edmonds-Karp算法(BFS增广路)求解最大流。支持残量图管理、
 *       最小割集计算和增广路径追踪。适用于网络流分析、调度和资源分配。
 *
 * 协作: GraphDFS(图搜索) / BipartiteMatching(二分图匹配) / TopologicalSort(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QList>

/**
 * @brief Edmonds-Karp最大流网络
 */
class FlowNetwork : public QObject {
    Q_OBJECT

public:
    /** @brief 流统计 */
    struct Stats {
        quint64 totalFlows = 0;             ///< 累计最大流计算次数
        quint64 totalAugmentingPaths = 0;   ///< 累计增广路径数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double maxFlowValue = 0.0;          ///< 最近一次最大流值
        int minCutSize = 0;                 ///< 最小割边数
    };

    explicit FlowNetwork(QObject* parent = nullptr);

    /**
     * @brief 设置图的顶点数
     * @param n 顶点数，>= 2
     */
    void setVertexCount(int n);

    /**
     * @brief 添加有向边
     * @param from 起点
     * @param to 终点
     * @param capacity 容量(>= 0)
     */
    void addEdge(int from, int to, double capacity);

    /**
     * @brief 计算从source到sink的最大流
     * @param source 源点
     * @param sink 汇点
     * @return 最大流值
     */
    double maxFlow(int source, int sink);

    /**
     * @brief 获取最小割集(S侧顶点和T侧顶点)
     * @param source 源点
     * @param sink 汇点
     * @return {S侧顶点列表, T侧顶点列表}
     */
    QPair<QVector<int>, QVector<int>> minCut(int source, int sink);

    /**
     * @brief 获取边上的流量
     * @param from 起点
     * @param to 终点
     * @return 当前流量
     */
    double edgeFlow(int from, int to) const;

    /** @brief 清空所有边 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最大流计算完成 @param flowValue 最大流值 @param paths 增广路径数 */
    void flowCompleted(double flowValue, int paths);

private:
    /** @brief 边结构 */
    struct Edge {
        int to;                 ///< 目标顶点
        double capacity;        ///< 残余容量
        double flow;            ///< 当前流量
        int rev;                ///< 反向边索引
    };

    /** @brief BFS寻找增广路径 */
    QPair<double, QVector<int>> bfsAugmentingPath(int source, int sink) const;

    int m_n = 0;
    QVector<QList<Edge>> m_adj;  ///< 邻接表

    Stats m_stats;
    double m_timeSum = 0.0;
};
