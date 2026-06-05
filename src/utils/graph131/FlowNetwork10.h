#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief FlowNetwork10 - 流网络算法第10代实现
 *
 * 提供最大流/最小割算法，支持Ford-Fulkerson、
 * Edmonds-Karp及Dinic算法，含残余图管理。
 */
class FlowNetwork10 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFlowOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit FlowNetwork10(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Edmonds-Karp算法求最大流（BFS增广）
     * @param capacityMatrix 容量矩阵
     * @param source 源点索引
     * @param sink 汇点索引
     * @return 最大流值
     */
    double edmondsKarp(const QVector<QVector<double>>& capacityMatrix,
                       int source, int sink);

    /**
     * @brief Dinic算法求最大流（分层图+阻塞流）
     * @param capacityMatrix 容量矩阵
     * @param source 源点索引
     * @param sink 汇点索引
     * @return 最大流值
     */
    double dinic(const QVector<QVector<double>>& capacityMatrix, int source, int sink);

    /**
     * @brief 求最小割（与最大流对偶）
     * @param capacityMatrix 容量矩阵
     * @param source 源点索引
     * @param sink 汇点索引
     * @return 割集顶点划分 (S侧, T侧)
     */
    QPair<QVector<int>, QVector<int>> minCut(
        const QVector<QVector<double>>& capacityMatrix, int source, int sink);

    /**
     * @brief 获取各边的实际流量
     * @return 流量矩阵
     */
    QVector<QVector<double>> getFlowMatrix() const;

signals:
    void flowCompleted(double maxFlow);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
