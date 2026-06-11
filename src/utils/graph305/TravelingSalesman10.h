/**
 * @file TravelingSalesman10.h
 * @brief 旅行商问题求解器(Lin-Kernighan-Helsgaun k-opt移动与候选集引导边交换实现近最优路径构造) — Traveling Salesman with Lin-Kernighan-Helsgaun k-opt Moves and Candidate Set Guided Edge Exchange for Near-optimal Tour Construction
 *
 * 功能: 实现旅行商问题求解器(TSP solver)，采用Lin-Kernighan-Helsgaun k-opt移动(LKH k-opt moves)
 *       与候选集引导边交换(candidate set guided edge exchange)实现近最优路径构造(near-optimal tour construction)。
 *
 * 协作: Dijkstra12(最短路径) / MinimumSpanningTree10(最小生成树) / Christofides10(近似算法)
 */
#pragma once

#include <QObject>
#include <QVector>

class TravelingSalesman10 : public QObject {
    Q_OBJECT

public:
    /** @brief TSP solution result */
    struct TourResult {
        QVector<int> tour;              // Ordered city indices
        double totalDistance = 0.0;
        int numMoves = 0;               // Total k-opt moves applied
        bool optimal = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int numCities = 0;
        double avgProcessingTimeMs = 0.0;
        double bestDistance = 1e300;
    };

    explicit TravelingSalesman10(QObject *parent = nullptr);
    ~TravelingSalesman10() override;

    void setMaxIterations(int iter);
    void setCandidateSetSize(int k);
    void setKOptMax(int maxK);

    /** @brief Solve TSP with distance matrix [n×n] */
    TourResult solve(const QVector<QVector<double>>& distMatrix);

    /** @brief Solve TSP with 2D coordinates */
    TourResult solveFromCoords(const QVector<QPair<double,double>>& coords);

    /** @brief Compute tour distance from given tour */
    double computeTourDistance(const QVector<int>& tour,
                                const QVector<QVector<double>>& distMatrix) const;

    /** @brief Build nearest-neighbor candidate set */
    QVector<QVector<int>> buildCandidateSet(
        const QVector<QVector<double>>& distMatrix) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int cities, double distance, int moves, double timeMs);

private:
    int m_maxIter = 1000;
    int m_candidateK = 5;
    int m_kOptMax = 5;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build initial tour using nearest neighbor heuristic */
    QVector<int> nearestNeighborTour(const QVector<QVector<double>>& dist) const;

    /** @brief Apply Lin-Kernighan-Helsgaun improvement */
    int lkhImprove(QVector<int>& tour,
                    const QVector<QVector<double>>& dist,
                    const QVector<QVector<int>>& candidates);

    /** @brief Try a single k-opt move */
    bool tryKOptMove(QVector<int>& tour, int k,
                      const QVector<QVector<double>>& dist,
                      const QVector<QVector<int>>& candidates);

    /** @brief Reverse a segment of the tour */
    void reverseSegment(QVector<int>& tour, int from, int to);

    /** @brief Compute gain from edge exchange */
    double exchangeGain(double dOld1, double dOld2,
                         double dNew1, double dNew2) const;

    /** @brief Compute Euclidean distance between two points */
    double euclideanDist(QPair<double,double> a,
                          QPair<double,double> b) const;

    /** @brief Build distance matrix from coordinates */
    QVector<QVector<double>> buildDistMatrix(
        const QVector<QPair<double,double>>& coords) const;
};
