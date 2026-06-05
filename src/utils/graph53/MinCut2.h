/**
 * @file MinCut2.h
 * @brief 最小割2 — Stoer-Wagner全局最小割
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class MinCut2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalCuts = 0;
        int totalVertices = 0;
        int totalPhases = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MinCut2(QObject* parent = nullptr);

    void setGraph(int n, const QVector<QPair<QPair<int,int>,double>>& edges);
    double findMinCut();
    QVector<QPair<int,int>> cutEdges() const;
    QPair<QVector<int>,QVector<int>> partition() const;

    int vertices() const { return m_n; }
    double cutValue() const { return m_cutValue; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cutFound(double value, int partitionSize);

private:
    int m_n = 0;
    double m_cutValue = 0.0;
    QVector<QVector<double>> m_adj;
    QVector<int> m_partitionA;
    QVector<int> m_partitionB;
    QVector<QPair<int,int>> m_cutEdges;

    double stoerWagnerPhase(QVector<double>& weights, QVector<bool>& merged,
                            QVector<int>& mergeOrder);

    Stats m_stats;
    double m_timeSum = 0.0;
};
