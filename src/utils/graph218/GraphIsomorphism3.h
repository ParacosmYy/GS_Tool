/**
 * @file GraphIsomorphism3.h
 * @brief 图同构检测(个体化-精炼+规范增广+自同构群) — Graph Isomorphism via Individualization-Refinement with Canonical Augmentation and Automorphism Group Computation
 *
 * 功能: 实现图同构检测，支持个体化-精炼算法、
 *       规范增广和自同构群计算。
 *
 * 协作: MaxClique8(最大团) / GraphColoring5(图着色) / HamiltonianCycle6(哈密顿回路)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构检测(个体化-精炼+规范增广+自同构群)
 */
class GraphIsomorphism3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalChecks = 0;
        int numVertices = 0;
        int automorphismGroupSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism3(QObject *parent = nullptr);
    ~GraphIsomorphism3() override;

    /** @brief Check if two adjacency matrices represent isomorphic graphs */
    bool isIsomorphic(const QVector<QVector<int>>& g1, const QVector<QVector<int>>& g2);

    /** @brief Compute canonical labeling of a graph */
    QVector<int> canonicalLabel(const QVector<QVector<int>>& graph) const;

    /** @brief Refine coloring by propagating neighbor color constraints */
    QVector<int> refineColoring(const QVector<QVector<int>>& graph,
                                 const QVector<int>& coloring) const;

    /** @brief Individualize a vertex: assign unique color */
    QVector<int> individualize(const QVector<int>& coloring, int vertex) const;

    /** @brief Search tree with individualization-refinement */
    void searchTree(const QVector<QVector<int>>& graph,
                     const QVector<int>& coloring,
                     QVector<QVector<int>>& canonicalCandidates) const;

    /** @brief Compute automorphism group generators */
    QVector<QVector<int>> automorphismGenerators(const QVector<QVector<int>>& graph) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void isomorphismCheckCompleted(bool result, int vertices, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compare two colorings lexicographically */
    static bool coloringLess(const QVector<int>& a, const QVector<int>& b);

    /** @brief Count vertices with each color */
    static QVector<int> colorHistogram(const QVector<int>& coloring, int numColors);

    /** @brief Check if coloring is discrete (all colors unique) */
    static bool isDiscrete(const QVector<int>& coloring);

    /** @brief Find first non-trivial color class */
    static int targetColorClass(const QVector<int>& coloring);
};
