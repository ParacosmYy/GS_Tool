/**
 * @file ChinesePostman3.h
 * @brief 中国邮路算法(Edmonds奇度顶点最大匹配+多重图边处理) — Chinese Postman with Edmonds Maximum Matching on Odd-Degree Vertices and Multi-Graph Edge Handling
 *
 * 功能: 实现中国邮路算法，支持Edmonds最大权匹配、
 *       奇度顶点配对优化和多重图边处理。
 *
 * 协作: Dijkstra9(Dijkstra最短路) / FloydWarshall7(Floyd最短路) / EulerPath4(欧拉路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 中国邮路算法(Edmonds奇度顶点最大匹配+多重图边处理)
 */
class ChinesePostman3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge representation */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
        bool isDuplicate = false;
    };

    explicit ChinesePostman3(QObject *parent = nullptr);
    ~ChinesePostman3() override;

    /** @brief Add edge to the graph */
    void addEdge(int from, int to, double weight);

    /** @brief Set vertex count */
    void setVertexCount(int n);

    /** @brief Solve Chinese postman tour */
    QVector<int> solve();

    /** @brief Compute total tour cost */
    double tourCost() const;

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices() const;

    /** @brief Edmonds maximum matching on odd vertices */
    QVector<QPair<int, int>> edmondsMatching(const QVector<int>& oddVerts);

    /** @brief Duplicate edges along shortest paths for matched pairs */
    void duplicateMatchedEdges(const QVector<QPair<int, int>>& matches);

    /** @brief Find Euler tour in augmented graph */
    QVector<int> eulerTour() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int vertices, int edges, double cost, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<QVector<double>> m_dist;     // Floyd-Warshall distances
    QVector<QVector<int>> m_next;        // Path reconstruction
    QVector<QVector<int>> m_adjMulti;    // Multi-graph adjacency
    double m_tourCost = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Floyd-Warshall all-pairs shortest path */
    void floydWarshall();

    /** @brief Reconstruct shortest path between u and v */
    QVector<int> reconstructPath(int u, int v) const;

    /** @brief Hierholzer's algorithm for Euler tour */
    QVector<int> hierholzer() const;
};
