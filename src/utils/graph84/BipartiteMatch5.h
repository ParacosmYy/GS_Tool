#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BipartiteMatch5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalMatches = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit BipartiteMatch5(QObject* parent = nullptr);
    void setLeftSize(int n);
    void setRightSize(int m);
    void addEdge(int u, int v);
    int maxCardinality();
    QVector<QPair<int,int>> maxCardinalityMatching();
    QVector<int> vertexCover();
    bool isPerfect() const { return m_perfect; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void matchingCompleted(int size);
private:
    int m_left = 0; int m_right = 0; bool m_perfect = false;
    QVector<QVector<int>> m_adj;
    bool augment(int u, QVector<bool>& visited, QVector<int>& matchR);
    Stats m_stats; double m_timeSum = 0.0;
};
