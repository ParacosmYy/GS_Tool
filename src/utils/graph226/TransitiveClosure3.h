/**
 * @file TransitiveClosure3.h
 * @brief 传递闭包(Floyd-Warshall位并行优化+可达性位图压缩) — Transitive Closure with Floyd-Warshall Bit-Parallel Optimization and Reachability Bitmap Compression
 *
 * 功能: 实现传递闭包计算，支持Floyd-Warshall位并行优化、
 *       可达性位图压缩和批量查询。
 *
 * 协作: StronglyConnected4(强连通分量) / ShortestPath5(最短路径) / TopologicalSort4(拓扑排序)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 传递闭包(Floyd-Warshall位并行优化+可达性位图压缩)
 */
class TransitiveClosure3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double reachabilityDensity = 0.0;
        int compressedSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransitiveClosure3(QObject *parent = nullptr);
    ~TransitiveClosure3() override;

    /** @brief Build from adjacency list */
    void build(const QVector<QVector<int>>& adjList);

    /** @brief Build from edge list */
    void buildFromEdges(const QVector<QPair<int, int>>& edges, int numVertices);

    /** @brief Check if vertex u can reach vertex v */
    bool canReach(int u, int v) const;

    /** @brief Get all vertices reachable from u */
    QVector<int> reachableFrom(int u) const;

    /** @brief Get all vertices that can reach v */
    QVector<int> reachingTo(int v) const;

    /** @brief Compute reachability bitmap for vertex u */
    QVector<quint64> reachabilityBitmap(int u) const;

    /** @brief Batch query: check multiple (u,v) pairs */
    QVector<bool> batchQuery(const QVector<QPair<int, int>>& queries) const;

    /** @brief Get reachability matrix (as bit-parallel rows) */
    QVector<QVector<quint64>> closureMatrix() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void closureComputed(int vertices, double density, double timeMs);

private:
    int m_n = 0;
    int m_wordsPerRow = 0;  // Number of quint64 words per row

    // Bit-parallel reachability matrix
    QVector<QVector<quint64>> m_reach;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Floyd-Warshall with bit-parallel optimization */
    void floydWarshallBitParallel();

    /** @brief Set bit in bitmap */
    static void setBit(QVector<quint64>& bitmap, int pos);

    /** @brief Test bit in bitmap */
    static bool testBit(const QVector<quint64>& bitmap, int pos);

    /** @brief Count set bits in bitmap */
    static int popCount(const QVector<quint64>& bitmap);
};
