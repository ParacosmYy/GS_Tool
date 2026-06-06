/**
 * @file MaximumFlow.h
 * @brief 最大流(Dinic阻塞流+层次图BFS分层) — Maximum Flow via Dinic's Algorithm with Blocking Flow and Level Graph BFS Layering
 *
 * 功能: 实现Dinic最大流算法，支持BFS层次图分层、DFS阻塞流增广、
 *       当前弧优化、最小割计算和多源多汇扩展。
 *
 * 协作: MinCostFlow6(最小费用流) / BipartiteMatch8(二分匹配) / PushRelabel9(推送重标)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Dinic最大流算法(阻塞流+层次图)
 */
class MaximumFlow : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the flow network */
    struct Edge {
        int to = 0;             ///< Target vertex
        int rev = 0;            ///< Reverse edge index in adjacency list
        double capacity = 0.0;  ///< Residual capacity
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        double maxFlow = 0.0;
        double minCutCapacity = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumFlow(int numVertices, QObject *parent = nullptr);
    ~MaximumFlow() override;

    /** @brief Add directed edge with capacity */
    void addEdge(int from, int to, double capacity);

    /** @brief Compute max flow from source to sink */
    double maxFlow(int source, int sink);

    /** @brief Compute min cut (returns cut vertices on source side) */
    QVector<int> minCut(int source, int sink) const;

    /** @brief Get flow on edge (from -> to) */
    double edgeFlow(int from, int to) const;

    /** @brief Reset graph for reuse */
    void reset();

    /** @brief Number of vertices */
    int vertexCount() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void flowComputed(int source, int sink, double flow);

private:
    int m_n = 0;

    QVector<QVector<Edge>> m_graph;
    QVector<int> m_level;
    QVector<int> m_iter;
    QVector<double> m_edgeCap;   ///< Original capacities for flow query

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief BFS: build level graph */
    bool bfs(int source, int sink);

    /** @brief DFS: find blocking flow */
    double dfs(int v, int sink, double f);

    /** @brief Internal dfs with current-arc optimization */
    double dinicDfs(int v, int sink, double f);
};
