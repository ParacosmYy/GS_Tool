/**
 * @file TravelingSalesman7.h
 * @brief 旅行商问题(Lin-Kernighan启发式+螺旋增益准则迭代局部搜索) — TSP with Lin-Kernighan Heuristic and Helical Gain Criterion for Iterated Local Search Improvement
 *
 * 功能: 实现旅行商问题(TSP)求解器，使用Lin-Kernighan启发式(LK heuristic)
 *       进行k-opt邻域搜索，结合螺旋增益准则(helical gain criterion)评估
 *       边交换收益，通过迭代局部搜索(iterated local search)持续改进解。
 *
 * 协作: Dijkstra9(最短路径) / AStar9(启发式搜索) / MinimumSpanningTree7(最小生成树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 旅行商问题(Lin-Kernighan+螺旋增益准则)
 */
class TravelingSalesman7 : public QObject {
    Q_OBJECT

public:
    /** @brief Tour result */
    struct TourResult {
        QVector<int> tour;
        double totalDistance = 0.0;
        int numImprovements = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numCities = 0;
        int numIterations = 0;
        int numImprovements = 0;
        double bestDistance = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman7(QObject *parent = nullptr);
    ~TravelingSalesman7() override;

    /** @brief Set distance matrix (NxN symmetric) */
    void setDistanceMatrix(const QVector<QVector<double>>& matrix);

    /** @brief Set coordinates for Euclidean distance computation */
    void setCoordinates(const QVector<QVector<double>>& coords);

    /** @brief Set maximum LK search iterations */
    void setMaxIterations(int maxIter);

    /** @brief Solve TSP starting from nearest-neighbor initial tour */
    TourResult solve();

    /** @brief Improve an existing tour with LK heuristic */
    TourResult improve(const QVector<int>& initialTour);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourImproved(int iteration, double distance, double timeMs);

private:
    QVector<QVector<double>> m_distMatrix;
    int m_numCities = 0;
    int m_maxIter = 100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute total tour distance */
    double tourDistance(const QVector<int>& tour) const;

    /** @brief Build initial tour via nearest-neighbor heuristic */
    QVector<int> nearestNeighborTour() const;

    /** @brief 2-opt improvement step */
    bool twoOptStep(QVector<int>& tour);

    /** @brief Lin-Kernighan helical gain evaluation */
    double helicalGain(const QVector<int>& tour, int i, int j, int k) const;

    /** @brief Perform LK k-opt move on tour */
    bool lkMove(QVector<int>& tour, int depth);

    /** @brief Reverse a segment of the tour between indices */
    void reverseSegment(QVector<int>& tour, int from, int to);

    /** @brief Compute Euclidean distance between two points */
    double euclideanDist(const QVector<double>& a,
                         const QVector<double>& b) const;
};
