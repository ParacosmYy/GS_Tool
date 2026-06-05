#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class MinSpanningTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalEdges = 0; double avgProcessingTimeMs = 0.0; };
    explicit MinSpanningTree6(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v, double weight);
    QVector<QPair<int,int>> solve();
    double totalWeight() const { return m_totalWeight; }
    bool isConnected() const { return m_connected; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int edges, double weight);
private:
    int m_n = 0; double m_totalWeight = 0.0; bool m_connected = false;
    QVector<QPair<double,QPair<int,int>>> m_edges;
    QVector<int> m_parent;
    int findSet(int v);
    bool unionSets(int a, int b);
    Stats m_stats; double m_timeSum = 0.0;
};
