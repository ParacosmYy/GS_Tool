/**
 * @file GraphIsomorphism9.h
 * @brief 图同构(Weisfeiler-Lehman k维细化+哈希规范标号) — Graph Isomorphism with Weisfeiler-Lehman k-Dimensional Refinement and Hash-Based Canonical Labeling
 *
 * 功能: 实现图同构检测(Graph Isomorphism)，使用Weisfeiler-Lehman k维细化
 *       (WL k-dimensional refinement)迭代更新节点颜色标签，通过哈希规范
 *       标号(hash-based canonical labeling)生成图的规范表示进行比较。
 *
 * 协作: ShortestPath6(最短路径) / MinimumSpanningTree6(最小生成树) / GraphColoring5(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QHash>

/**
 * @brief 图同构(Weisfeiler-Lehman细化+哈希规范标号)
 */
class GraphIsomorphism9 : public QObject {
    Q_OBJECT

public:
    /** @brief Adjacency list representation */
    using AdjList = QVector<QVector<int>>;

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int wlIterations = 0;
        bool lastResult = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism9(QObject *parent = nullptr);
    ~GraphIsomorphism9() override;

    /** @brief Set WL refinement iterations */
    void setMaxIterations(int iters);

    /** @brief Check if two graphs are isomorphic */
    bool isIsomorphic(const AdjList& g1, const AdjList& g2);

    /** @brief Compute canonical label for a graph */
    QString canonicalLabel(const AdjList& graph);

    /** @brief Get WL coloring after last run */
    QVector<quint64> lastColoring() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void isomorphismChecked(bool result, int iterations, double timeMs);

private:
    int m_maxIter = 50;

    QVector<quint64> m_lastColors;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief WL refinement: update colors from neighbor multiset */
    QVector<quint64> wlRefine(const AdjList& graph,
                                const QVector<quint64>& colors) const;

    /** @brief Hash a color vector into a single color */
    quint64 hashColors(const QVector<quint64>& sortedNeighborColors) const;

    /** @brief Count edges in adjacency list */
    int countEdges(const AdjList& graph) const;

    /** @brief Compute degree sequence (sorted) */
    QVector<int> degreeSequence(const AdjList& graph) const;

    /** @brief FNV-1a hash for combining values */
    static quint64 fnv1a(quint64 hash, const void* data, int len);
};
