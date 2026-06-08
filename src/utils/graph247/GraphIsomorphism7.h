/**
 * @file GraphIsomorphism7.h
 * @brief 图同构(度序列过滤+暴力排列搜索与规范证书) — Graph Isomorphism with Degree-Sequence Filtering and Brute-Force Permutation Search with Canonical Certificate
 *
 * 功能: 实现图同构(graph isomorphism)检测，采用度序列过滤(degree-sequence filtering)
 *       进行快速排除，再通过暴力排列搜索(brute-force permutation search)和规范证书
 *       (canonical certificate)精确验证。
 *
 * 协作: GraphColoring5(图着色) / ShortestPath6(最短路径) / MaxFlow4(最大流)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(度序列过滤+暴力排列搜索与规范证书)
 */
class GraphIsomorphism7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int permutationsTested = 0;
        int filteredByDegree = 0;
        bool lastResult = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism7(QObject *parent = nullptr);
    ~GraphIsomorphism7() override;

    /** @brief Check if two adjacency matrices represent isomorphic graphs */
    bool isIsomorphic(const QVector<QVector<int>>& adjA,
                      const QVector<QVector<int>>& adjB);

    /** @brief Compute canonical certificate string for a graph */
    QVector<int> canonicalCertificate(const QVector<QVector<int>>& adj) const;

    /** @brief Compute degree sequence */
    QVector<int> degreeSequence(const QVector<QVector<int>>& adj) const;

    /** @brief Find isomorphism mapping (returns permutation A->B, empty if not iso) */
    QVector<int> findMapping(const QVector<QVector<int>>& adjA,
                              const QVector<QVector<int>>& adjB);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void comparisonCompleted(int permutations, bool result, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Check if permutation makes adjA equal to adjB */
    bool checkPermutation(const QVector<QVector<int>>& adjA,
                           const QVector<QVector<int>>& adjB,
                           const QVector<int>& perm) const;

    /** @brief Generate next permutation in-place */
    bool nextPermutation(QVector<int>& perm) const;

    /** @brief Compute degree sequence sorted descending */
    QVector<int> sortedDegrees(const QVector<QVector<int>>& adj) const;

    /** @brief Refine candidate permutations using degree constraints */
    QVector<QVector<int>> degreeFilteredPerms(
        const QVector<int>& degA, const QVector<int>& degB) const;
};
