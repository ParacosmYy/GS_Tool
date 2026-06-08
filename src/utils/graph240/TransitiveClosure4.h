/**
 * @file TransitiveClosure4.h
 * @brief 传递闭包(BFS位集传播+压缩稀疏行可达性索引) — Transitive Closure with BFS-Based Bitset Propagation and Compressed Sparse Row Reachability Indexing
 *
 * 功能: 实现有向图的传递闭包计算，使用BFS驱动的位集传播算法，
 *       并构建压缩稀疏行(CSR)格式的可达性索引支持高效查询。
 *
 * 协作: TopologicalSort5(拓扑排序) / SCC6(强连通分量) / ShortestPath7(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QBitArray>

/**
 * @brief 传递闭包(BFS位集传播+CSR可达性索引)
 */
class TransitiveClosure4 : public QObject {
    Q_OBJECT

public:
    /** @brief Reachability query result */
    struct ReachResult {
        bool reachable = false;
        int hopCount = -1;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numBfsRuns = 0;
        double avgProcessingTimeMs = 0.0;
        double indexBuildTimeMs = 0.0;
    };

    explicit TransitiveClosure4(QObject *parent = nullptr);
    ~TransitiveClosure4() override;

    /** @brief Build graph from edge list */
    void buildGraph(int numVertices,
                    const QVector<QPair<int, int>>& edges);

    /** @brief Compute full transitive closure via BFS bitset propagation */
    void computeClosure();

    /** @brief Query reachability (u -> v) */
    ReachResult query(int u, int v) const;

    /** @brief Get all vertices reachable from u */
    QVector<int> reachableSet(int u) const;

    /** @brief Build CSR reachability index for fast queries */
    void buildCSRIndex();

    /** @brief Fast reachability check using CSR index */
    bool csrReachable(int u, int v) const;

    int vertexCount() const { return m_numVertices; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void closureComputed(int vertices, int reachablePairs, double timeMs);

private:
    int m_numVertices = 0;

    // Adjacency lists
    QVector<QVector<int>> m_adj;

    // Transitive closure: bitset per vertex (row u has bit v set if u->v)
    QVector<QBitArray> m_closure;

    // CSR reachability index
    QVector<int> m_csrRowStart;  // row pointers
    QVector<int> m_csrTargets;   // target vertices (sorted per row)

    // Hop counts for BFS
    QVector<QVector<int>> m_hopDist;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief BFS from source, propagate bitset */
    QBitArray bfsBitset(int source, QVector<int>& hops) const;
};
