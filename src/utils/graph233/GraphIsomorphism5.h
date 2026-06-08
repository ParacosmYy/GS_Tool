/**
 * @file GraphIsomorphism5.h
 * @brief 图同构(个体化-细化+无迹典范增强对称图) — Graph Isomorphism with Individualization-Refinement and Traceless Canonical Augmentation for Symmetric Graphs
 *
 * 功能: 实现基于个体化-细化范式的图同构判定算法，
 *       通过无迹典范增强处理高度对称图的高效判定。
 *
 * 协作: CommunityDetection4(社区检测) / GraphColoring3(图着色) / ShortestPath5(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(个体化-细化+无迹典范增强)
 */
class GraphIsomorphism5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int refinementSteps = 0;
        bool lastResult = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism5(QObject *parent = nullptr);
    ~GraphIsomorphism5() override;

    /** @brief Check if two adjacency matrices represent isomorphic graphs */
    bool isIsomorphic(const QVector<QVector<int>>& adjA,
                      const QVector<QVector<int>>& adjB);

    /** @brief Compute canonical labeling of a graph */
    QVector<int> canonicalLabeling(const QVector<QVector<int>>& adj);

    /** @brief Compute vertex coloring via refinement */
    QVector<int> refine(const QVector<QVector<int>>& adj,
                        QVector<int> coloring) const;

    /** @brief Compute degree sequence */
    QVector<int> degreeSequence(const QVector<QVector<int>>& adj) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void isomorphismChecked(int n, bool result, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Count edges in adjacency matrix */
    int countEdges(const QVector<QVector<int>>& adj) const;

    /** @brief Check if coloring is discrete (all distinct) */
    bool isDiscrete(const QVector<int>& coloring) const;

    /** @brief Find first non-trivial color class */
    int firstNonTrivialClass(const QVector<int>& coloring) const;

    /** @brief Individualize vertex v in coloring */
    QVector<int> individualize(const QVector<int>& coloring, int vertex) const;

    /** @brief Compute canonical form hash */
    quint64 canonicalHash(const QVector<QVector<int>>& adj,
                           const QVector<int>& labeling) const;

    /** @brief Depth-first search with individualization-refinement */
    QVector<int> searchTree(const QVector<QVector<int>>& adj,
                             QVector<int> coloring,
                             int depth, int maxDepth);
};
