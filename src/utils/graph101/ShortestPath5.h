#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 最短路径求解器
 *
 * 支持Dijkstra/Bellman-Ford/Floyd-Warshall算法。
 */
class ShortestPath5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPathsComputed = 0;
        int totalRelaxations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ShortestPath5(QObject* parent = nullptr);

    /** @brief Dijkstra单源最短路径(非负权) */
    QVector<double> dijkstra(const QVector<QVector<QPair<int, double>>>& adjList, int source);

    /** @brief Bellman-Ford单源最短路径(允许负权) */
    QVector<double> bellmanFord(const QVector<QVector<QPair<int, double>>>& adjList,
                                 int source, int nodeCount);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pathComputed(int source, int destinations, bool hasNegativeCycle);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
