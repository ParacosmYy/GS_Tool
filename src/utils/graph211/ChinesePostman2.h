/**
 * @file ChinesePostman2.h
 * @brief 中国邮路问题(有向图弧平衡+最小费用流+欧拉回路) — Chinese Postman for Directed Graphs with Arc Balancing via Min-Cost Flow and Euler Tour
 *
 * 功能: 实现有向图中国邮路问题求解，支持弧平衡、
 *       最小费用流匹配和欧拉回路构建。
 *
 * 协作: TSP10(旅行商) / Dijkstra8(最短路径) / EulerPath3(欧拉路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 中国邮路问题(有向图+最小费用流)
 */
class ChinesePostman2 : public QObject {
    Q_OBJECT

public:
    /** @brief Directed edge */
    struct Arc {
        int from = -1;
        int to = -1;
        double cost = 0.0;
        int multiplicity = 1;  // extra traversals needed
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int numNodes = 0;
        int numArcs = 0;
        double totalCost = 0.0;
        int extraArcs = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman2(QObject *parent = nullptr);
    ~ChinesePostman2() override;

    void setMaxIterations(int iter);

    /** @brief Solve Chinese Postman on directed graph */
    QVector<int> solve(int numNodes, const QVector<Arc>& arcs);

    /** @brief Compute arc balance: out-degree - in-degree for each node */
    QVector<int> arcBalance(int numNodes, const QVector<Arc>& arcs) const;

    /** @brief Find shortest paths between all pairs (Floyd-Warshall) */
    QVector<QVector<double>> allPairsShortest(int numNodes, const QVector<Arc>& arcs) const;

    /** @brief Compute min-cost flow for imbalance matching */
    QVector<Arc> minCostFlow(const QVector<int>& balance,
                              const QVector<QVector<double>>& dist) const;

    /** @brief Extract Euler tour from multigraph */
    QVector<int> eulerTour(int numNodes, const QVector<Arc>& arcs,
                           const QVector<Arc>& extraArcs) const;

    /** @brief Get tour cost */
    double tourCost() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int nodes, int extraArcs, double cost, double timeMs);

private:
    int m_maxIter = 10000;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_tourCost = 0.0;

    /** @brief Build adjacency list from arcs */
    QVector<QVector<QPair<int, double>>> buildAdjList(int n, const QVector<Arc>& arcs) const;
};
