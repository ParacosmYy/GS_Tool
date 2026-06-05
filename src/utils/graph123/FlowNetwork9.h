#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 流网络最大流/最小割算法实现
 *
 * 基于Dinic算法实现有向图的最大流计算，支持动态添加边和容量更新，
 * 适用于网络流量优化、资源分配和二分图匹配等场景。
 */
class FlowNetwork9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit FlowNetwork9(QObject* parent = nullptr);

    /** @brief 设置图的节点数并初始化邻接表 */
    void setNodeCount(int n);

    /** @brief 添加一条从u到v的有向边，容量为capacity */
    void addEdge(int from, int to, double capacity);

    /** @brief 计算从源点到汇点的最大流 */
    double maxFlow(int source, int sink);

    /** @brief 计算最小割，返回割边列表 */
    QVector<QPair<int, int>> minCut(int source, int sink);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最大流计算完成信号，返回流量值 */
    void flowComputed(double maxValue);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_nodeCount = 0;
};
