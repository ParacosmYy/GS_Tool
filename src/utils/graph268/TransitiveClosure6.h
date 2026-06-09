/**
 * @file TransitiveClosure6.h
 * @brief 传递闭包(Floyd-Warshall全对可达性+后继矩阵路径重建) — Transitive Closure with Floyd-Warshall and Path Reconstruction via Successor Matrix for All-Pairs Reachability
 *
 * 功能: 实现传递闭包(transitive closure)算法，使用Floyd-Warshall全对最短
 *       路径(Floyd-Warshall all-pairs shortest path)计算可达性矩阵，通过
 *       后继矩阵(successor matrix)重建任意节点对之间的最短路径。
 *
 * 协作: TopologicalSort5(拓扑排序) / Dijkstra4(Dijkstra最短路径) / BellmanFord3(Bellman-Ford)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 传递闭包(Floyd-Warshall+后继矩阵路径重建)
 */
class TransitiveClosure6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numEdges = 0;
        int numReachablePairs = 0;
        double avgPathLength = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransitiveClosure6(QObject *parent = nullptr);
    ~TransitiveClosure6() override;

    /** @brief Set directed graph as adjacency matrix (INF = no edge) */
    void setGraph(const QVector<QVector<double>>& adjMatrix);

    /** @brief Compute transitive closure, return reachability matrix */
    QVector<QVector<bool>> compute();

    /** @brief Get all-pairs shortest distance matrix */
    QVector<QVector<double>> distanceMatrix() const;

    /** @brief Reconstruct shortest path from i to j */
    QVector<int> reconstructPath(int from, int to) const;

    /** @brief Check if node j is reachable from node i */
    bool isReachable(int from, int to) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void closureComputed(int nodes, int reachablePairs, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<double>> m_dist;
    QVector<QVector<int>> m_successor;  // Successor matrix for path reconstruction
    QVector<QVector<bool>> m_reachability;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_pathSum = 0.0;
    int m_pathCount = 0;

    static constexpr double INF = 1e18;

    /** @brief Initialize successor matrix from adjacency matrix */
    void initSuccessor();
};
