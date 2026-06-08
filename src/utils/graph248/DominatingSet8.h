/**
 * @file DominatingSet8.h
 * @brief 支配集(贪心近似+闭邻域基数剪枝) — Dominating Set with Greedy Approximation and Candidate Pruning Using Closed Neighborhood Cardinality Bounds
 *
 * 功能: 实现支配集(dominating set)算法，采用贪心近似(greedy approximation)策略，
 *       并使用闭邻域基数界限(closed neighborhood cardinality bounds)进行候选剪枝
 *       (candidate pruning)以加速求解过程。
 *
 * 协作: MaxClique8(最大团) / GraphColoring9(图着色) / MinimumSpanningTree8(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 支配集(贪心近似+闭邻域基数剪枝)
 */
class DominatingSet8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int dominatingSetSize = 0;
        int numPruned = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet8(QObject *parent = nullptr);
    ~DominatingSet8() override;

    /** @brief Build graph from adjacency list */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Build graph from edge list (undirected) */
    void setEdges(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Compute minimum dominating set (greedy approximation) */
    QVector<int> solve();

    /** @brief Check if a given set is a valid dominating set */
    bool isValidDominatingSet(const QVector<int>& candidateSet) const;

    /** @brief Get the closed neighborhood (vertex + its neighbors) */
    QVector<int> closedNeighborhood(int vertex) const;

    /** @brief Get number of uncovered vertices for a candidate */
    int uncoveredCount(int vertex, const QVector<bool>& covered) const;

    /** @brief Get adjacency list */
    QVector<QVector<int>> graph() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void vertexAdded(int vertex, int newlyDominated);
    void solveCompleted(int setSize, double timeMs);

private:
    int m_numVertices = 0;
    QVector<QVector<int>> m_adj;     // Adjacency list

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute closed neighborhood cardinality for pruning bound */
    int closedNeighborhoodSize(int vertex) const;

    /** @brief Prune candidates using cardinality bounds */
    QVector<int> pruneCandidates(const QVector<int>& candidates,
                                  int remainingUndominated) const;

    /** @brief Compute upper bound on dominating set size */
    int upperBound(int remaining) const;
};
