#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class FlowNetwork3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFlows = 0; double avgProcessingTimeMs = 0.0; };
    explicit FlowNetwork3(QObject* parent = nullptr);
    void setGraph(int n, const QVector<int>& from, const QVector<int>& to,
                  const QVector<double>& cap, const QVector<double>& cost);
    QPair<double,double> minCostMaxFlow(int source, int sink);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void flowComplete(double cost, double flow);
private:
    int m_n = 0;
    QVector<int> m_rowPtr, m_colIdx;
    QVector<double> m_values;
    Stats m_stats; double m_timeSum = 0.0;
};
