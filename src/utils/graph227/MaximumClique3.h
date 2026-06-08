/**
 * @file MaximumClique3.h
 * @brief 最大团搜索(Tomita分支限界+着色上界剪枝) — Maximum Clique with Branch-and-Bound Tomita Algorithm and Color-Based Upper Bound Pruning
 *
 * 功能: 实现最大团搜索，支持Tomita分支限界、
 *       贪心着色上界剪枝和团枚举。
 *
 * 协作: GraphColoring5(图着色) / MaxIndependentSet4(最大独立集) / BronKerbosch2(BK算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大团搜索(Tomita分支限界+着色剪枝)
 */
class MaximumClique3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSearches = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxCliqueSize = 0;
        int nodesExplored = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumClique3(QObject *parent = nullptr);
    ~MaximumClique3() override;

    /** @brief Set adjacency matrix (n x n, symmetric, 0/1) */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Set adjacency from edge list */
    void setGraphFromEdges(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Find maximum clique */
    QVector<int> findMaximumClique();

    /** @brief Find all maximal cliques up to a size limit */
    QVector<QVector<int>> findAllMaximalCliques(int maxSizeLimit = 0);

    /** @brief Greedy coloring for upper bound estimation */
    QVector<int> greedyColoring(const QVector<int>& vertices) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int cliqueSize, int nodesExplored, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;  // Adjacency matrix
    QVector<int> m_bestClique;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Tomita branch-and-bound recursive search */
    void expand(QVector<int>& current, QVector<int>& candidates,
                int& bestSize, int& nodesExplored);

    /** @brief Number of vertices */
    int vertexCount() const { return m_n; }

    /** @brief Check if vertex v is adjacent to all vertices in clique */
    bool isAdjacentToAll(int v, const QVector<int>& clique) const;
};
