/**
 * @file MaximumClique7.h
 * @brief 最大团(Bron-Kerbosch轴心选择与退化排序高效极大枚举) — Maximum Clique with Bron-Kerbosch Pivot Selection and Degeneracy Ordering for Efficient Maximal Enumeration
 *
 * 功能: 实现最大团(Maximum clique)算法，采用Bron-Kerbosch轴心选择(pivot selection)
 *       与退化排序(degeneracy ordering)进行高效极大团枚举(maximal clique enumeration)。
 *
 * 协作: GraphColoring10(图着色) / MaximumMatching10(最大匹配) / IndependentSet8(最大独立集)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大团(Bron-Kerbosch轴心选择与退化排序高效极大枚举)
 */
class MaximumClique7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxCliqueSize = 0;
        int totalMaximalCliques = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumClique7(QObject *parent = nullptr);
    ~MaximumClique7() override;

    /** @brief Load graph from adjacency matrix */
    void setAdjacencyMatrix(const QVector<QVector<int>>& matrix);

    /** @brief Load graph from edge list (pairs of vertex indices) */
    void setEdgeList(int numVertices, const QVector<QPair<int,int>>& edges);

    /** @brief Find the maximum clique size and its members */
    QVector<int> findMaximumClique();

    /** @brief Enumerate all maximal cliques (bounded by limit) */
    QVector<QVector<int>> enumerateMaximalCliques(int maxCount = 10000);

    /** @brief Compute degeneracy ordering of vertices */
    QVector<int> degeneracyOrdering() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cliqueFound(int size, int numMaximal, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;    // adjacency lists
    QVector<QVector<int>> m_adjMatrix;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Result cache
    QVector<int> m_maxClique;
    QVector<QVector<int>> m_maximalCliques;

    /** @brief Bron-Kerbosch with pivot (Tomita variant) */
    void bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X,
                           QVector<QVector<int>>& results, int maxCount) const;

    /** @brief Select pivot vertex that maximizes |P ∩ N(u)| */
    int selectPivot(const QVector<int>& P, const QVector<int>& X) const;

    /** @brief Intersect vertex set with neighbors of v */
    QVector<int> intersectWithNeighbors(const QVector<int>& set, int v) const;

    /** @brief Compute core decomposition for degeneracy ordering */
    QVector<int> coreDecomposition() const;
};
