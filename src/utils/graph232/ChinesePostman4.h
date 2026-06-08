/**
 * @file ChinesePostman4.h
 * @brief 中国邮路问题(Edmonds-Johnson匹配+邮路边复制) — Chinese Postman with Edmonds-Johnson Matching on General Graphs and Postman Tour Edge Duplication
 *
 * 功能: 实现中国邮路问题求解，采用Edmonds-Johnson算法进行
 *       一般图最小权完美匹配，通过边复制构造欧拉邮路。
 *
 * 协作: EulerianPath3(欧拉路径) / MaxFlow3(最大流) / GraphShortestPath4(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Edmonds-Johnson匹配+邮路边复制)
 */
class ChinesePostman4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double totalTourCost = 0.0;
        double matchingCost = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge representation */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
    };

    explicit ChinesePostman4(QObject *parent = nullptr);
    ~ChinesePostman4() override;

    /** @brief Set graph: vertices and edges */
    void setGraph(int vertices, const QVector<Edge>& edges);

    /** @brief Solve Chinese Postman, returns tour vertex sequence */
    QVector<int> solve();

    /** @brief Compute shortest paths between all odd-degree vertices */
    QVector<QVector<double>> oddShortestPaths() const;

    /** @brief Get minimum weight perfect matching cost */
    double matchingCost() const;

    /** @brief Check if graph is Eulerian */
    bool isEulerian() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(double tourCost, double matchCost, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<QVector<QPair<int, double>>> m_adj;  // adjacency list

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_matchingCost = 0.0;

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices() const;

    /** @brief Floyd-Warshall all-pairs shortest paths */
    QVector<QVector<double>> floydWarshall() const;

    /** @brief Edmonds-Johnson minimum weight perfect matching */
    QVector<QPair<int, int>> minWeightMatching(
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& dist);

    /** @brief Find Euler tour via Hierholzer */
    QVector<int> eulerTour(
        const QVector<QVector<QPair<int, double>>>& augmentedAdj) const;
};
