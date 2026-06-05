#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
/** @brief SCC enhanced - Tarjan/Kosaraju/2-SAT/condensation */
class StronglyConnected3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalVerticesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit StronglyConnected3(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> tarjanSCC();
    QVector<int> kosarajuSCC();
    bool solve2SAT(const QVector<QPair<int,int>>& clauses, int numVars);
    QVector<QPair<int,int>> condensationGraph() const;
    int numComponents() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computationComplete(int numComponents);
private:
    int m_n = 0;
    QVector<QVector<int>> m_adj, m_radj;
    QVector<int> m_component;
    Stats m_stats; double m_timeSum = 0.0;
};
