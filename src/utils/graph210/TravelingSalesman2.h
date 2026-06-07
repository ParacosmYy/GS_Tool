/**
 * @file TravelingSalesman2.h
 * @brief 旅行商问题(Christofides 3/2近似+奇度顶点最小权完美匹配) — TSP with Christofides 3/2-Approximation via Minimum-Weight Perfect Matching on Odd-Degree Vertices
 *
 * 功能: 实现TSP的Christofides 3/2-近似算法，支持
 *       最小生成树、奇度顶点最小权完美匹配和欧拉回路。
 *
 * 协作: Dijkstra6(最短路径) / MinimumSpanningTree3(最小生成树) / Hungarian6(匈牙利算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 旅行商问题(Christofides 3/2近似)
 */
class TravelingSalesman2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int numCities = 0;
        double tourCost = 0.0;
        double approxRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman2(QObject *parent = nullptr);
    ~TravelingSalesman2() override;

    /** @brief Set distance matrix (symmetric TSP) */
    void setDistanceMatrix(const QVector<QVector<double>>& dist);

    /** @brief Set city coordinates for Euclidean TSP */
    void setCities(const QVector<QVector<double>>& coords);

    /** @brief Solve TSP using Christofides algorithm */
    QVector<int> solve();

    /** @brief Compute cost of a given tour */
    double tourCost(const QVector<int>& tour) const;

    /** @brief Improve tour with 2-opt local search */
    QVector<int> twoOpt(const QVector<int>& tour) const;

    /** @brief Get lower bound (MST cost) */
    double lowerBound() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int cities, double cost, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<double>> m_dist;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute MST using Prim's algorithm */
    QVector<QVector<int>> minimumSpanningTree() const;

    /** @brief Find odd-degree vertices in MST */
    QVector<int> oddDegreeVertices(const QVector<QVector<int>>& mst) const;

    /** @brief Minimum-weight perfect matching (greedy) */
    QVector<QPair<int,int>> minWeightMatching(const QVector<int>& oddVertices) const;

    /** @brief Find Eulerian tour in multigraph */
    QVector<int> eulerianTour(const QVector<QVector<int>>& multigraph) const;

    /** @brief Shortcut Eulerian tour to Hamiltonian cycle */
    QVector<int> shortcutTour(const QVector<int>& euler) const;
};
