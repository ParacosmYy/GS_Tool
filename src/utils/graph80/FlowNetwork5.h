#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class FlowNetwork5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit FlowNetwork5(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v, double cap);
    void setSource(int s);
    void setSink(int t);
    double maxFlow();
    double minCostMaxFlow();
    QVector<QPair<int,int>> minCut();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void flowComputed(double flow, int augmentations);
private:
    int m_n = 0; int m_source = 0; int m_sink = 0;
    struct Edge { int to, rev; double cap, cost; };
    QVector<QVector<Edge>> m_adj;
    double bfsLevelGraph(QVector<int>& level);
    double dfsBlocking(int u, double pushed, QVector<int>& ptr, const QVector<int>& level);
    Stats m_stats; double m_timeSum = 0.0;
};
