/**
 * @file Matching3.h
 * @brief 匹配增强 — Hopcroft-Karp/加权匈牙利/二分图/最小权匹配
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class Matching3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalMatches = 0; int totalAugmentations = 0; double avgProcessingTimeMs = 0.0; };
    explicit Matching3(QObject* parent = nullptr);
    void setBipartite(int leftSize, int rightSize, const QVector<QPair<int,int>>& edges);
    void setWeighted(const QVector<QPair<QPair<int,int>,double>>& edges, int leftSize, int rightSize);
    QVector<QPair<int,int>> maximumMatching();
    QVector<QPair<int,int>> minimumWeightMatching();
    int matchingSize() const;
    double matchingWeight() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void matchingComplete(int size, double weight);
private:
    bool bfs(QVector<int>& dist);
    bool dfs(int u, QVector<int>& dist, QVector<int>& matchL, QVector<int>& matchR);
    int m_leftSize = 0, m_rightSize = 0;
    QVector<QVector<int>> m_adj;
    QVector<QVector<double>> m_weights;
    QVector<QPair<int,int>> m_matching;
    double m_totalWeight = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
