/**
 * @file TransitiveClosure7.h
 * @brief 传递闭包(Floyd-Warshall位向量优化与Warshall位并行可达性计算) — Transitive Closure with Floyd-Warshall Bit-Vector Optimization and Warshall's Bit-Parallel Reachability Computation
 *
 * 功能: 实现传递闭包(Transitive closure)，采用Floyd-Warshall位向量优化(Floyd-Warshall
 *       bit-vector optimization)与Warshall位并行可达性计算(Warshall's bit-parallel
 *       reachability computation)。
 *
 * 协作: ShortestPath9(最短路径) / TopologicalSort8(拓扑排序) / StrongConnect7(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 传递闭包(Floyd-Warshall位向量优化与Warshall位并行可达性计算)
 */
class TransitiveClosure7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numReachable = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransitiveClosure7(QObject *parent = nullptr);
    ~TransitiveClosure7() override;

    /** @brief Build from adjacency matrix (n x n, 0/1 values) */
    void setAdjacencyMatrix(const QVector<QVector<int>>& matrix);

    /** @brief Build from edge list (pairs of vertex indices) */
    void setEdgeList(int numVertices, const QVector<QPair<int, int>>& edges);

    /** @brief Compute transitive closure using Floyd-Warshall */
    QVector<QVector<int>> compute();

    /** @brief Compute using bit-vector parallel Warshall algorithm */
    QVector<QVector<int>> computeBitParallel();

    /** @brief Check if vertex u can reach vertex v */
    bool isReachable(int u, int v) const;

    /** @brief Get all vertices reachable from vertex u */
    QVector<int> reachableFrom(int u) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void closureComputed(int numVertices, int numReachable, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adjMatrix;
    QVector<QVector<int>> m_closure;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Bit-vector storage: each row stored as array of quint64
    QVector<QVector<quint64>> m_bitRows;
    int m_bitCols = 0;  // number of quint64 per row

    /** @brief Pack adjacency matrix into bit-vectors */
    void packBits();

    /** @brief Unpack bit-vectors back to int matrix */
    QVector<QVector<int>> unpackBits() const;
};
