#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 流网络最大流求解器
 *
 * 基于Ford-Fulkerson/Dinic算法求解最大流最小割问题,
 * 适用于网络流量优化、任务分配与瓶颈分析。
 */
class FlowNetwork7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit FlowNetwork7(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加有向边及容量 */
    void addEdge(int from, int to, double capacity);

    /** @brief 计算源点到汇点的最大流 */
    void maxFlow(int source, int sink);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号,返回最大流量 */
    void solved(double maxFlowValue);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
