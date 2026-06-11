/**
 * @file ChinesePostman15.h
 * @brief 中国邮递员问题(Christofides匹配启发式与捷径消除实现稀疏图近优邮路) — Chinese Postman with Christofides Matching Heuristic and Shortcut Elimination for Near-optimal Postman Tour on Sparse Graphs
 *
 * 功能: 实现中国邮递员问题(Chinese postman problem)，采用Christofides匹配启发式(Christofides matching heuristic)
 *       与捷径消除(shortcut elimination)实现稀疏图近优邮路(near-optimal postman tour on sparse graphs)。
 *
 * 协作: TSP14(旅行商) / EulerCircuit13(欧拉回路) / Dijkstra11(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

class ChinesePostman15 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the graph */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
        bool required = false;           // Must be traversed
    };

    /** @brief Tour result */
    struct TourResult {
        QVector<int> tour;               // Sequence of vertices
        double totalCost = 0.0;
        int numDuplicateEdges = 0;       // Extra edges added for parity
        bool isOptimal = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman15(QObject *parent = nullptr);
    ~ChinesePostman15() override;

    void setDirected(bool directed);

    /** @brief Solve Chinese postman on given graph */
    TourResult solve(int numVertices, const QVector<Edge>& edges);

    /** @brief Get all odd-degree vertices */
    QVector<int> findOddDegreeVertices(int n, const QVector<Edge>& edges) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int tourLen, double cost, double timeMs);

private:
    bool m_directed = false;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute all-pairs shortest paths (Floyd-Warshall) */
    void floydWarshall(int n, const QVector<QVector<double>>& adj,
                        QVector<QVector<double>>& dist,
                        QVector<QVector<int>>& next) const;

    /** @brief Find minimum weight perfect matching (greedy heuristic) */
    QVector<QPair<int, int>> minWeightMatching(
        const QVector<int>& oddVertices,
        const QVector<QVector<double>>& dist) const;

    /** @brief Find Euler tour using Hierholzer's algorithm */
    QVector<int> eulerTour(int n, QVector<QVector<int>>& adjList) const;

    /** @brief Shortcut elimination: remove repeated vertices */
    QVector<int> shortcutElimination(const QVector<int>& tour, int n) const;
};
