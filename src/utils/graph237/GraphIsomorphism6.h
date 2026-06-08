/**
 * @file GraphIsomorphism6.h
 * @brief 图同构(Weisfeiler-Leman k维精细化+k可区分性测试) — Graph Isomorphism with Weisfeiler-Leman k-Dimensional Refinement and k-Discriminability Testing
 *
 * 功能: 实现图同构判定算法，采用Weisfeiler-Leman k维标签精细化，
 *       通过k可区分性测试判断两个图是否同构。
 *
 * 协作: GraphColoring5(图着色) / CommunityDetect4(社区发现) / ShortestPath8(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 图同构(Weisfeiler-Leman k维精细化)
 */
class GraphIsomorphism6 : public QObject {
    Q_OBJECT

public:
    /** @brief Isomorphism test result */
    struct IsoResult {
        bool isomorphic = false;
        bool definitelyNot = false;
        int refinementIterations = 0;
        int kDimension = 1;
        QVector<int> coloring1;
        QVector<int> coloring2;
        QVector<int> mapping;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices1 = 0;
        int numVertices2 = 0;
        int numEdges1 = 0;
        int numEdges2 = 0;
        int kDimension = 1;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism6(QObject *parent = nullptr);
    ~GraphIsomorphism6() override;

    /** @brief Set WL refinement dimension k */
    void setDimension(int k);

    /** @brief Test isomorphism between two adjacency matrices */
    IsoResult test(const QVector<QVector<int>>& adj1,
                   const QVector<QVector<int>>& adj2);

    /** @brief Compute WL coloring for a graph */
    QVector<int> wlColoring(const QVector<QVector<int>>& adj, int maxIter = 100) const;

    /** @brief Verify coloring histogram compatibility */
    bool histogramCompatible(const QVector<int>& c1, const QVector<int>& c2) const;

    /** @brief Find vertex mapping between two colored graphs */
    QVector<int> findMapping(const QVector<QVector<int>>& adj1,
                              const QVector<QVector<int>>& adj2,
                              const QVector<int>& c1,
                              const QVector<int>& c2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testCompleted(bool isomorphic, int iterations, double timeMs);

private:
    int m_k = 1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief One WL refinement step */
    QVector<int> wlRefine(const QVector<QVector<int>>& adj,
                            const QVector<int>& colors) const;

    /** @brief Compress sorted neighbor label tuple to new color */
    int compressColors(const QVector<int>& sortedColors,
                        const QHash<QString, int>& colorMap, int& nextColor) const;

    /** @brief k-WL coloring (higher-order) */
    QVector<int> kWLColoring(const QVector<QVector<int>>& adj, int k, int maxIter) const;

    /** @brief Check if coloring is stable (no change) */
    bool isStable(const QVector<int>& prev, const QVector<int>& curr) const;
};
