/**
 * @file TransitiveClosure9.h
 * @brief 传递闭包(Floyd-Warshall位集优化与Warshall可达性实现紧凑图连通性计算) — Transitive Closure with Floyd-Warshall Bitset Optimization and Warshall Reachability for Compact Graph Connectivity Computation
 *
 * 功能: 实现传递闭包(transitive closure)，采用Floyd-Warshall位集优化(Floyd-Warshall bitset optimization)
 *       与Warshall可达性(Warshall reachability)实现紧凑图连通性计算(compact graph connectivity computation)。
 *
 * 协作: ShortestPath12(最短路径) / TopologicalSort8(拓扑排序) / StronglyConnected8(强连通分量)
 */
#pragma once

#include <QObject>
#include <QVector>

class TransitiveClosure9 : public QObject {
    Q_OBJECT

public:
    /** @brief Closure computation result */
    struct ClosureResult {
        QVector<QVector<bool>> reachable;
        int numNodes = 0;
        int numReachablePairs = 0;
        int numComponents = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalComputations = 0;
        int graphSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransitiveClosure9(QObject *parent = nullptr);
    ~TransitiveClosure9() override;

    /** @brief Compute transitive closure from adjacency matrix */
    ClosureResult compute(const QVector<QVector<bool>>& adjMatrix);

    /** @brief Compute transitive closure from edge list */
    ClosureResult computeFromEdges(const QVector<QPair<int, int>>& edges,
                                    int numNodes);

    /** @brief Query if node u can reach node v */
    bool isReachable(int u, int v) const;

    /** @brief Get all nodes reachable from u */
    QVector<int> reachableFrom(int u) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void closureDone(int n, int pairs, int components, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Cached reachability matrix */
    QVector<QVector<bool>> m_reachable;

    /** @brief Count connected components via BFS */
    int countComponents(const QVector<QVector<bool>>& reachable, int n) const;

    /** @brief Floyd-Warshall with bitset-like optimization using word packing */
    void floydWarshallBitset(QVector<QVector<bool>>& dist, int n) const;
};
