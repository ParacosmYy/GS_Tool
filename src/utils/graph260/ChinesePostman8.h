/**
 * @file ChinesePostman8.h
 * @brief 中国邮路问题(Edmonds花算法奇度顶点最小权完美匹配) — Chinese Postman with Edmonds Blossom Algorithm for Minimum Weight Perfect Matching on Odd-Degree Vertices
 *
 * 功能: 实现中国邮路问题(Chinese Postman Problem)求解器，使用Edmonds花算法
 *       (Edmonds blossom algorithm)在奇度顶点(odd-degree vertices)上求最小权
 *       完美匹配(minimum weight perfect matching)，构造最优欧拉回路。
 *
 * 协作: Dijkstra7(最短路径) / FloydWarshall6(全源最短路) / EulerTour5(欧拉回路)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Edmonds花算法奇度顶点最小权完美匹配)
 */
class ChinesePostman8 : public QObject {
    Q_OBJECT

public:
    /** @brief Weighted edge */
    struct Edge {
        int from = -1;
        int to = -1;
        double weight = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double totalWeight = 0.0;
        double matchingWeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman8(QObject *parent = nullptr);
    ~ChinesePostman8() override;

    /** @brief Set the graph (undirected, weighted) */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Solve: find minimum cost Euler tour covering all edges */
    void solve();

    /** @brief Get the Euler tour as a sequence of vertex indices */
    QVector<int> tour() const;

    /** @brief Get the total tour cost */
    double tourCost() const;

    /** @brief Get edges added by matching (duplicated for Euler) */
    QVector<Edge> addedEdges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(double totalCost, int tourLength, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;

    // Adjacency: m_adj[v] = list of (neighbor, edge_index)
    QVector<QVector<QPair<int, int>>> m_adj;

    QVector<int> m_tour;
    QVector<Edge> m_addedEdges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices() const;

    /** @brief All-pairs shortest paths (Floyd-Warshall) */
    QVector<QVector<double>> allPairsShortestPath() const;

    /** @brief Blossom algorithm for minimum weight perfect matching */
    QVector<QPair<int, int>> blossomMatching(
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& dist) const;

    /** @brief Hierholzer's algorithm for Euler tour */
    QVector<int> eulerTour();

    /** @brief Find shortest path between two vertices (for path reconstruction) */
    QVector<int> shortestPath(int src, int dst,
                              const QVector<QVector<int>>& next) const;
};
