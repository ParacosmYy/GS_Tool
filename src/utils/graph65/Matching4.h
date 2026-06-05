#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class Matching4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalMatches = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit Matching4(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<QPair<int,int>,double>>& edges);
    QVector<QPair<int,int>> maximumWeighted();
    QVector<QPair<int,int>> maximumCardinality();
    double totalWeight() const { return m_totalWeight; }
    int matchingSize() const { return m_matching.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void matchingFound(int size, double weight);
private:
    int m_n = 0; double m_totalWeight = 0.0;
    QVector<QVector<QPair<int,double>>> m_adj;
    QVector<QPair<int,int>> m_matching;
    Stats m_stats; double m_timeSum = 0.0;
};
