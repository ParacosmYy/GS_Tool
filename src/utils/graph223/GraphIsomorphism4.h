/**
 * @file GraphIsomorphism4.h
 * @brief 图同构(颜色细化+个体化+紧凑规范标号) — Graph Isomorphism via Color Refinement with Individualization and Compact Canonical Labeling
 *
 * 功能: 实现图同构判定，支持颜色细化(Weisfeiler-Leman)、
 *       个体化搜索策略和紧凑规范标号生成。
 *
 * 协作: GraphColoring6(图着色) / CommunityDetection7(社区发现) / MaxClique5(最大团)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 图同构(颜色细化+个体化+紧凑规范标号)
 */
class GraphIsomorphism4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int refinementIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism4(QObject *parent = nullptr);
    ~GraphIsomorphism4() override;

    /** @brief Check if two adjacency matrices represent isomorphic graphs */
    bool isIsomorphic(const QVector<QVector<int>>& g1,
                      const QVector<QVector<int>>& g2) const;

    /** @brief Compute canonical label for a graph */
    QVector<int> canonicalLabel(const QVector<QVector<int>>& graph) const;

    /** @brief Color refinement (1-WL Weisfeiler-Leman) */
    QVector<int> colorRefine(const QVector<QVector<int>>& graph,
                              int maxIter = 100) const;

    /** @brief Find automorphism group generators */
    QVector<QVector<int>> findAutomorphisms(
        const QVector<QVector<int>>& graph) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void isomorphismChecked(bool result, int vertices, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Encode vertex neighborhood as a color signature */
    QVector<QPair<int, QVector<int>>> neighborhoodSignature(
        const QVector<QVector<int>>& graph,
        const QVector<int>& colors) const;

    /** @brief Compress color values to contiguous range */
    QVector<int> compressColors(const QVector<int>& colors) const;

    /** @brief Individualize a vertex (assign unique color) and refine */
    QVector<int> individualize(const QVector<QVector<int>>& graph,
                                const QVector<int>& colors,
                                int vertex) const;

    /** @brief Generate compact canonical string from coloring */
    QVector<int> buildCanonicalFromColors(
        const QVector<QVector<int>>& graph,
        const QVector<int>& colors) const;

    /** @brief Check if coloring is discrete (all colors unique) */
    static bool isDiscrete(const QVector<int>& colors);

    /** @brief Count cells in color partition */
    static int countCells(const QVector<int>& colors);
};
