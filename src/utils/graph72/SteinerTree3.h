#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SteinerTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalTerminals = 0; double avgProcessingTimeMs = 0.0; };
    explicit SteinerTree3(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v, double weight);
    void setTerminals(const QVector<int>& terminals);
    QVector<QPair<int,int>> solve();
    double totalWeight() const { return m_totalWeight; }
    int steinerNodes() const { return m_steinerCount; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int edges, double weight);
private:
    int m_n = 0; double m_totalWeight = 0.0; int m_steinerCount = 0;
    QVector<int> m_terminals;
    QVector<QVector<QPair<int,double>>> m_adj;
    QVector<QVector<double>> allPairsShortest();
    QVector<QPair<int,int>> steinerApprox();
    Stats m_stats; double m_timeSum = 0.0;
};
