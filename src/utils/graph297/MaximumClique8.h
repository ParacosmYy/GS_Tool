/**
 * @file MaximumClique8.h
 * @brief 最大团(位并行分支限界与顶点着色上界的精确团枚举) — Maximum Clique with Bit-parallel Branch-and-bound and Vertex Coloring Upper Bound for Efficient Exact Clique Enumeration
 *
 * 功能: 实现最大团(maximum clique)，采用位并行分支限界(bit-parallel branch-and-bound)
 *       与顶点着色上界(vertex coloring upper bound)实现精确团枚举(exact clique enumeration)。
 *
 * 协作: GraphColoring9(图着色) / BronKerbosch7(BK团枚举) / MIS7(最大独立集)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大团(位并行分支限界与顶点着色上界)
 */
class MaximumClique8 : public QObject {
    Q_OBJECT

public:
    /** @brief Clique search result */
    struct CliqueResult {
        QVector<int> maxClique;
        int cliqueSize = 0;
        int nodesExplored = 0;
        int totalCliques = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int bestCliqueSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumClique8(QObject *parent = nullptr);
    ~MaximumClique8() override;

    /** @brief Build adjacency from edge list */
    void setGraph(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Build adjacency from adjacency matrix */
    void setGraph(const QVector<QVector<int>>& adjMatrix);

    /** @brief Find the maximum clique */
    CliqueResult findMaximumClique();

    /** @brief Find all maximal cliques up to a limit */
    QVector<QVector<int>> findAllMaximalCliques(int maxCount = 1000);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchDone(int cliqueSize, int nodesExplored, double timeMs);

private:
    int m_n = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Adjacency bitmasks for bit-parallel operations */
    QVector<quint64> m_adjBits;

    /** @brief Regular adjacency list (fallback for large graphs) */
    QVector<QVector<int>> m_adjList;

    /** @brief Current best clique */
    QVector<int> m_bestClique;
    int m_bestSize = 0;

    /** @brief Counters */
    int m_nodesExplored = 0;
    int m_totalCliques = 0;

    /** @brief Greedy vertex coloring for upper bound */
    int greedyColoring(const QVector<int>& candidates) const;

    /** @brief Branch-and-bound with bit-parallel expansion */
    void expand(QVector<int>& current, QVector<int>& candidates);

    /** @brief Check if vertex v is adjacent to all in clique */
    bool isAdjacentToAll(int v, const QVector<int>& clique) const;
};
