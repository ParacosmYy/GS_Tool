#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BipartiteMatch4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalMatches = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit BipartiteMatch4(QObject* parent = nullptr);
    void setLeftSize(int n);
    void setRightSize(int m);
    void addEdge(int u, int v, double weight);
    QVector<QPair<int,int>> maxMatching();
    QVector<QPair<int,int>> maxWeightMatching();
    int matchingSize() const { return m_matchSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void matchingCompleted(int size, double totalWeight);
private:
    int m_left = 0; int m_right = 0; int m_matchSize = 0;
    QVector<QVector<QPair<int,double>>> m_adj;
    bool dfs(int u, QVector<bool>& visited, QVector<int>& matchR);
    Stats m_stats; double m_timeSum = 0.0;
};
