/**
 * @file DominatingSet12.h
 * @brief 支配集(贪心度数选择与冗余节点消除实现连通最小支配集近似) — Dominating Set with Greedy Degree-based Selection and Redundant Node Elimination for Connected Minimum Dominating Set Approximation
 *
 * 功能: 实现支配集(dominating set)，采用贪心度数选择(greedy degree-based selection)
 *       与冗余节点消除(redundant node elimination)实现连通最小支配集近似(connected MDS approximation)。
 *
 * 协作: VertexCover10(顶点覆盖) / MaxClique8(最大团) / GraphColoring10(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 支配集(贪心度数选择与冗余节点消除实现连通最小支配集近似)
 */
class DominatingSet12 : public QObject {
    Q_OBJECT

public:
    /** @brief Graph representation (adjacency list) */
    struct Graph {
        int numVertices = 0;
        QVector<QVector<int>> adj;      // adj[v] = neighbors of v
    };

    /** @brief Dominating set result */
    struct DSResult {
        QVector<int> dominatingSet;     // Vertices in the MDS
        QVector<int> dominatedBy;       // dominatedBy[v] = which MDS vertex covers v
        int setSize = 0;
        bool isConnected = false;       // Whether the MDS induces a connected subgraph
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet12(QObject *parent = nullptr);
    ~DominatingSet12() override;

    /** @brief Set graph from adjacency list */
    void setGraph(const Graph& graph);

    /** @brief Set graph from edge list */
    void setGraphFromEdges(int n, const QVector<QPair<int,int>>& edges);

    /** @brief Compute MDS via greedy degree selection */
    DSResult computeMDS();

    /** @brief Compute connected MDS with Steiner node augmentation */
    DSResult computeConnectedMDS();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mdsComputed(int setSize, bool connected, double timeMs);

private:
    Graph m_graph;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute degrees for all vertices */
    QVector<int> computeDegrees() const;

    /** @brief Check if vertex set dominates the graph */
    bool isDominated(const QVector<int>& domSet) const;

    /** @brief Eliminate redundant nodes from dominating set */
    QVector<int> eliminateRedundant(const QVector<int>& domSet) const;

    /** @brief Check if vertex set induces connected subgraph via BFS */
    bool isConnectedSubgraph(const QVector<int>& vertices) const;

    /** @brief BFS to find shortest path between two vertices avoiding certain nodes */
    QVector<int> shortestPath(int src, int dst, const QSet<int>& avoid) const;

    /** @brief Augment dominating set to become connected via Steiner nodes */
    void augmentToConnected(DSResult& result) const;
};
