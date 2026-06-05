/**
 * @file TravelingSalesman2.h
 * @brief 旅行商问题2 — 2-opt+3-opt+LK局部搜索
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class TravelingSalesman2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalImprovements = 0;
        int totalNodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TravelingSalesman2(QObject* parent = nullptr);

    void setCities(const QVector<QPair<double,double>>& coordinates);
    void setDistanceMatrix(const QVector<double>& dist, int n);
    QVector<int> solve();
    double tourLength(const QVector<int>& tour) const;
    QVector<int> improve(const QVector<int>& initialTour);

    int numCities() const { return m_n; }
    double bestLength() const { return m_bestLength; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int nodes, double length, int improvements);

private:
    int m_n = 0;
    QVector<double> m_dist;
    double m_bestLength = 0.0;
    QVector<int> m_bestTour;

    QVector<int> nearestNeighborStart() const;
    double twoOpt(QVector<int>& tour);
    double threeOpt(QVector<int>& tour);
    double orOpt(QVector<int>& tour);
    bool twoOptSwap(QVector<int>& tour, int i, int j);

    Stats m_stats;
    double m_timeSum = 0.0;
};
