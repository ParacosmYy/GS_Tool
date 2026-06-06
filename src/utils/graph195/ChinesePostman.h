/**
 * @file ChinesePostman.h
 * @brief 中国邮路(奇度配对最小权匹配+欧拉回路构造) — Chinese Postman (Route Inspection) with Odd-degree Pairing via Min-weight Matching and Euler Circuit Construction
 *
 * 功能: 实现中国邮路问题求解，支持奇度顶点配对(Floyd最短路+最小权完美匹配)、
 *       欧拉回路构造和最短遍历路线计算。
 *
 * 协作: Dijkstra8(Dijkstra) / FloydWarshall10(Floyd) / EulerPath9(欧拉路径)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 中国邮路问题求解器(奇度配对+欧拉回路)
 */
class ChinesePostman : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numVertices = 0;
        int numEdges = 0;
        int numOddVertices = 0;
        double totalCost = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Edge definition */
    struct Edge {
        int from = 0;
        int to = 0;
        double weight = 1.0;
    };

    explicit ChinesePostman(QObject *parent = nullptr);
    ~ChinesePostman() override;

    /** @brief 设置图(邻接表+边列表) */
    void setGraph(int numVertices, const QVector<Edge>& edges);

    /** @brief 求解中国邮路，返回遍历顶点序列 */
    QVector<int> solve();

    /** @brief 获取总路径代价 */
    double totalCost() const;

    /** @brief 查找奇度顶点 */
    QVector<int> findOddDegreeVertices() const;

    /** @brief Floyd-Warshall全源最短路 */
    QVector<QVector<double>> allPairsShortestPath() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(int numEdges, double totalCost);

private:
    int m_numVertices = 0;
    QVector<Edge> m_edges;
    QVector<QVector<QPair<int, double>>> m_adj; ///< Adjacency: (neighbor, weight)
    double m_totalCost = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Minimum weight perfect matching via brute-force for small odd sets */
    double minWeightMatching(const QVector<int>& oddVertices,
                              const QVector<QVector<double>>& dist,
                              QVector<QPair<int, int>>& pairs) const;

    /** @brief Recursive matching search */
    void matchSearch(const QVector<int>& odds, int idx, double currentCost,
                     QVector<bool>& used, double& bestCost,
                     QVector<QPair<int, int>>& currentPairs,
                     QVector<QPair<int, int>>& bestPairs,
                     const QVector<QVector<double>>& dist) const;

    /** @brief Build augmented multigraph and find Euler circuit */
    QVector<int> eulerCircuit(const QVector<QPair<int, int>>& extraEdges) const;

    /** @brief Hierholzer's algorithm for Euler circuit */
    void hierholzer(int start, QVector<QVector<QPair<int, double>>>& augAdj,
                     QVector<int>& circuit) const;
};
