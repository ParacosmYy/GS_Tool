/**
 * @file ChinesePostman.h
 * @brief 中国邮路问题 — Euler回路/最短邮路
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class ChinesePostman : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalVertices = 0;
        int totalEdgeDuplications = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman(QObject* parent = nullptr);

    void setGraph(int n, bool directed,
                  const QVector<QPair<QPair<int,int>,double>>& edges);
    double solve();
    QVector<int> eulerTour() const;
    QVector<QPair<int,int>> duplicatedEdges() const;

    bool isEulerian() const;
    int vertices() const { return m_n; }
    double totalCost() const { return m_totalCost; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(double cost, int duplications);

private:
    int m_n = 0;
    bool m_directed = false;
    double m_totalCost = 0.0;
    QVector<QVector<QPair<int,double>>> m_adj;
    QVector<int> m_tour;
    QVector<QPair<int,int>> m_duplicated;

    void eulerianizeUndirected();
    void eulerianizeDirected();
    QVector<int> findEulerTour();
    QVector<double> minCostMatching(const QVector<int>& oddVertices);

    Stats m_stats;
    double m_timeSum = 0.0;
};
