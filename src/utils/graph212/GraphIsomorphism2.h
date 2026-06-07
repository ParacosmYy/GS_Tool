/**
 * @file GraphIsomorphism2.h
 * @brief 图同构(颜色精炼+2维Weisfeiler-Lehman测试) — Graph Isomorphism via Color Refinement with 2-Dimensional Weisfeiler-Lehman Test
 *
 * 功能: 实现图同构检测，支持颜色精炼、
 *       2维Weisfeiler-Lehman测试和不变量计算。
 *
 * 协作: CommunityDetect7(社区检测) / MaxFlow5(最大流) / ShortestPath6(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 图同构(颜色精炼+2-WL测试)
 */
class GraphIsomorphism2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTests = 0;
        int graphSize1 = 0;
        int graphSize2 = 0;
        int wlIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Graph in adjacency list form */
    struct Graph {
        int numVertices = 0;
        QVector<QVector<int>> adjacency;     // neighbor lists
        QVector<QVector<double>> weights;    // edge weights
    };

    explicit GraphIsomorphism2(QObject *parent = nullptr);
    ~GraphIsomorphism2() override;

    void setMaxIterations(int iters);
    void setUse2WL(bool enable);

    /** @brief Test if two graphs are isomorphic */
    bool isIsomorphic(const Graph& g1, const Graph& g2);

    /** @brief 1-D WL color refinement */
    QVector<int> colorRefine1D(const Graph& g) const;

    /** @brief 2-D WL color refinement */
    QVector<QVector<int>> colorRefine2D(const Graph& g) const;

    /** @brief Compute canonical coloring string */
    QByteArray canonicalForm(const Graph& g) const;

    /** @brief Compute degree sequence invariant */
    QVector<int> degreeSequence(const Graph& g) const;

    /** @brief Compute eigenvalue-based invariant */
    QVector<double> eigenInvariant(const Graph& g) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testCompleted(bool result, int iterations, double timeMs);

private:
    int m_maxIter = 100;
    bool m_use2WL = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hash a sorted tuple of integers */
    quint64 hashTuple(const QVector<int>& tuple) const;

    /** @brief Power iteration for largest eigenvalues */
    QVector<double> powerIteration(const QVector<QVector<double>>& matrix, int k) const;

    /** @brief Check basic invariants (quick reject) */
    bool checkInvariants(const Graph& g1, const Graph& g2) const;
};
