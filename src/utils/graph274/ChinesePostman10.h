/**
 * @file ChinesePostman10.h
 * @brief 中国邮路问题(Blossom V匹配增广路径最小奇集配对) — Chinese Postman with Blossom Algorithm V Matching and Augmenting Path for Minimum Odd-set Pairing
 *
 * 功能: 实现中国邮路问题(Chinese postman)的精确算法，采用Blossom
 *      算法V进行最大权重匹配(maximum weight matching)，结合增广
 *       路径(augmenting path)实现最小奇集配对(minimum odd-set
 *       pairing)，求解最优欧拉回路。
 *
 * 协作: Dijkstra7(最短路径) / FloydWarshall8(全源最短路) / EulerPath10(欧拉回路)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Blossom V匹配增广路径最小奇集配对)
 */
class ChinesePostman10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge representation */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 0.0;
        bool isDuplicate = false;  // Added edge for Eulerian
    };

    explicit ChinesePostman10(QObject *parent = nullptr);
    ~ChinesePostman10() override;

    /** @brief Add an undirected edge */
    void addEdge(int u, int v, double weight);

    /** @brief Solve the Chinese postman problem */
    double solve();

    /** @brief Get the Eulerian tour (vertex sequence) */
    QVector<int> eulerTour() const;

    /** @brief Get all edges including duplicates */
    QVector<Edge> allEdges() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(int numOdd, double extraCost, double totalCost, double timeMs);

private:
    int m_n = 0;
    QVector<Edge> m_edges;
    QVector<Edge> m_allEdges;    // Including duplicates
    QVector<int> m_tour;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find odd-degree vertices */
    QVector<int> findOddVertices() const;

    /** @brief All-pairs shortest paths (Floyd-Warshall) */
    void floydWarshall(QVector<QVector<double>>& dist,
                       QVector<QVector<int>>& next) const;

    /** @brief Blossom V minimum weight perfect matching on odd vertices */
    QVector<QPair<int, int>> blossomMatch(
        const QVector<int>& oddVerts,
        const QVector<QVector<double>>& dist) const;

    /** @brief Find augmenting path for matching */
    bool findAugmentingPath(int start, const QVector<int>& oddVerts,
                            const QVector<QVector<double>>& dist,
                            QVector<bool>& matched,
                            QVector<QPair<int, int>>& matching) const;
};
