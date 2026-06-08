/**
 * @file TravelingSalesman6.h
 * @brief 旅行商问题(Christofides 1.5-近似算法+度量空间最短路径构造) — TSP with Christofides 1.5-Approximation and Shortcutting for Metric Space Optimal Tour Construction
 *
 * 功能: 实现旅行商问题(TSP, Traveling Salesman Problem)的Christofides 1.5-近似算法
 *       (Christofides 1.5-approximation)，包含最小生成树(minimum spanning tree)、
 *       最小完美匹配(minimum weight perfect matching)与度量空间捷径优化(metric shortcutting)。
 *
 * 协作: Dijkstra7(最短路径) / MinimumSpanningTree5(最小生成树) / AStarSearch6(A*搜索)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 旅行商问题(Christofides 1.5-近似+度量空间捷径优化)
 */
class TravelingSalesman6 : public QObject {
    Q_OBJECT

public:
    /** @brief Tour result */
    struct TourResult {
        QVector<int> tour;       // ordered vertex indices
        double totalDistance = 0.0;
        int numVertices = 0;
    };

    /** @brief Edge for MST/matching */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int mstEdges = 0;
        int matchingEdges = 0;
        double approximationRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman6(QObject *parent = nullptr);
    ~TravelingSalesman6() override;

    /** @brief Set distance matrix [n x n] (symmetric, metric) */
    void setDistanceMatrix(const QVector<QVector<double>>& dist);

    /** @brief Set vertex coordinates (for Euclidean TSP) */
    void setCoordinates(const QVector<QVector<double>>& coords);

    /** @brief Solve TSP using Christofides algorithm */
    TourResult solve();

    /** @brief Compute tour distance for a given tour */
    double tourDistance(const QVector<int>& tour) const;

    /** @brief Get minimum spanning tree edges */
    QVector<Edge> mstEdges() const;

    /** @brief Get minimum perfect matching edges */
    QVector<Edge> matchingEdges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mstComputed(int edges, double totalWeight);
    void matchingComputed(int edges, double totalWeight);
    void tourCompleted(int vertices, double distance, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<double>> m_dist;  // distance matrix
    QVector<QVector<double>> m_coords; // vertex coordinates

    QVector<Edge> m_mstEdges;
    QVector<Edge> m_matchEdges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute MST using Prim's algorithm */
    QVector<Edge> computeMST() const;

    /** @brief Find odd-degree vertices in MST */
    QVector<int> findOddDegreeVertices(const QVector<Edge>& mst) const;

    /** @brief Compute minimum weight perfect matching (greedy) */
    QVector<Edge> computeMinMatching(const QVector<int>& oddVerts) const;

    /** @brief Find Eulerian tour in multigraph (Hierholzer) */
    QVector<int> eulerianTour(const QVector<Edge>& allEdges) const;

    /** @brief Shortcut Eulerian tour to Hamiltonian cycle */
    QVector<int> shortcutTour(const QVector<int>& eulerTour) const;

    /** @brief Compute Euclidean distance between two points */
    double euclideanDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Build distance matrix from coordinates */
    void buildDistanceMatrix();
};
