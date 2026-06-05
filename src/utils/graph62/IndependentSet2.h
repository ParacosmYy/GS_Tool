/**
 * @file IndependentSet2.h
 * @brief 独立集2 — 最大独立集近似+图着色方法
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class IndependentSet2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalVertices = 0;
        int independentSetSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IndependentSet2(QObject* parent = nullptr);

    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> solve();
    QVector<int> solveGreedy();
    QVector<int> solveColoring();
    bool isIndependent(const QVector<int>& set) const;
    QVector<int> maximalIndependentSet() const;

    int vertices() const { return m_n; }
    int solutionSize() const { return m_solution.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(int setSize, int vertices);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    QVector<int> m_solution;

    Stats m_stats;
    double m_timeSum = 0.0;
};
