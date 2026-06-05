#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ShortestPath4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit ShortestPath4(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v, double weight, bool directed = false);
    QVector<double> dijkstra(int source);
    QVector<double> bellmanFord(int source);
    QVector<QVector<double>> floydWarshall();
    bool hasNegativeCycle() const { return m_negCycle; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int source, int reachable);
private:
    int m_n = 0; bool m_negCycle = false;
    QVector<QVector<QPair<int,double>>> m_adj;
    Stats m_stats; double m_timeSum = 0.0;
};
