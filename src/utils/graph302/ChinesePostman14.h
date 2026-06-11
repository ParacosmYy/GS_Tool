/**
 * @file ChinesePostman14.h
 * @brief 中国邮路问题(Edmonds奇度匹配与权重无向Euler游览的环游拼接) — Chinese Postman with Edmonds Matching on Odd-degree Subgraph and Tour Concatenation for Weighted Undirected Euler Tour
 *
 * 功能: 实现中国邮路问题(Chinese Postman)，采用Edmonds奇度匹配(Edmonds matching)
 *       与环游拼接(tour concatenation)实现权重无向Euler游览(weighted undirected Euler tour)。
 *
 * 协作: EulerTour12(Euler游览) / Dijkstra7(最短路) / FloydWarshall6(Floyd-Warshall)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 中国邮路问题(Edmonds奇度匹配与权重无向Euler游览的环游拼接)
 */
class ChinesePostman14 : public QObject {
    Q_OBJECT

public:
    /** @brief Weighted edge */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 1.0;
        bool duplicated = false;      // Edge added for Eulerian completion
    };

    /** @brief Tour result */
    struct TourResult {
        QVector<int> tour;            // Vertex sequence of the Euler tour
        double totalCost = 0.0;
        double matchingCost = 0.0;
        int originalEdges = 0;
        int duplicatedEdges = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int oddDegreeVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman14(QObject *parent = nullptr);
    ~ChinesePostman14() override;

    /** @brief Add an undirected weighted edge */
    void addEdge(int u, int v, double weight);

    /** @brief Set vertex count (must be called before solve) */
    void setVertexCount(int n);

    /** @brief Solve the Chinese Postman Problem */
    TourResult solve();

    /** @brief Check if graph is Eulerian (all even degrees) */
    bool isEulerian() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourFound(int vertices, int edges, double cost, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute vertex degrees */
    QVector<int> computeDegrees() const;

    /** @brief Find odd-degree vertices */
    QVector<int> oddDegreeVertices() const;

    /** @brief Floyd-Warshall all-pairs shortest paths */
    QVector<QVector<double>> allPairsShortest(QVector<QVector<int>>& next) const;

    /** @brief Edmonds-style minimum weight perfect matching ( Blossom simplified via DP ) */
    QVector<QPair<int,int>> minWeightMatching(const QVector<int>& oddVerts,
                                               const QVector<QVector<double>>& dist) const;

    /** @brief Recursive DP matching for small sets */
    double matchingDP(int mask, const QVector<int>& odd,
                      const QVector<QVector<double>>& dist,
                      QVector<QVector<double>>& memo) const;

    /** @brief Reconstruct matching from DP */
    void reconstructMatching(int mask, const QVector<int>& odd,
                              const QVector<QVector<double>>& dist,
                              const QVector<QVector<double>>& memo,
                              QVector<QPair<int,int>>& pairs) const;

    /** @brief Build adjacency list and find Euler tour (Hierholzer) */
    QVector<int> eulerTour(int start);

    /** @brief Shortest path between matched pair using next-hop table */
    QVector<QPair<int,int>> shortestPathEdges(int u, int v,
                                               const QVector<QVector<int>>& next) const;
};
