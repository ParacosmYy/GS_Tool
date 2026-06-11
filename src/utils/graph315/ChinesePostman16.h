/**
 * @file ChinesePostman16.h
 * @brief 中国邮路问题(Blossom算法匹配与Euler游历扩展实现有向和混合图邮路) — Chinese Postman with Blossom Algorithm Matching and Euler Tour Extension for Postman Tour on Directed and Mixed Graphs
 *
 * 功能: 实现中国邮路问题(Chinese postman problem)，采用Blossom算法匹配(blossom algorithm matching)
 *       与Euler游历扩展(Euler tour extension)实现有向和混合图邮路(postman tour on directed and mixed graphs)。
 *
 * 协作: Dijkstra(最短路径) / EulerPath(欧拉路径) / FloydWarshall(全源最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

class ChinesePostman16 : public QObject {
    Q_OBJECT

public:
    /** @brief Graph edge */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
        bool directed = false;         // true for directed edges in mixed graph
    };

    /** @brief Tour result */
    struct TourResult {
        QVector<int> tour;             // Sequence of vertices
        double totalCost = 0.0;
        int numDuplicatedEdges = 0;
        bool isEulerian = false;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int lastVertexCount = 0;
        int lastEdgeCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman16(QObject *parent = nullptr);
    ~ChinesePostman16() override;

    void setGraph(int vertices, const QVector<Edge>& edges);

    /** @brief Solve undirected Chinese postman */
    TourResult solveUndirected();

    /** @brief Solve directed Chinese postman */
    TourResult solveDirected();

    /** @brief Solve mixed graph Chinese postman (heuristic) */
    TourResult solveMixed();

    /** @brief Check if graph is Eulerian */
    bool isEulerian() const;

    /** @brief Find Euler tour using Hierholzer's algorithm */
    QVector<int> findEulerTour() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(double cost, int duplicated, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<QVector<double>> m_adj;     // Adjacency matrix for shortest paths
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute all-pairs shortest paths (Floyd-Warshall) */
    QVector<QVector<double>> floydWarshall() const;

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices() const;

    /** @brief Compute vertex balance (in-degree - out-degree) for directed */
    QVector<double> vertexBalance() const;

    /** @brief Minimum weight perfect matching via blossom algorithm (simplified) */
    QVector<QPair<int, int>> minWeightMatching(const QVector<int>& vertices,
                                                const QVector<QVector<double>>& dist) const;

    /** @brief Hierholzer's Euler tour */
    QVector<int> hierholzer(const QVector<QVector<QPair<int, double>>>& adjList) const;

    /** @brief Build augmented adjacency list with duplicated edges */
    QVector<QVector<QPair<int, double>>> buildAugmentedAdjacency(
        const QVector<QPair<int, int>>& matchings) const;

    /** @brief Find shortest path between two vertices (Dijkstra) */
    QVector<int> shortestPath(int src, int dst) const;
};
