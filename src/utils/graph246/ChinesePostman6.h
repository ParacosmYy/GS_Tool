/**
 * @file ChinesePostman6.h
 * @brief 中国邮路问题(Blossom算法最小权完美匹配+层次化欧拉回路构造) — Chinese Postman with Blossom Algorithm for Minimum Weight Perfect Matching and Hierarchical Tour Construction
 *
 * 功能: 实现中国邮路问题(Chinese Postman Problem)求解器，采用Blossom算法(blossom algorithm)
 *       求解最小权完美匹配(minimum weight perfect matching)，并通过层次化欧拉回路构造(hierarchical
 *       Eulerian tour construction)生成最优遍历路径。
 *
 * 协作: Dijkstra5(最短路径) / EulerPath4(欧拉路径) / MinCostFlow3(最小费用流)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Blossom算法最小权完美匹配+层次化欧拉回路构造)
 */
class ChinesePostman6 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the graph */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 1.0;
        bool required = true;
    };

    /** @brief Tour result */
    struct TourResult {
        QVector<int> vertices;
        QVector<int> edgeIndices;
        double totalCost = 0.0;
        int numRepeatedEdges = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        int numMatchedPairs = 0;
        double matchCost = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman6(QObject *parent = nullptr);
    ~ChinesePostman6() override;

    /** @brief Build graph from edge list */
    void buildGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Solve the Chinese Postman Problem */
    TourResult solve();

    /** @brief Get shortest path distance between two vertices */
    double shortestPath(int u, int v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void matchingCompleted(int pairs, double cost);
    void tourCompleted(double totalCost, int repeatedEdges);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<QVector<double>> m_dist;  // all-pairs shortest path
    QVector<QVector<int>> m_next;     // path reconstruction

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Floyd-Warshall all-pairs shortest path */
    void floydWarshall();

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices() const;

    /** @brief Blossom algorithm for minimum weight perfect matching */
    QVector<QPair<int, int>> blossomMatch(const QVector<int>& oddVerts);

    /** @brief Hierarchical Eulerian tour construction */
    TourResult buildEulerTour(const QVector<QPair<int, int>>& matchedPairs);

    /** @brief Greedy matching fallback (for small instances) */
    QVector<QPair<int, int>> greedyMatch(const QVector<int>& oddVerts);
};
