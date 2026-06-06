/**
 * @file TravelingSalesman.h
 * @brief 旅行商问题求解(最近邻启发+2-opt局部搜索+MST下界) — TSP Solver with Nearest-Neighbor Heuristic, 2-opt Local Search and MST Lower Bound
 *
 * 功能: 实现旅行商问题(TSP)近似求解，支持最近邻贪心构造、
 *       2-opt局部搜索优化和最小生成树(MST)下界估计。
 *
 * 协作: PrimMST4(Prim MST) / DijkstraShortest5(Dijkstra) / Christofides4(Christofides)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 旅行商问题求解器(最近邻+2-opt+MST下界)
 */
class TravelingSalesman : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int numCities = 0;
        double tourCost = 0.0;
        double mstLowerBound = 0.0;
        double optimalityGap = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman(QObject *parent = nullptr);
    ~TravelingSalesman() override;

    void setStartCity(int city);
    void setMax2OptIterations(int iter);
    void setNumRestarts(int restarts);

    /** @brief 求解TSP，返回城市访问顺序 */
    QVector<int> solve(const QVector<QVector<double>>& distMatrix);

    /** @brief 最近邻启发式构造初始路径 */
    QVector<int> nearestNeighbor(const QVector<QVector<double>>& dist,
                                  int start) const;

    /** @brief 2-opt局部搜索优化 */
    QVector<int> twoOptImprove(const QVector<QVector<double>>& dist,
                                QVector<int> tour) const;

    /** @brief 计算路径总代价 */
    double tourCost(const QVector<QVector<double>>& dist,
                    const QVector<int>& tour) const;

    /** @brief Prim MST下界估计 */
    double mstLowerBound(const QVector<QVector<double>>& dist) const;

    /** @brief 多起点求解(选最优) */
    QVector<int> multiStartSolve(const QVector<QVector<double>>& dist);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int cities, double cost, double lowerBound);

private:
    int m_startCity = 0;
    int m_max2OptIter = 1000;
    int m_numRestarts = 5;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Reverse a segment of the tour [i, j] */
    void reverseSegment(QVector<int>& tour, int i, int j) const;

    /** @brief Compute 2-opt delta for swapping edges */
    double twoOptDelta(const QVector<QVector<double>>& dist,
                       const QVector<int>& tour, int i, int j) const;
};
