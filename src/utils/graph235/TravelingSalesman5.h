/**
 * @file TravelingSalesman5.h
 * @brief 旅行商问题(Christofides 1.5近似+局部搜索2-opt改进) — Traveling Salesman Problem with Christofides 1.5-Approximation and Local Search 2-Opt Improvement Heuristic
 *
 * 功能: 实现TSP求解器，先使用Christofides算法获得1.5-近似解，
 *       再通过2-opt局部搜索优化改进路径质量。
 *
 * 协作: Dijkstra10(最短路径) / MinimumSpanningTree8(最小生成树) / GraphColoring5(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 旅行商问题(Christofides+2-opt)
 */
class TravelingSalesman5 : public QObject {
    Q_OBJECT

public:
    /** @brief TSP solution result */
    struct TSPResult {
        QVector<int> tour;
        double totalDistance = 0.0;
        int twoOptSwaps = 0;
        bool optimal = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numCities = 0;
        double christofidesCost = 0.0;
        double improvedCost = 0.0;
        double improvementPercent = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman5(QObject *parent = nullptr);
    ~TravelingSalesman5() override;

    /** @brief Set distance matrix (symmetric) */
    void setDistanceMatrix(const QVector<QVector<double>>& dist);

    /** @brief Solve TSP: Christofides + 2-opt improvement */
    TSPResult solve();

    /** @brief Solve using only 2-opt from given initial tour */
    TSPResult solve2Opt(const QVector<int>& initialTour);

    /** @brief Compute tour total distance */
    double tourDistance(const QVector<int>& tour) const;

    /** @brief Get minimum spanning tree (Prim's algorithm) */
    QVector<QPair<int, int>> minimumSpanningTree() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(double cost, double improved, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<double>> m_dist;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Prim's MST algorithm */
    QVector<QPair<int, int>> primMST() const;

    /** @brief Find odd-degree vertices in MST */
    QVector<int> oddDegreeVertices(
        const QVector<QPair<int, int>>& mst) const;

    /** @brief Minimum weight perfect matching (greedy approximation) */
    QVector<QPair<int, int>> minWeightMatching(
        const QVector<int>& oddVerts) const;

    /** @brief Find Euler tour in multigraph */
    QVector<int> eulerTour(
        const QVector<QPair<int, int>>& mst,
        const QVector<QPair<int, int>>& matching) const;

    /** @brief Shortcut Euler tour to Hamiltonian cycle */
    QVector<int> shortcutToHamiltonian(const QVector<int>& euler) const;

    /** @brief 2-opt local search improvement */
    void applyTwoOpt(QVector<int>& tour, int& swapCount);

    /** @brief Calculate distance between cities i and j */
    double dist(int i, int j) const;
};
