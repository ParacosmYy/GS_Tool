/**
 * @file TransitiveClosure5.h
 * @brief 传递闭包(Roy-Warshall位向量优化+压缩稀疏行可达矩阵) — Transitive Closure with Roy-Warshall Bit-Vector Optimization and Compressed-Sparse-Row Reachability Matrix
 *
 * 功能: 实现传递闭包(transitive closure)，采用Roy-Warshall位向量优化(Roy-Warshall bit-vector
 *       optimization)利用位运算并行加速矩阵乘法，通过压缩稀疏行可达矩阵(compressed-sparse-row
 *       reachability matrix)稀疏存储可达关系，实现高效图可达性查询。
 *
 * 协作: ShortestPath12(最短路径) / StrongConnect6(强连通) / TopologicalSort9(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QBitArray>

/**
 * @brief 传递闭包(Roy-Warshall位向量优化+压缩稀疏行可达矩阵)
 */
class TransitiveClosure5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransitiveClosure5(QObject *parent = nullptr);
    ~TransitiveClosure5() override;

    /** @brief Build graph with n vertices */
    void setNumVertices(int n);

    /** @brief Add directed edge u -> v */
    void addEdge(int u, int v);

    /** @brief Build from adjacency list */
    void buildFromAdjList(const QVector<QVector<int>>& adj);

    /** @brief Compute transitive closure */
    void compute();

    /** @brief Query reachability: can u reach v? */
    bool reachable(int u, int v) const;

    /** @brief Get all vertices reachable from u (CSR format) */
    QVector<int> reachableSet(int u) const;

    /** @brief Get full reachability matrix row as bit array */
    QBitArray reachabilityRow(int u) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computeCompleted(int n, int edges, double timeMs);

private:
    int m_n = 0;

    // Roy-Warshall bit-vector matrix: m_matrix[u][block]
    QVector<QVector<quint64>> m_matrix;
    int m_blocks = 0;

    // CSR reachability storage
    QVector<int> m_csrOffsets;
    QVector<int> m_csrTargets;
    bool m_csrValid = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Set bit in matrix[u][v] */
    void setBit(int u, int v);

    /** @brief Test bit in matrix[u][v] */
    bool testBit(int u, int v) const;

    /** @brief Build CSR from bit-vector matrix */
    void buildCSR();
};
