/**
 * @file MaximumFlow6.h
 * @brief 最大流(Push-Relabel+间隙启发式全局重标号高效最大流) — Maximum Flow with Push-Relabel Algorithm and Gap Heuristic with Global Relabeling for Efficient Max-flow Computation
 *
 * 功能: 实现最大流(maximum flow)的Push-Relabel算法，采用间隙启发式
 *       (gap heuristic)和全局重标号(global relabeling)优化，高效
 *       计算网络最大流。
 *
 * 协作: MinCostFlow8(最小费用流) / BipartiteMatching7(二分图匹配) / EdmondsKarp5(Edmonds-Karp)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大流(Push-Relabel+间隙启发式全局重标号高效最大流)
 */
class MaximumFlow6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numEdges = 0;
        double maxFlowValue = 0.0;
        int numPushes = 0;
        int numRelabels = 0;
        int numGaps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Directed edge in flow network */
    struct Edge {
        int to = -1;
        int rev = -1;         // Reverse edge index
        double capacity = 0.0;
        double flow = 0.0;
    };

    explicit MaximumFlow6(QObject *parent = nullptr);
    ~MaximumFlow6() override;

    /** @brief Initialize network with n nodes */
    void initNetwork(int numNodes);

    /** @brief Add directed edge with capacity */
    void addEdge(int from, int to, double capacity);

    /** @brief Compute max flow from source to sink */
    double maxFlow(int source, int sink);

    /** @brief Get min-cut partition (nodes reachable from source in residual) */
    QVector<int> minCut(int source) const;

    /** @brief Get edge flows */
    QVector<QPair<int, int>> edgeFlows() const;

    /** @brief Get residual capacity of an edge */
    double residualCapacity(int from, int edgeIdx) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void flowComputed(double maxValue, int pushes, int relabels, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<Edge>> m_graph;
    QVector<double> m_excess;
    QVector<int> m_height;
    QVector<int> m_count;     // Height count for gap heuristic

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Push flow from node u along edge e */
    void push(int u, int ei);

    /** @brief Relabel node u */
    void relabel(int u);

    /** @brief Gap heuristic: nodes above gap level get infinite height */
    void gapHeuristic(int gapLevel);

    /** @brief Global relabeling via reverse BFS from sink */
    void globalRelabel(int source, int sink);

    /** @brief Discharge node u */
    void discharge(int u);
};
