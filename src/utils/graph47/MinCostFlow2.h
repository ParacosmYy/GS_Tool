/**
 * @file MinCostFlow2.h
 * @brief Min cost flow enhanced - successive shortest path/cycle canceling
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class MinCostFlow2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFlows = 0; int totalVerticesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    struct FlowResult { double totalCost = 0.0; double totalFlow = 0.0; int iterations = 0; };
    explicit MinCostFlow2(QObject* parent = nullptr);
    void setGraph(int n, const QVector<int>& from, const QVector<int>& to,
                  const QVector<double>& capacity, const QVector<double>& cost);
    FlowResult minCostFlow(int source, int sink, double demand);
    FlowResult minCostMaxFlow(int source, int sink);
    QVector<QPair<int,int>> flowEdges() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void flowComplete(double cost, double flow);
private:
    bool spfa(int source, int sink, QVector<double>& dist, QVector<int>& prev);
    int m_n = 0;
    QVector<int> m_head, m_to, m_next; QVector<double> m_cap, m_cost, m_flow;
    Stats m_stats; double m_timeSum = 0.0;
};
