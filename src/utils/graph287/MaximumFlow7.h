/**
 * @file MaximumFlow7.h
 * @brief 最大流(Dinic层次图BFS与阻塞流DFS分层网络增广) — Maximum Flow with Dinic's Level Graph BFS and Blocking Flow DFS for Layered Network Augmentation
 *
 * 功能: 实现最大流(maximum flow)，采用Dinic层次图BFS(Dinic's level graph BFS)
 *       与阻塞流DFS(blocking flow DFS)实现分层网络增广(layered network augmentation)。
 *
 * 协作: MinCostFlow8(最小费用流) / BipartiteMatch9(二分匹配) / PushRelabel10(推送-重标号)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大流(Dinic层次图BFS与阻塞流DFS分层网络增广)
 */
class MaximumFlow7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numEdges = 0;
        int numBFSPhases = 0;
        double maxFlow = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumFlow7(QObject *parent = nullptr);
    ~MaximumFlow7() override;

    /** @brief Set number of nodes in the flow network */
    void setNodes(int n);

    /** @brief Add directed edge with capacity (and reverse edge with 0) */
    void addEdge(int from, int to, double capacity);

    /** @brief Compute max flow from source to sink using Dinic's algorithm */
    double maxFlow(int source, int sink);

    /** @brief Get flow on edge (by original edge index) */
    double edgeFlow(int edgeIndex) const;

    /** @brief Get min-cut partition (nodes reachable from source) */
    QVector<int> minCut(int source) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void flowComputed(double maxFlow, int phases, double timeMs);

private:
    int m_n = 0;

    /** @brief Edge in adjacency list */
    struct Edge {
        int to = -1;
        int rev = -1;       // index of reverse edge in adj[to]
        double cap = 0.0;
        double flow = 0.0;
    };

    QVector<QVector<Edge>> m_adj;
    QVector<int> m_level;       // BFS level of each node
    QVector<int> m_iter;        // Current edge iterator for DFS

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief BFS: build level graph from source */
    bool buildLevelGraph(int source, int sink);

    /** @brief DFS: find blocking flow with current iterator optimization */
    double blockingFlow(int node, int sink, double pushed);

    /** @brief Reset edge capacities and flows */
    void resetEdges();
};
