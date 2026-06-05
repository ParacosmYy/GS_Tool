#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Matching5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalMatches = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit Matching5(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v, double weight);
    QVector<QPair<int,int>> maxCardinality();
    QVector<QPair<int,int>> maxWeight();
    double totalWeight() const { return m_totalWeight; }
    int matchingSize() const { return m_matchSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void matchingCompleted(int size, double weight);
private:
    int m_n = 0; double m_totalWeight = 0.0; int m_matchSize = 0;
    QVector<QVector<QPair<int,double>>> m_adj;
    bool augment(int u, QVector<bool>& visited, QVector<int>& match);
    Stats m_stats; double m_timeSum = 0.0;
};
