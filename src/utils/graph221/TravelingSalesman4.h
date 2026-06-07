/**
 * @file TravelingSalesman4.h
 * @brief 旅行商问题(Lin-Kernighan-Helsgaun启发式+最小生成树候选集) — TSP with Lin-Kernighan-Helsgaun Heuristic and Candidate Set from Minimum Spanning Tree
 *
 * 功能: 实现TSP求解器，支持LKH启发式搜索、
 *       最小生成树候选集生成和k-opt邻域优化。
 *
 * 协作: Dijkstra8(最短路径) / MinimumSpanningTree6(最小生成树) / Christofides3(近似算法)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 旅行商问题(Lin-Kernighan-Helsgaun启发式+最小生成树候选集)
 */
class TravelingSalesman4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numCities = 0;
        int iterations = 0;
        double bestTourLength = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman4(QObject *parent = nullptr);
    ~TravelingSalesman4() override;

    /** @brief Set city coordinates */
    void setCities(const QVector<QPair<double, double>>& cities);

    /** @brief Set candidate set size per city */
    void setCandidateSize(int k);

    /** @brief Set max LKH iterations */
    void setMaxIterations(int iter);

    /** @brief Build candidate set from MST (alpha-nearness) */
    void buildCandidateSet();

    /** @brief Solve TSP using LKH heuristic */
    QVector<int> solve();

    /** @brief Compute tour length */
    double tourLength(const QVector<int>& tour) const;

    /** @brief 2-opt local improvement */
    QVector<int> twoOpt(const QVector<int>& tour) const;

    /** @brief Get candidate neighbors for a city */
    QVector<int> getCandidates(int city) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int cities, double length, int iterations, double timeMs);

private:
    QVector<QPair<double, double>> m_cities;
    int m_candidateSize = 5;
    int m_maxIter = 100;

    /** @brief Candidate neighbor list for each city */
    QVector<QVector<int>> m_candidates;

    /** @brief Distance matrix (lower triangle stored flat) */
    QVector<double> m_distMatrix;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build distance matrix */
    void buildDistMatrix();

    /** @brief Get distance between two cities */
    double dist(int i, int j) const;

    /** @brief Build MST using Prim's algorithm */
    QVector<QPair<int, int>> buildMST() const;

    /** @brief Compute alpha-nearness values from MST */
    QVector<QVector<QPair<int, double>>> computeAlphaNearness() const;

    /** @brief Nearest neighbor heuristic for initial tour */
    QVector<int> nearestNeighborTour() const;
};
