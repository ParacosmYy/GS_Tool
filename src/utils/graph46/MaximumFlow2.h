/**
 * @file MaximumFlow2.h
 * @brief Maximum flow enhanced - Dinic/capacity scaling/min-cut
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class MaximumFlow2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFlows = 0; int totalVerticesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit MaximumFlow2(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<QPair<int,int>,double>>& edges, int source, int sink);
    double maxFlow();
    double dinicFlow();
    double scalingFlow();
    QVector<QPair<int,int>> minCut() const;
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void flowComplete(double flow);
private:
    bool bfsLevel();
    double dfsSend(int u, double flow);
    int m_n = 0, m_source = 0, m_sink = 0;
    QVector<QVector<QPair<int,double>>> m_adj;
    QVector<int> m_level, m_iter;
    double m_maxFlowVal = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
