/**
 * @file TravelingSalesman9.h
 * @brief 旅行商问题求解器(Lin-Kernighan-Helsgaun可变深度搜索与候选集的近最优路径改进) — TSP with Lin-Kernighan-Helsgaun Variable-depth Search and Candidate Set for Near-optimal Tour Improvement
 *
 * 功能: 实现旅行商问题求解器(TSP solver)，采用Lin-Kernighan-Helsgaun可变深度搜索(LKH variable-depth search)
 *       与候选集(candidate set)实现近最优路径改进(near-optimal tour improvement)。
 *
 * 协作: GraphBFS6(图BFS) / Dijkstra9(最短路径) / Christofides7(近似TSP)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 旅行商问题求解器(LKH可变深度搜索与候选集)
 */
class TravelingSalesman9 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver configuration */
    struct Config {
        int maxCandidates = 5;      // Max nearest neighbors in candidate set
        int maxTrials = 100;        // Max LKH trials
        double precision = 1e-6;    // Convergence precision
        bool use2Opt = true;        // Apply 2-opt after LKH
    };

    /** @brief Solver result */
    struct TSPResult {
        QVector<int> tour;           // Optimal tour (city indices)
        double tourLength = 0.0;
        int iterations = 0;
        int improvements = 0;
        bool optimal = false;
        double processingTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numCities = 0;
        double bestTourLength = 1e18;
        int totalImprovements = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman9(QObject *parent = nullptr);
    ~TravelingSalesman9() override;

    /** @brief Set solver configuration */
    void setConfig(const Config& config);

    /** @brief Set distance matrix (n x n) */
    void setDistanceMatrix(const QVector<QVector<double>>& dist);

    /** @brief Set city coordinates for Euclidean TSP */
    void setCities(const QVector<QVector<double>>& coords);

    /** @brief Solve TSP starting from nearest-neighbor initial tour */
    TSPResult solve();

    /** @brief Build candidate set (alpha-nearness) */
    QVector<QVector<int>> buildCandidateSet() const;

    /** @brief Compute tour length */
    double tourLength(const QVector<int>& tour) const;

    /** @brief Apply 2-opt local search */
    QVector<int> twoOpt(const QVector<int>& tour);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourImproved(int iteration, double length, double timeMs);
    void solveComplete(int cities, double length, int improvements, double timeMs);

private:
    Config m_config;
    int m_n = 0;                     // Number of cities
    QVector<QVector<double>> m_dist; // Distance matrix
    QVector<QVector<int>> m_candidates; // Candidate set

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build nearest-neighbor initial tour */
    QVector<int> nearestNeighborTour(int startCity) const;

    /** @brief Lin-Kernighan-Helsgaun variable-depth move */
    QVector<int> lkhImprove(const QVector<int>& tour);

    /** @brief Evaluate a k-opt move */
    double evalMove(const QVector<int>& tour, const QVector<QPair<int, int>>& breaks,
                     const QVector<QPair<int, int>>& joins) const;

    /** @brief Reconstruct tour after k-opt */
    QVector<int> reconstructTour(const QVector<int>& tour,
                                   const QVector<QPair<int, int>>& joins) const;

    /** @brief Compute Euclidean distance */
    double eucDist(const QVector<double>& a, const QVector<double>& b) const;
};
