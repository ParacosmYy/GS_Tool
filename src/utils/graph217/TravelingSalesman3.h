/**
 * @file TravelingSalesman3.h
 * @brief 旅行商问题(Lin-Kernighan启发式+k-opt邻域搜索+候选集生成) — Traveling Salesman Problem with Lin-Kernighan Heuristic, k-opt Moves and Candidate Set Generation
 *
 * 功能: 实现LKH启发式TSP求解器，支持k-opt邻域搜索、
 *       候选集生成和自适应k值调整。
 *
 * 协作: GraphBFS7(图搜索) / MinimumSpan7(最小生成树) / AStar7(A*搜索)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 旅行商问题(Lin-Kernighan启发式+k-opt邻域搜索+候选集生成)
 */
class TravelingSalesman3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int numCities = 0;
        double bestTourLength = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman3(QObject *parent = nullptr);
    ~TravelingSalesman3() override;

    void setMaxKOpt(int k);
    void setCandidateSetSize(int size);
    void setMaxIterations(int iter);

    /** @brief Solve TSP on n x 2 coordinate matrix, returns optimal tour order */
    QVector<int> solve(const QVector<QVector<double>>& cities);

    /** @brief Build distance matrix from coordinates */
    QVector<QVector<double>> buildDistanceMatrix(const QVector<QVector<double>>& cities) const;

    /** @brief Generate candidate set (nearest neighbors) for each city */
    QVector<QVector<int>> generateCandidates(const QVector<QVector<double>>& distMatrix) const;

    /** @brief Compute total tour length */
    double tourLength(const QVector<int>& tour, const QVector<QVector<double>>& distMatrix) const;

    /** @brief Lin-Kernighan k-opt improvement step */
    QVector<int> linKernighan(const QVector<int>& tour,
                               const QVector<QVector<double>>& distMatrix,
                               const QVector<QVector<int>>& candidates);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int cities, double tourLength, double timeMs);

private:
    int m_maxK = 5;
    int m_candidateSize = 5;
    int m_maxIter = 100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two 2D points */
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    /** @brief Perform a single k-opt move */
    QVector<int> kOptMove(const QVector<int>& tour, int k,
                           const QVector<QVector<double>>& distMatrix,
                           const QVector<QVector<int>>& candidates) const;

    /** @brief 2-opt reversal between indices i and j */
    QVector<int> twoOptSwap(const QVector<int>& tour, int i, int j) const;

    /** @brief Compute gain from breaking and rejoining edges */
    double moveGain(const QVector<QVector<double>>& distMatrix,
                     int a, int b, int c, int d) const;
};
