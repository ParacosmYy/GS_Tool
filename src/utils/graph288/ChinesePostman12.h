/**
 * @file ChinesePostman12.h
 * @brief 中国邮路问题(Kolmogorov Blossom最小权完美匹配与混合图邮路构造) — Chinese Postman with Minimum-weight Perfect Matching via Kolmogorov Blossom and Postman Tour Construction for Mixed Graphs
 *
 * 功能: 实现中国邮路问题(Chinese postman)，采用Kolmogorov Blossom最小权完美匹配(minimum-weight perfect matching)
 *       与混合图邮路构造(postman tour construction for mixed graphs)。
 *
 * 协作: EulerPath11(欧拉路径) / Dijkstra15(最短路径) / FloydWarshall9(Floyd-Warshall)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Kolmogorov Blossom最小权完美匹配与混合图邮路构造)
 */
class ChinesePostman12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double tourWeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge representation */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 1.0;
        bool directed = false;
    };

    explicit ChinesePostman12(QObject *parent = nullptr);
    ~ChinesePostman12() override;

    /** @brief Set graph as adjacency list with edge weights */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Compute Chinese postman tour, returns vertex sequence */
    QVector<int> computeTour();

    /** @brief Get total tour weight */
    double tourWeight() const;

    /** @brief Get list of odd-degree vertices */
    QVector<int> oddVertices() const;

    /** @brief Get the duplicated edges for the augmentation */
    QVector<Edge> augmentedEdges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourComputed(int numVertices, double tourWeight, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<QVector<double>> m_adjMatrix;   // All-pairs shortest path
    QVector<int> m_oddVerts;
    QVector<Edge> m_augEdges;
    double m_tourWeight = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute all-pairs shortest paths (Floyd-Warshall) */
    void floydWarshall();

    /** @brief Find odd-degree vertices in undirected subgraph */
    void findOddVertices();

    /** @brief Blossom algorithm for minimum-weight perfect matching on odd vertices */
    QVector<int> blossomMatching();

    /** @brief Find Euler tour on augmented graph (Hierholzer) */
    QVector<int> eulerTour();
};
