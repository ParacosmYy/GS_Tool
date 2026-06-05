/**
 * @file IndependentSet.h
 * @brief 独立集求解 — 最大独立集/贪心近似/分支定界/补图转换
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class IndependentSet : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSearches = 0;
        int totalVerticesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
        int bestSize = 0;
    };
    explicit IndependentSet(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> greedyMaximum();
    QVector<int> branchAndBound();
    QVector<int> complementClique();
    bool isIndependent(const QVector<int>& vertices) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void searchComplete(int setSize);
private:
    int bound(QVector<bool>& available) const;
    void bbSearch(QVector<bool>& available, QVector<int>& current,
                  QVector<int>& best, int depth);
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    Stats m_stats;
    double m_timeSum = 0.0;
};
