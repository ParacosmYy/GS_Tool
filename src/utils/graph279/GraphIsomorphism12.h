/**
 * @file GraphIsomorphism12.h
 * @brief 图同构(Weisfeiler-Lehman k维细化与颜色精炼多项式时间判别) — Graph Isomorphism with Weisfeiler-Lehman k-Dimensional Refinement and Color Refinement for Polynomial-Time Discrimination
 *
 * 功能: 实现图同构(graph isomorphism)，采用Weisfeiler-Lehman k维细化(Weisfeiler-Lehman
 *       k-dimensional refinement)和颜色精炼(color refinement)实现多项式时间判别
 *       (polynomial-time discrimination)。
 *
 * 协作: GraphColoring10(图着色) / CommunityDetect11(社区检测) / ShortestPath9(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(Weisfeiler-Lehman k维细化与颜色精炼多项式时间判别)
 */
class GraphIsomorphism12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int wlIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism12(QObject *parent = nullptr);
    ~GraphIsomorphism12() override;

    /** @brief Check if two graphs are isomorphic */
    bool isIsomorphic(const QVector<QVector<int>>& adj1,
                      const QVector<QVector<int>>& adj2) const;

    /** @brief Compute WL color signature for a graph */
    QVector<quint64> wlSignature(const QVector<QVector<int>>& adj) const;

    /** @brief Run k-dimensional WL refinement */
    QVector<QVector<quint64>> wlKDim(const QVector<QVector<int>>& adj, int k) const;

    /** @brief Compute canonical label for graph */
    QString canonicalLabel(const QVector<QVector<int>>& adj) const;

    /** @brief Get vertex coloring from WL refinement */
    QVector<int> vertexColoring(const QVector<QVector<int>>& adj) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void comparisonComplete(bool isomorphic, int iterations, double timeMs);

private:
    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief 1-dimensional WL color refinement */
    QVector<quint64> wlRefine1D(const QVector<QVector<int>>& adj,
                                  int maxIter) const;

    /** @brief Compress color multiset into hash */
    quint64 hashColors(const QVector<quint64>& colors) const;

    /** @brief Sort and compare color signatures */
    static bool signaturesEqual(QVector<quint64> sig1, QVector<quint64> sig2);

    /** @brief Count edges in adjacency list */
    static int countEdges(const QVector<QVector<int>>& adj);
};
