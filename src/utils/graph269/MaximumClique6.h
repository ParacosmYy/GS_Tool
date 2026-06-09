/**
 * @file MaximumClique6.h
 * @brief 最大团(MaxSAT上界+渐进边界紧缩搜索空间) — Maximum Clique with MaxSAT-Based Upper Bound and Progressive Bounding for Tight Search Space Reduction
 *
 * 功能: 实现最大团问题(Maximum Clique Problem)的精确求解，使用MaxSAT
 *       松弛上界(MaxSAT-based upper bound)和渐进边界紧缩(progressive
 *       bounding)策略逐步收紧搜索空间，实现高效分支限界搜索。
 *
 * 协作: GraphColoring8(图着色) / BronKerbosch7(极大团枚举) / SATSolver5(SAT求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大团(MaxSAT上界+渐进边界紧缩)
 */
class MaximumClique6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int cliqueSize = 0;
        int numBoundPrunes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumClique6(QObject *parent = nullptr);
    ~MaximumClique6() override;

    /** @brief Load adjacency matrix (n x n, symmetric, 0/1) */
    void setGraph(const QVector<QVector<int>>& adjMatrix);

    /** @brief Load adjacency list */
    void setGraphList(const QVector<QVector<int>>& adjList, int numVertices);

    /** @brief Find maximum clique, return vertex indices */
    QVector<int> solve();

    /** @brief Get clique size without returning vertices */
    int cliqueSize() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int cliqueSize, int prunes, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;     // Adjacency list
    QVector<QVector<int>> m_adjMat;  // Adjacency matrix
    QVector<int> m_bestClique;
    int m_bestSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief MaxSAT-based upper bound via greedy coloring */
    int maxSatBound(const QVector<int>& candidates) const;

    /** @brief Greedy coloring to estimate chromatic number upper bound */
    int greedyColor(const QVector<int>& vertices,
                    QVector<int>& colors) const;

    /** @brief Branch-and-bound recursive search */
    void branchBound(const QVector<int>& currentClique,
                     QVector<int>& candidates);

    /** @brief Check if vertex v is adjacent to all vertices in clique */
    bool isCliqueVertex(int v, const QVector<int>& clique) const;

    /** @brief Count edges in graph */
    int countEdges() const;
};
