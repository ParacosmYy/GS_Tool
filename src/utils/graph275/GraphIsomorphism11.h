/**
 * @file GraphIsomorphism11.h
 * @brief 图同构(度序列指纹排序邻接编码紧致规范形式) — Graph Isomorphism with Degree-sequence Fingerprint and Compact Canonical Form via Sorted Adjacency Encoding
 *
 * 功能: 实现图同构检测(graph isomorphism)，采用度序列指纹(degree-sequence
 *       fingerprint)和排序邻接编码(sorted adjacency encoding)构建紧致
 *       规范形式(compact canonical form)进行高效的图比较与同构判定。
 *
 * 协作: TopologicalSort8(拓扑排序) / BipartiteMatch9(二分图匹配) / Dijkstra9(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图同构(度序列指纹排序邻接编码紧致规范形式)
 */
class GraphIsomorphism11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numGraphsCompared = 0;
        int numIsomorphicPairs = 0;
        int numVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Graph represented as adjacency list */
    struct Graph {
        int numVertices = 0;
        bool directed = false;
        QVector<QVector<int>> adjacency;  // adjacency[v] = neighbors of v
    };

    explicit GraphIsomorphism11(QObject *parent = nullptr);
    ~GraphIsomorphism11() override;

    /** @brief Check if two graphs are isomorphic */
    bool isIsomorphic(const Graph& g1, const Graph& g2) const;

    /** @brief Compute degree-sequence fingerprint of a graph */
    QVector<int> degreeFingerprint(const Graph& g) const;

    /** @brief Compute compact canonical form string */
    QVector<quint64> canonicalForm(const Graph& g) const;

    /** @brief Find isomorphic mapping from g1 to g2 (empty if not isomorphic) */
    QVector<int> findMapping(const Graph& g1, const Graph& g2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void comparisonCompleted(int n, bool isomorphic, double timeMs);

private:
    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Compute degree of each vertex */
    QVector<int> computeDegrees(const Graph& g) const;

    /** @brief Sort adjacency lists and encode as canonical hash */
    QVector<quint64> sortedAdjacencyEncoding(const Graph& g,
                                              const QVector<int>& vertexOrder) const;

    /** @brief Refine vertex ordering by degree partitioning */
    QVector<QVector<int>> partitionByDegree(const Graph& g) const;

    /** @brief Check isomorphism by backtracking search with pruning */
    bool backtrackIso(const Graph& g1, const Graph& g2,
                       QVector<int>& mapping, QVector<bool>& used,
                       const QVector<QVector<int>>& part1,
                       const QVector<QVector<int>>& part2) const;

    /** @brief Count edges in a graph */
    int countEdges(const Graph& g) const;

    /** @brief Simple hash combining for adjacency row */
    quint64 hashRow(const QVector<int>& neighbors, int vertex, int n) const;
};
