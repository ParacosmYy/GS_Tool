/**
 * @file ChinesePostman11.h
 * @brief 中国邮路问题(Hierholzer回路遍历与死边最小化最优欧拉环游构造) — Chinese Postman with Hierholzer Circuit Traversal and Deadhead Edge Minimization for Optimal Euler Tour Construction
 *
 * 功能: 实现中国邮路问题(Chinese postman problem)，采用Hierholzer回路遍历(Hierholzer circuit
 *       traversal)和死边最小化(deadhead edge minimization)实现最优欧拉环游构造
 *       (optimal Euler tour construction)。
 *
 * 协作: Dijkstra10(最短路径) / EulerCircuit10(欧拉回路) / FloydWarshall9(全源最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Hierholzer回路遍历与死边最小化最优欧拉环游构造)
 */
class ChinesePostman11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double totalCost = 0.0;
        double matchingCost = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge representation */
    struct Edge {
        int from;
        int to;
        double weight;
        bool isDuplicate = false;  // Added for Eulerization
    };

    explicit ChinesePostman11(QObject *parent = nullptr);
    ~ChinesePostman11() override;

    /** @brief Build graph from adjacency list (each edge given once) */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Compute optimal Chinese postman tour, returns vertex sequence */
    QVector<int> solve();

    /** @brief Get total tour cost */
    double tourCost() const;

    /** @brief Get edges that were duplicated for Eulerization */
    QVector<Edge> duplicatedEdges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourComputed(int numVertices, double totalCost, double matchingCost, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<Edge> m_duplicates;
    QVector<int> m_tour;
    double m_tourCost = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find vertices with odd degree */
    QVector<int> findOddDegreeVertices() const;

    /** @brief All-pairs shortest paths (Floyd-Warshall) */
    void floydWarshall(QVector<QVector<double>>& dist,
                       QVector<QVector<int>>& next) const;

    /** @brief Minimum weight perfect matching on odd-degree vertices (greedy) */
    QVector<QPair<int, int>> minWeightMatching(
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& dist);

    /** @brief Build Eulerized adjacency and run Hierholzer */
    QVector<int> hierholzerTour();

    /** @brief Build adjacency list from original + duplicated edges */
    QVector<QVector<QPair<int, int>>> buildAdjacency() const;
};
