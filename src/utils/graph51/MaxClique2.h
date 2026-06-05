#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class MaxClique2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSearches = 0; int totalVerticesProcessed = 0; double avgProcessingTimeMs = 0.0; int bestSize = 0; };
    explicit MaxClique2(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> findMaximum();
    QList<QVector<int>> enumerateMaximal();
    int chromaticLowerBound() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void searchComplete(int size);
private:
    QVector<int> greedyColoring(const QVector<int>& order) const;
    QVector<int> degeneracyOrdering() const;
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    Stats m_stats; double m_timeSum = 0.0;
};
