/**
 * @file DominatingSet7.h
 * @brief 支配集(Greedy CDS+Steiner树剪枝无线网络) — Dominating Set via Greedy Connected Dominating Set with Steiner Tree Pruning for Wireless Networks
 *
 * 功能: 实现贪心连通支配集算法，通过Steiner树剪枝优化无线网络中的
 *       虚拟骨干节点选择，支持动态图更新。
 *
 * 协作: MinSpanTree6(最小生成树) / Dijkstra7(最短路径) / GraphColor5(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 支配集(Greedy CDS+Steiner树剪枝)
 */
class DominatingSet7 : public QObject {
    Q_OBJECT

public:
    /** @brief Graph edge */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 1.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int dominatingSetSize = 0;
        int steinerPruned = 0;
        int graphDegree = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet7(QObject *parent = nullptr);
    ~DominatingSet7() override;

    /** @brief Build graph from edge list */
    void buildGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Add a single edge to the graph */
    void addEdge(int from, int to, double weight = 1.0);

    /** @brief Compute greedy connected dominating set */
    QVector<int> computeCDS();

    /** @brief Apply Steiner tree pruning to reduce CDS */
    QVector<int> steinerPrune(const QVector<int>& cds);

    /** @brief Check if a vertex set is a valid dominating set */
    bool isDominating(const QVector<int>& vertexSet) const;

    /** @brief Get neighbors of a vertex */
    QVector<int> neighbors(int vertex) const;

    /** @brief Get the current dominating set */
    QVector<int> dominatingSet() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void dominatingSetComputed(int setSize, int pruned, double timeMs);

private:
    int m_numVertices = 0;
    QVector<QVector<int>> m_adjList;      // Adjacency list
    QVector<QVector<double>> m_weights;   // Edge weights

    QVector<int> m_dominatingSet;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute degree of a vertex */
    int degree(int vertex) const;

    /** @brief BFS from source, returns parent map */
    QVector<int> bfs(int source, const QVector<bool>& allowed) const;

    /** @brief Check connectivity of a vertex subset */
    bool isConnected(const QVector<int>& subset) const;

    /** @brief Compute Steiner tree cost */
    double steinerCost(const QVector<int>& terminals) const;

    /** @brief Find shortest path between two vertices */
    QVector<int> shortestPath(int from, int to, const QVector<bool>& allowed) const;

    /** @brief Compute maximum graph degree */
    int maxDegree() const;
};
