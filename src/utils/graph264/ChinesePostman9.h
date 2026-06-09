/**
 * @file ChinesePostman9.h
 * @brief 中国邮路问题(Gabow匹配算法+增广欧拉回路邮路构造) — Chinese Postman with Gabow's Matching Algorithm and Postman Tour Construction via Augmented Euler Circuit
 *
 * 功能: 实现中国邮路问题求解(Chinese Postman Problem)，使用Gabow匹配算法
 *       (Gabow's matching)为奇度顶点找到最小权完美匹配，通过增广边构造
 *       欧拉回路(Euler circuit)生成最优邮路遍历。
 *
 * 协作: Dijkstra8(最短路径) / EulerCircuit7(欧拉回路) / GraphColoring6(图着色)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 中国邮路问题(Gabow匹配+增广欧拉回路)
 */
class ChinesePostman9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        int numMatchedPairs = 0;
        double totalTourWeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman9(QObject *parent = nullptr);
    ~ChinesePostman9() override;

    /** @brief Set graph: adjacency list with weights. edges[i] = [{neighbor, weight}] */
    void setGraph(const QVector<QVector<QPair<int, double>>>& adjacency, int numVertices);

    /** @brief Compute optimal postman tour, return vertex sequence */
    QVector<int> computeTour();

    /** @brief Get total tour weight */
    double tourWeight() const;

    /** @brief Get matched odd-degree pairs */
    QVector<QPair<int, int>> matchedPairs() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourComputed(int numVertices, double weight, double timeMs);

private:
    int m_numVertices = 0;
    QVector<QVector<QPair<int, double>>> m_adj;  // Adjacency list
    QVector<int> m_tour;
    double m_tourWeight = 0.0;
    QVector<QPair<int, int>> m_matchedPairs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find odd-degree vertices */
    QVector<int> findOddDegreeVertices() const;

    /** @brief All-pairs shortest paths via Dijkstra */
    QVector<QVector<double>> allPairsShortest() const;

    /** @brief Gabow's matching for minimum weight perfect matching */
    QVector<QPair<int, int>> gabowMatching(
        const QVector<int>& oddVertices,
        const QVector<QVector<double>>& dist) const;

    /** @brief Dijkstra from source, return distances */
    QVector<double> dijkstra(int source) const;

    /** @brief Construct Euler circuit via Hierholzer's algorithm */
    QVector<int> eulerCircuit(
        QVector<QVector<QPair<int, double>>>& augAdj) const;

    /** @brief Augment graph with duplicate edges along shortest paths */
    void augmentGraph(QVector<QVector<QPair<int, double>>>& augAdj,
                       const QVector<QPair<int, int>>& pairs,
                       const QVector<QVector<double>>& dist,
                       const QVector<QVector<int>>& next) const;

    /** @brief Compute next-hop matrix for path reconstruction */
    QVector<QVector<int>> computeNextHop(
        const QVector<QVector<double>>& dist) const;
};
