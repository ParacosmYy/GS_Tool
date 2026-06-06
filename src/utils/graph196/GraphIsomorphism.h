/**
 * @file GraphIsomorphism.h
 * @brief 图同构检测(规范标记+度序列过滤+迭代分区精炼) — Graph Isomorphism via Canonical Labeling with Degree-sequence Filtering and Iterative Partition Refinement
 *
 * 功能: 实现图同构检测，支持度序列快速过滤、迭代分区精炼(Weisfeiler-Lehman)、
 *       规范标记(canonical labeling)生成和同构判定。
 *
 * 协作: DFS7(深度优先) / BFS6(广度优先) / TopologicalSort5(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 图同构检测器(规范标记+度序列过滤)
 */
class GraphIsomorphism : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalComparisons = 0;
        int isomorphicCount = 0;
        int filteredByDegree = 0;
        int refinementIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism(QObject *parent = nullptr);
    ~GraphIsomorphism() override;

    /** @brief 检查两个图是否同构 */
    bool isIsomorphic(const QVector<QVector<int>>& adj1, const QVector<QVector<int>>& adj2);

    /** @brief 计算度序列(排序后) */
    QVector<int> degreeSequence(const QVector<QVector<int>>& adj) const;

    /** @brief Weisfeiler-Lehman迭代分区精炼 */
    QVector<int> wlRefinement(const QVector<QVector<int>>& adj, int iterations = 10) const;

    /** @brief 生成规范标记 */
    QString canonicalLabel(const QVector<QVector<int>>& adj) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void comparisonCompleted(bool isomorphic, int vertices);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Count edges */
    int countEdges(const QVector<QVector<int>>& adj) const;

    /** @brief Compute degree of each vertex */
    QVector<int> computeDegrees(const QVector<QVector<int>>& adj) const;

    /** @brief Relabel graph by given permutation */
    QVector<QVector<int>> permuteGraph(const QVector<QVector<int>>& adj,
                                        const QVector<int>& perm) const;

    /** @brief Serialize adjacency to canonical string */
    QString serializeAdj(const QVector<QVector<int>>& adj) const;

    /** @brief Find best permutation candidates from coloring */
    QVector<QVector<int>> generateCandidates(const QVector<QVector<int>>& adj,
                                               const QVector<int>& coloring) const;

    /** @brief Check adjacency equality */
    bool adjEqual(const QVector<QVector<int>>& a, const QVector<QVector<int>>& b) const;
};
