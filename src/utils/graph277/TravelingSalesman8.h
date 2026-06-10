/**
 * @file TravelingSalesman8.h
 * @brief 旅行商问题(Christofides 1.5近似最小生成树最小权重完美匹配) — TSP with Christofides 1.5-approximation using Minimum Spanning Tree and Minimum Weight Perfect Matching
 *
 * 功能: 实现旅行商问题(TSP)求解器，采用Christofides 1.5近似算法(Christofides
 *       1.5-approximation)通过最小生成树(MST)和最小权重完美匹配(minimum weight
 *       perfect matching)构建近似最优哈密尔顿回路(Hamiltonian circuit)。
 *
 * 协作: Dijkstra8(最短路径) / KruskalMST7(最小生成树) / HungarianAlgorithm6(匈牙利算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 旅行商问题(Christofides 1.5近似最小生成树最小权重完美匹配)
 */
class TravelingSalesman8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        double tourCost = 0.0;
        double mstCost = 0.0;
        int matchingEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman8(QObject *parent = nullptr);
    ~TravelingSalesman8() override;

    /** @brief Set distance matrix (symmetric TSP) */
    void setDistanceMatrix(const QVector<QVector<double>>& distances);

    /** @brief Set node coordinates (computes Euclidean distances) */
    void setCoordinates(const QVector<QPair<double, double>>& coords);

    /** @brief Solve TSP using Christofides 1.5-approximation */
    QVector<int> solve();

    /** @brief Get tour cost */
    double tourCost() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourComputed(int numNodes, double cost, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<double>> m_dist;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute minimum spanning tree using Prim's algorithm */
    QVector<QPair<int, int>> computeMST() const;

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices(const QVector<QPair<int, int>>& mst) const;

    /** @brief Greedy minimum weight perfect matching on subgraph */
    QVector<QPair<int, int>> minWeightMatching(const QVector<int>& nodes) const;

    /** @brief Combine MST and matching into Eulerian multigraph */
    QVector<QVector<int>> buildMultigraph(const QVector<QPair<int, int>>& mst,
                                          const QVector<QPair<int, int>>& matching) const;

    /** @brief Find Euler tour using Hierholzer's algorithm */
    QVector<int> eulerTour(const QVector<QVector<int>>& multigraph) const;

    /** @brief Shortcut Euler tour to Hamiltonian circuit */
    QVector<int> shortcutToHamiltonian(const QVector<int>& euler) const;
};
