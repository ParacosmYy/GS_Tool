/**
 * @file ChinesePostman7.h
 * @brief 中国邮路问题(Hierholzer回路构造+最小权奇度顶点匹配) — Chinese Postman with Hierholzer Circuit Construction and Minimum-Weight Odd-Degree Vertex Matching
 *
 * 功能: 实现中国邮路问题(Chinese postman problem)求解器，采用Hierholzer回路构造
 *       (Hierholzer circuit construction)结合最小权奇度顶点匹配(minimum-weight odd-degree
 *       vertex matching)算法，求解无向/有向图的最短欧拉闭路。
 *
 * 协作: Dijkstra6(最短路径) / EulerPath5(欧拉路径) / GraphColoring8(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Hierholzer回路构造+最小权奇度顶点匹配)
 */
class ChinesePostman7 : public QObject {
    Q_OBJECT

public:
    /** @brief Weighted edge */
    struct Edge {
        int from = -1;
        int to = -1;
        double weight = 1.0;
        int index = -1;     // original edge index
        bool directed = false;
    };

    /** @brief Tour result */
    struct Tour {
        QVector<int> edgeSequence;   // edge indices in tour order
        QVector<int> vertexSequence; // vertex visit order
        double totalCost = 0.0;
        int numDuplicatedEdges = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double tourCost = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman7(QObject *parent = nullptr);
    ~ChinesePostman7() override;

    /** @brief Set directed graph mode */
    void setDirected(bool directed);

    /** @brief Add edge to the graph */
    void addEdge(int from, int to, double weight = 1.0);

    /** @brief Build graph from adjacency list */
    void buildGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief Solve and return the optimal tour */
    Tour solve();

    /** @brief Check if graph is Eulerian (all even degree for undirected) */
    bool isEulerian() const;

    /** @brief Get odd-degree vertices */
    QVector<int> oddDegreeVertices() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void matchingCompleted(int pairs, double cost);
    void tourCompleted(double cost, double timeMs);

private:
    bool m_directed = false;
    int m_numVertices = 0;
    QVector<Edge> m_edges;

    // Adjacency: m_adj[v] = list of edge indices
    QVector<QVector<int>> m_adj;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find shortest paths between all odd-degree vertices (Dijkstra) */
    QVector<QVector<double>> shortestPaths(const QVector<int>& sources) const;

    /** @brief Minimum-weight perfect matching on odd-degree vertices */
    QVector<QPair<int, int>> minWeightMatching(
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& dist) const;

    /** @brief Duplicate edges along shortest paths */
    void duplicateEdges(const QVector<QPair<int, int>>& pairs,
                        const QVector<int>& oddVerts,
                        const QVector<QVector<double>>& dist);

    /** @brief Reconstruct shortest path (for edge duplication) */
    QVector<int> reconstructPath(int from, int to) const;

    /** @brief Hierholzer algorithm for Eulerian circuit */
    QVector<int> hierholzer();

    /** @brief Dijkstra from source */
    QVector<double> dijkstra(int source) const;
};
