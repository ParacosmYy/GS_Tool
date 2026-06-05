#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 流网络最大流求解器
 *
 * 基于Edmonds-Karp/Dinic算法计算网络最大流，
 * 支持动态添加边和容量设置。
 */
class FlowNetwork8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit FlowNetwork8(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加有向边及容量 */
    void addEdge(int from, int to, double capacity);

    /** @brief 计算从源点到汇点的最大流 */
    double maxFlow(int source, int sink);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最大流求解完成信号 */
    void solved(double maxFlowValue);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
