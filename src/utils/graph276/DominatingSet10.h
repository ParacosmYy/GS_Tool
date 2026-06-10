/**
 * @file DominatingSet10.h
 * @brief 支配集(贪心加权选择剪枝优化最小权连通支配) — Dominating Set with Greedy Weighted Selection and Pruning Optimization for Minimum Weight Connected Domination
 *
 * 功能: 实现支配集(dominating set)算法，采用贪心加权选择(greedy weighted
 *       selection)和剪枝优化(pruning optimization)求解最小权连通支配集
 *       (minimum weight connected domination)。
 *
 * 协作: MinSpanTree10(最小生成树) / VertexCover10(顶点覆盖) / MaxClique10(最大团)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 支配集(贪心加权选择剪枝优化最小权连通支配)
 */
class DominatingSet10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int dominatingSetSize = 0;
        double totalWeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge with weight */
    struct Edge {
        int from = -1;
        int to = -1;
        double weight = 1.0;
    };

    explicit DominatingSet10(QObject *parent = nullptr);
    ~DominatingSet10() override;

    /** @brief Build graph from adjacency list with vertex weights */
    void buildGraph(int numVertices, const QVector<Edge>& edges,
                    const QVector<double>& vertexWeights);

    /** @brief Compute minimum weight dominating set */
    QVector<int> computeDominatingSet();

    /** @brief Ensure connectivity: connect DS via Steiner-like pruning */
    QVector<int> computeConnectedDominatingSet();

    /** @brief Prune redundant vertices from DS */
    QVector<int> pruneRedundant(const QVector<int>& ds);

    /** @brief Check if set dominates all vertices */
    bool isDominating(const QVector<int>& set) const;

    /** @brief Get adjacency list */
    QVector<QVector<int>> adjacencyList() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void dominatingSetFound(int size, double totalWeight, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;           // adjacency list
    QVector<QVector<int>> m_adjSet;        // adjacency as sets (no dupes)
    QVector<double> m_weights;             // vertex weights
    QVector<Edge> m_edges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute uncovered count if vertex v is NOT in DS */
    int uncoveredCount(const QVector<bool>& dominated) const;

    /** @brief Greedy select next best vertex by weight/coverage ratio */
    int greedySelect(const QVector<bool>& inSet, const QVector<bool>& dominated) const;

    /** @brief BFS to find shortest path between two DS vertices */
    QVector<int> shortestPath(int from, int to, const QVector<bool>& inSet) const;

    /** @brief Connect DS components via shortest paths */
    QVector<int> connectComponents(QVector<int> ds) const;

    /** @brief Union-Find for connectivity check */
    int findRoot(QVector<int>& parent, int x) const;

    /** @brief Get connected components of the DS */
    QVector<QVector<int>> getComponents(const QVector<int>& ds) const;
};
