/**
 * @file ChinesePostman5.h
 * @brief 中国邮路问题(Blossom V最小权完美匹配奇度子图) — Chinese Postman with Blossom V Matching for Minimum-Weight Perfect Matching on Odd-Degree Subgraph
 *
 * 功能: 实现中国邮路问题的求解，使用Blossom V算法在奇度顶点子图上
 *       求最小权完美匹配，构造欧拉化后的最短遍历路径。
 *
 * 协作: Dijkstra6(最短路) / FloydWarshall4(全源最短路) / EulerPath3(欧拉路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路(Blossom V匹配+欧拉化)
 */
class ChinesePostman5 : public QObject {
    Q_OBJECT

public:
    /** @brief Edge in the graph */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
        bool isDuplicated = false;
    };

    /** @brief Tour result */
    struct TourResult {
        QVector<int> tour;        // vertex sequence
        QVector<Edge> edges;      // original + duplicated edges
        double totalCost = 0.0;
        int numDuplicated = 0;
        bool isEulerian = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        int numMatchedPairs = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman5(QObject *parent = nullptr);
    ~ChinesePostman5() override;

    /** @brief Build graph from edge list */
    void buildGraph(int vertices, const QVector<Edge>& edges);

    /** @brief Solve Chinese postman and return optimal tour */
    TourResult solve();

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices() const;

    /** @brief Blossom V minimum-weight perfect matching */
    QVector<QPair<int,int>> blossomVMatch(
        const QVector<int>& oddVerts) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourCompleted(double cost, int duplicatedEdges, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<QVector<double>> m_adjMatrix;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief All-pairs shortest path via Floyd-Warshall */
    void floydWarshall(QVector<QVector<double>>& dist) const;

    /** @brief Build complete graph on odd vertices with shortest path weights */
    QVector<QVector<double>> buildOddSubgraph(
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& dist) const;

    /** @brief Greedy minimum-weight perfect matching (Blossom V simplified) */
    QVector<QPair<int,int>> minWeightPerfectMatch(
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& weights) const;

    /** @brief Find Euler tour via Hierholzer's algorithm */
    QVector<int> hierholzerEuler(
        const QVector<QVector<QPair<int,double>>>& multigraph) const;

    /** @brief Build adjacency from edges + duplicated edges */
    void buildMultigraph(
        const QVector<QPair<int,int>>& matched,
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& dist,
        QVector<QVector<QPair<int,double>>>& mg) const;
};
