#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ChinesePostman2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalEdges = 0; double avgProcessingTimeMs = 0.0; };
    explicit ChinesePostman2(QObject* parent = nullptr);
    void setDirected(bool directed);
    void addEdge(int u, int v, double weight);
    QVector<int> solve();
    double tourCost() const { return m_cost; }
    bool isEulerian() const { return m_eulerian; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int edges, double cost);
private:
    bool m_directed = false; double m_cost = 0.0; bool m_eulerian = true;
    int m_n = 0;
    QVector<QVector<QPair<int,double>>> m_adj;
    QVector<int> findEulerTour();
    void matchOddVertices();
    Stats m_stats; double m_timeSum = 0.0;
};
