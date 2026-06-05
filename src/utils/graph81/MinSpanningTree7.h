#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class MinSpanningTree7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalEdges = 0; double avgProcessingTimeMs = 0.0; };
    explicit MinSpanningTree7(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v, double weight);
    QVector<QPair<int,int>> solvePrim();
    QVector<QPair<int,int>> solveKruskal();
    double totalWeight() const { return m_weight; }
    bool isConnected() const { return m_connected; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int edges, double weight);
private:
    int m_n = 0; double m_weight = 0.0; bool m_connected = false;
    QVector<QPair<double,QPair<int,int>>> m_edges;
    QVector<QVector<QPair<int,double>>> m_adj;
    Stats m_stats; double m_timeSum = 0.0;
};
