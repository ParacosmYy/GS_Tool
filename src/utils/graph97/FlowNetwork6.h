#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 流网络最大流求解器
 *
 * 基于Ford-Fulkerson/Dinic算法的最大流计算。
 */
class FlowNetwork6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFlowsComputed = 0;
        int totalAugmentingPaths = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FlowNetwork6(QObject* parent = nullptr);

    /** @brief 计算从source到sink的最大流 */
    double maxFlow(const QVector<QVector<double>>& capacity, int source, int sink);

    /** @brief 获取最小割集 */
    QPair<QVector<int>, QVector<int>> minCut(int source) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void flowComputed(int source, int sink, double maxFlowValue);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<int> m_sourceSide;
};
