/**
 * @file GraphIsomorphism16.h
 * @brief 图同构(Weisfeiler-Leman k维细化与顶点分类实现有界度图认证) — Graph Isomorphism with Weisfeiler-Leman K-dimensional Refinement and Vertex Classification for Bounded-degree Graph Certification
 *
 * 功能: 实现图同构检测(graph isomorphism)，采用Weisfeiler-Leman k维细化(Weisfeiler-Leman k-dimensional refinement)
 *       与顶点分类(vertex classification)实现有界度图认证(bounded-degree graph certification)。
 *
 * 协作: VF2Subgraph12(VF2子图匹配) / GraphColoring14(图着色) / GraphConnectivity11(图连通性)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

class GraphIsomorphism16 : public QObject {
    Q_OBJECT

public:
    /** @brief Isomorphism test result */
    struct IsoResult {
        bool isIsomorphic = false;
        bool isCertified = false;        // True if WL test certifies non-isomorphism
        QVector<int> mapping;            // Vertex mapping G1 -> G2
        int wlIterations = 0;
        int numVertices = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTests = 0;
        int isoCount = 0;
        int nonIsoCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism16(QObject *parent = nullptr);
    ~GraphIsomorphism16() override;

    void setMaxWLIterations(int iter);
    void setWLDimension(int k);              // k-WL dimension (1 or 2)

    /** @brief Test if two graphs are isomorphic */
    IsoResult testIsomorphism(const QVector<QVector<int>>& adj1,
                               const QVector<QVector<int>>& adj2) const;

    /** @brief Compute WL vertex coloring (stable coloring) */
    QVector<int> computeWLColoring(const QVector<QVector<int>>& adj) const;

    /** @brief Compute canonical label hash from WL coloring */
    quint64 canonicalHash(const QVector<QVector<int>>& adj) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testDone(int n1, int n2, bool iso, double timeMs);

private:
    int m_maxWLIter = 100;
    int m_wlDim = 1;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 1-dimensional WL refinement */
    QVector<int> wl1Refine(const QVector<QVector<int>>& adj) const;

    /** @brief 2-dimensional WL refinement */
    QVector<QVector<int>> wl2Refine(const QVector<QVector<int>>& adj) const;

    /** @brief Compress multiset of neighbor colors into new color */
    int compressColors(const QVector<int>& colors,
                        QHash<QString, int>& colorMap, int& nextColor) const;

    /** @brief Try to find isomorphism mapping via backtracking */
    QVector<int> findMapping(const QVector<QVector<int>>& adj1,
                              const QVector<QVector<int>>& adj2,
                              const QVector<int>& color1,
                              const QVector<int>& color2) const;

    /** @brief Verify mapping correctness */
    bool verifyMapping(const QVector<QVector<int>>& adj1,
                        const QVector<QVector<int>>& adj2,
                        const QVector<int>& mapping) const;
};
