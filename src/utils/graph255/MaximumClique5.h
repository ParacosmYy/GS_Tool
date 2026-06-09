/**
 * @file MaximumClique5.h
 * @brief 最大团(Bron-Kerbosch枢轴选择+退化排序有界搜索树) — Maximum Clique with Bron-Kerbosch Pivot Selection and Degeneracy Ordering for Bounded Search Tree
 *
 * 功能: 实现最大团问题(Maximum clique problem)求解，采用Bron-Kerbosch算法配合枢轴选择
 *       (pivot selection)剪枝候选顶点集，通过退化排序(degeneracy ordering)对顶点预排序
 *       构建有界搜索树(bounded search tree)，实现高效精确最大团搜索。
 *
 * 协作: GraphColoring4(图着色) / MinSpanningTree3(最小生成树) / ShortestPath6(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最大团(Bron-Kerbosch枢轴选择+退化排序有界搜索树)
 */
class MaximumClique5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxCliqueSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumClique5(QObject *parent = nullptr);
    ~MaximumClique5() override;

    /** @brief Load adjacency matrix (n x n symmetric, 0/1) */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Load edge list (pairs of vertex indices) */
    void setEdgeList(int numVertices, const QVector<QPair<int,int>>& edges);

    /** @brief Find maximum clique, returns vertex indices */
    QVector<int> solve();

    /** @brief Get clique size without returning vertices */
    int maxCliqueSize() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int cliqueSize, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;    // adjacency lists
    QVector<int> m_bestClique;
    int m_bestSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute degeneracy ordering of vertices */
    QVector<int> degeneracyOrdering() const;

    /** @brief Bron-Kerbosch with pivot (Tomita variant) */
    void bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X);

    /** @brief Select pivot vertex that maximizes |P ∩ N(u)| */
    int selectPivot(const QVector<int>& P, const QVector<int>& X) const;

    /** @brief Intersect vertex set with neighbors of v */
    QVector<int> intersectNeighbors(const QVector<int>& S, int v) const;
};
