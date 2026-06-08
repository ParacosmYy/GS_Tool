/**
 * @file MaximumClique4.h
 * @brief 最大团(位并行Bron-Kerbosch枢轴+退化序密集图剪枝) — Maximum Clique with Bit-Parallel Bron-Kerbosch Pivot and Degeneracy Ordering for Dense Graph Pruning
 *
 * 功能: 实现位并行(bit-parallel)Bron-Kerbosch枢轴算法求最大团，
 *       通过退化序(degeneracy ordering)预处理对密集图进行高效剪枝。
 *
 * 协作: GraphColoring5(图着色) / MaximumMatching6(最大匹配) / GraphIsomorphism4(图同构)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 最大团(位并行Bron-Kerbosch+退化序)
 */
class MaximumClique4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxCliqueSize = 0;
        int numBacktracks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumClique4(QObject *parent = nullptr);
    ~MaximumClique4() override;

    /** @brief Set graph as adjacency matrix */
    void setGraph(const QVector<QVector<int>>& adjacencyMatrix);

    /** @brief Set graph as edge list */
    void setGraph(int vertices, const QVector<QPair<int, int>>& edges);

    /** @brief Find maximum clique */
    QVector<int> solve();

    /** @brief Compute degeneracy ordering of the graph */
    QVector<int> degeneracyOrdering() const;

    /** @brief Get graph density (edges / max_edges) */
    double density() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cliqueFound(int size, double timeMs);

private:
    int m_numVertices = 0;
    // Adjacency bitmasks: m_adjBit[i] has bit j set if edge (i,j) exists
    QVector<quint64> m_adjBit;
    QVector<QVector<int>> m_adjList;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Best clique found
    QVector<int> m_bestClique;
    int m_bestSize = 0;

    /** @brief Recursive bit-parallel Bron-Kerbosch with pivot */
    void bkSearch(quint64 R, quint64 P, quint64 X, int depth);

    /** @brief Count bits set in a 64-bit integer */
    static int popcount(quint64 v);

    /** @brief Choose pivot from P ∪ X (Tomita heuristic) */
    int choosePivot(quint64 P, quint64 X) const;

    /** @brief Convert bitmask to vertex list */
    QVector<int> bitsetToVertices(quint64 mask) const;
};
