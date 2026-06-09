/**
 * @file MaximumFlow5.h
 * @brief 最大流(Dinic分层图BFS+阻塞流DFS动态树加速) — Maximum Flow with Dinic's Level Graph BFS and Blocking Flow DFS with Dynamic Tree Speedup
 *
 * 功能: 实现最大流(Maximum Flow)算法，使用Dinic分层图(level graph)BFS构建
 *       层次网络，阻塞流(blocking flow)DFS配合动态树(dynamic tree)加速
 *       寻找增广路径，用于网络流优化。
 *
 * 协作: MinCostMaxFlow8(最小费用最大流) / PushRelabel6(推重标) / BipartiteMatching5(二分匹配)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大流(Dinic分层图BFS+阻塞流DFS动态树加速)
 */
class MaximumFlow5 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the flow network */
    struct Edge {
        int to = 0;
        int rev = 0;         // Index of reverse edge in adjacency list of 'to'
        double capacity = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numEdges = 0;
        double maxFlowValue = 0.0;
        int bfsPhases = 0;
        int dfsPaths = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumFlow5(QObject *parent = nullptr);
    ~MaximumFlow5() override;

    /** @brief Initialize graph with n nodes */
    void initGraph(int n);

    /** @brief Add directed edge with capacity (and reverse edge with 0) */
    void addEdge(int from, int to, double capacity);

    /** @brief Compute maximum flow from source to sink */
    double maxFlow(int source, int sink);

    /** @brief Get min-cut partition (nodes reachable from source in residual) */
    QVector<int> minCut(int source) const;

    /** @brief Get flow on each edge */
    QVector<QVector<QPair<int, double>>> flowOnEdges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void flowComputed(int nodes, double maxFlow, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<Edge>> m_graph;
    QVector<int> m_level;        // BFS level graph
    QVector<int> m_iter;         // Current edge iterator for DFS

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Dynamic tree parent for acceleration */
    struct DynNode {
        int parent = -1;
        double minCap = 1e18;   // Min capacity on path to root
    };
    QVector<DynNode> m_dynTree;

    /** @brief BFS: build level graph, return false if sink unreachable */
    bool buildLevelGraph(int source, int sink);

    /** @brief DFS: find blocking flow with dynamic tree speedup */
    double blockingFlow(int v, int sink, double flow);

    /** @brief Dynamic tree: link node to parent with capacity */
    void dynLink(int node, int parent, double cap);

    /** @brief Dynamic tree: find minimum capacity on path to root */
    double dynMinCap(int node) const;

    /** @brief Dynamic tree: push flow along path, update capacities */
    void dynPush(double flow);

    /** @brief Dynamic tree: cut node from parent */
    void dynCut(int node);
};
