/**
 * @file GraphIsomorphism15.h
 * @brief 图同构(个体化-细化与广度优先搜索树认证的规范标号算法) — Graph Isomorphism with Individualization-refinement and Canonical Labeling via Breadth-first Search Tree Certification
 *
 * 功能: 实现图同构(Graph isomorphism)，采用个体化-细化(individualization-refinement)
 *       与广度优先搜索树认证(BFS tree certification)实现规范标号(canonical labeling)。
 *
 * 协作: BipartiteMatch13(二分图匹配) / MinSpanningTree14(最小生成树) / GraphColoring12(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(个体化-细化与广度优先搜索树认证的规范标号算法)
 */
class GraphIsomorphism15 : public QObject {
    Q_OBJECT

public:
    /** @brief Isomorphism test result */
    struct IsoResult {
        bool isomorphic = false;
        QVector<int> mapping;     // mapping[i] = j means node i in G1 maps to j in G2
        double timeMs = 0.0;
    };

    /** @brief Canonical form result */
    struct CanonResult {
        QVector<int> permutation; // Permutation that produces canonical form
        QVector<int> certHash;    // Integer hash of canonical adjacency
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastN = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism15(QObject *parent = nullptr);
    ~GraphIsomorphism15() override;

    /** @brief Test if two graphs are isomorphic (adjacency matrix) */
    IsoResult isIsomorphic(const QVector<QVector<int>>& g1,
                           const QVector<QVector<int>>& g2) const;

    /** @brief Compute canonical labeling of a graph */
    CanonResult canonicalLabel(const QVector<QVector<int>>& adj) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void isoTestDone(int n, bool result, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute degree sequence */
    QVector<int> degreeSequence(const QVector<QVector<int>>& adj) const;

    /** @brief Initial coloring by vertex degree */
    QVector<int> initialColoring(const QVector<QVector<int>>& adj) const;

    /** @brief Refine coloring using neighbor color multiset */
    QVector<int> refineColoring(const QVector<QVector<int>>& adj,
                                const QVector<int>& colors) const;

    /** @brief Individualization: fix a vertex to a unique color */
    QVector<int> individualize(const QVector<int>& colors, int vertex) const;

    /** @brief BFS tree certification from a root */
    QVector<int> bfsCertificate(const QVector<QVector<int>>& adj,
                                int root, const QVector<int>& colors) const;

    /** @brief Compute integer hash of adjacency under permutation */
    QVector<int> permutedAdjHash(const QVector<QVector<int>>& adj,
                                 const QVector<int>& perm) const;
};
