/**
 * @file MaximumFlow2.h
 * @brief 最大流(Push-Relabel+间隙启发式+全局重标号优化) — Maximum Flow via Push-Relabel with Gap Heuristic and Global Relabeling Optimization
 *
 * 功能: 实现最大流算法，支持Push-Relabel方法、
 *       间隙启发式优化和全局重标号策略。
 *
 * 协作: EdmondsKarp1(Edmonds-Karp) / Dinic3(Dinic算法) / MinCostFlow1(最小费用流)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大流(Push-Relabel+间隙启发式)
 */
class MaximumFlow2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int numNodes = 0;
        int numEdges = 0;
        double maxFlow = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge in the flow network */
    struct Edge {
        int to = 0;
        double capacity = 0.0;
        double flow = 0.0;
        int rev = 0;  // index of reverse edge in adj[to]
    };

    explicit MaximumFlow2(QObject *parent = nullptr);
    ~MaximumFlow2() override;

    void setNumNodes(int n);

    /** @brief Add directed edge u->v with capacity */
    void addEdge(int u, int v, double capacity);

    /** @brief Compute max flow from source to sink */
    double solve(int source, int sink);

    /** @brief Get current flow on each edge */
    QVector<QVector<Edge>> flowNetwork() const;

    /** @brief Get min-cut partition (reachable from source in residual graph) */
    QVector<int> minCut(int source) const;

    /** @brief Get total flow into a node */
    double inflow(int node) const;

    /** @brief Get total flow out of a node */
    double outflow(int node) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int nodes, double maxFlow, double timeMs);

private:
    int m_n = 0;

    QVector<QVector<Edge>> m_adj;
    QVector<double> m_excess;
    QVector<int> m_height;
    QVector<int> m_count;    // count of nodes at each height (gap heuristic)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Push flow from u along edge index e */
    void push(int u, int eIdx);

    /** @brief Relabel node u */
    void relabel(int u);

    /** @brief Discharge: push/relabel until excess is 0 */
    void discharge(int u);

    /** @brief Gap heuristic: nodes above gap height get infinite height */
    void gapHeuristic(int gapHeight);

    /** @brief Global relabeling: BFS from sink to reset heights */
    void globalRelabel(int sink);

    /** @brief Check if node is admissible for push */
    bool isAdmissible(int u, int eIdx) const;
};
