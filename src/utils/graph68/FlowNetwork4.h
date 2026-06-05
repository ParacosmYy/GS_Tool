#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class FlowNetwork4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit FlowNetwork4(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<QPair<int,int>,double>>& edges);
    double maxFlow(int source, int sink);
    QVector<QPair<int,int>> minCut(int source, int sink) const;
    int vertices() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void flowCompleted(double flow);
private:
    int m_n = 0;
    QVector<QVector<QPair<int,double>>> m_adj;
    double dinic(int s, int t);
    bool bfs(int s, int t, QVector<int>& level);
    double dfs(int u, int t, double f, QVector<int>& iter);
    Stats m_stats; double m_timeSum = 0.0;
};
