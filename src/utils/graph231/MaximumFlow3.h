/**
 * @file MaximumFlow3.h
 * @brief 最大流(Dinic阻塞流+动态树数据结构O(VE log V)缩放) — Maximum Flow with Dinic Blocking Flow and Dynamic Tree Data Structure for O(VE log V) Scaling
 *
 * 功能: 实现Dinic最大流算法，结合动态树数据结构实现O(VE log V)复杂度，
 *       支持层级图构建、阻塞流推进、容量缩放和多源多汇扩展。
 *
 * 协作: MinimumCostFlow4(最小费用流) / BipartiteMatching5(二分匹配) / GraphCut3(图割)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大流(Dinic阻塞流+动态树)
 */
class MaximumFlow3 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge structure for residual graph */
    struct Edge {
        int to = -1;
        int rev = 0;           // Reverse edge index
        double capacity = 0.0;
        double flow = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double maxFlow = 0.0;
        int dinicPhases = 0;
        int augmentingPaths = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumFlow3(QObject *parent = nullptr);
    ~MaximumFlow3() override;

    /** @brief Initialize graph with vertex count */
    void initGraph(int vertices);

    /** @brief Add directed edge with capacity */
    void addEdge(int from, int to, double capacity);

    /** @brief Compute max flow from source to sink */
    double maxFlow(int source, int sink);

    /** @brief Get min-cut partition (vertices reachable from source) */
    QVector<int> minCut(int source) const;

    /** @brief Get flow on specific edge */
    double edgeFlow(int from, int edgeIdx) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void flowCompleted(double maxFlow, int phases, double timeMs);

private:
    int m_n = 0;

    // Adjacency list: m_graph[u][i] = Edge
    QVector<QVector<Edge>> m_graph;

    // BFS level graph
    QVector<int> m_level;

    // DFS iterator for current arc optimization
    QVector<int> m_iter;

    // Dynamic tree parent pointers for O(VE log V)
    QVector<int> m_treeParent;
    QVector<double> m_treeCap;
    QVector<int> m_treeChild;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief BFS to build level graph */
    bool buildLevelGraph(int source, int sink);

    /** @brief DFS blocking flow with current arc optimization */
    double blockingFlow(int v, int sink, double flow);

    /** @brief Dynamic tree: find root and min capacity along path */
    int treeFindRoot(int v) const;

    /** @brief Dynamic tree: link edge with capacity */
    void treeLink(int child, int parent, double cap);

    /** @brief Dynamic tree: cut and send flow */
    double treeCutSend(int v, double flow);
};
