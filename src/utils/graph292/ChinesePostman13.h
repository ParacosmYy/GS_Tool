/**
 * @file ChinesePostman13.h
 * @brief 中国邮路问题(Edmonds花算法最小权匹配与有向图邮路遍历构造) — Chinese Postman with Edmonds' Blossom for Minimum-weight Matching and Postman Walk Construction for Directed Graphs
 *
 * 功能: 实现中国邮路问题(Chinese postman problem)，采用Edmonds花算法(Edmonds' blossom algorithm)
 *       最小权匹配(minimum-weight matching)与有向图邮路遍历构造(postman walk construction for directed graphs)。
 *
 * 协作: Dijkstra10(最短路径) / FloydWarshall9(全源最短路) / EulerianTrail12(欧拉路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 中国邮路问题(Edmonds花算法最小权匹配与有向图邮路遍历构造)
 */
class ChinesePostman13 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the graph */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 1.0;
        bool isOriginal = true;    // True = original, False = duplicated
    };

    /** @brief Postman tour result */
    struct TourResult {
        QVector<int> tour;          // Vertex sequence of the postman tour
        double totalCost = 0.0;
        int numOriginalEdges = 0;
        int numDuplicatedEdges = 0;
        bool isValid = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman13(QObject *parent = nullptr);
    ~ChinesePostman13() override;

    /** @brief Set graph as edge list */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Solve the Chinese Postman Problem */
    TourResult solve();

    /** @brief Find odd-degree vertices */
    QVector<int> findOddDegreeVertices() const;

    /** @brief Compute shortest paths between all pairs (Floyd-Warshall) */
    void computeAllPairsShortestPaths();

    /** @brief Get shortest path distance between u and v */
    double shortestPathDist(int u, int v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int tourLength, double cost, int oddCount, double timeMs);

private:
    int m_numVertices = 0;
    QVector<Edge> m_edges;

    // All-pairs shortest paths
    QVector<QVector<double>> m_dist;
    QVector<QVector<int>> m_next;
    bool m_apspComputed = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Minimum weight perfect matching (greedy for odd vertices) */
    QVector<QPair<int, int>> minimumWeightMatching(const QVector<int>& oddVertices);

    /** @brief Reconstruct path from u to v */
    QVector<int> reconstructPath(int u, int v) const;

    /** @brief Build Eulerian tour using Hierholzer's algorithm */
    QVector<int> buildEulerianTour(const QVector<Edge>& augmentedEdges);

    /** @brief Compute degree of vertex considering only original+augmented edges */
    QVector<int> computeDegrees(const QVector<Edge>& edgeList) const;
};
