/**
 * @file DominatingSet2.h
 * @brief 支配集2 — 贪心近似+加权支配
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class DominatingSet2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalVertices = 0;
        int dominatingSetSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet2(QObject* parent = nullptr);

    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    void setWeights(const QVector<double>& weights);
    QVector<int> solve();
    QVector<int> solveWeighted();
    bool isDominating(const QVector<int>& set) const;
    QVector<int> connectedDominatingSet();

    int vertices() const { return m_n; }
    int solutionSize() const { return m_solution.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(int setSize, int vertices);

private:
    int m_n = 0;
    QVector<double> m_weights;
    QVector<QVector<int>> m_adj;
    QVector<int> m_solution;

    QVector<int> greedyDominating() const;
    QVector<int> weightedGreedy() const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
