/**
 * @file BipartiteMatch3.h
 * @brief 二分图匹配3 — 加权KM+DFS/BFS混合
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class BipartiteMatch3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalMatches = 0;
        int totalAugmentations = 0;
        int leftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BipartiteMatch3(QObject* parent = nullptr);

    void setUnweighted(const QVector<QPair<int,int>>& edges, int leftN, int rightN);
    void setWeighted(const QVector<QPair<QPair<int,int>,double>>& edges,
                     int leftN, int rightN);
    QVector<QPair<int,int>> maximumMatching();
    QVector<QPair<int,int>> maximumWeightedMatching();
    QVector<QPair<int,int>> minimumWeightedMatching();

    int matchingSize() const { return m_matching.size(); }
    double matchingWeight() const { return m_totalWeight; }
    bool isPerfect() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void matchingFound(int size, double weight);

private:
    int m_leftN = 0;
    int m_rightN = 0;
    QVector<QPair<int,int>> m_matching;
    double m_totalWeight = 0.0;
    QVector<QVector<double>> m_weightMatrix;

    QVector<QPair<int,int>> hopcroftKarp();
    QVector<QPair<int,int>> kuhnMunkres();

    Stats m_stats;
    double m_timeSum = 0.0;
};
