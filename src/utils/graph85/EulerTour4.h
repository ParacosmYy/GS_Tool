#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class EulerTour4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalNodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit EulerTour4(QObject* parent = nullptr);
    void setRooted(bool rooted);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<int> compute();
    bool hasEulerTour() const { return m_hasTour; }
    int tourLength() const { return m_tourLen; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int length, bool hasTour);
private:
    bool m_rooted = true; int m_n = 0; bool m_hasTour = false; int m_tourLen = 0;
    QVector<QVector<QPair<int,int>>> m_adj;
    void findEulerTour(int start, QVector<int>& tour, QVector<bool>& used);
    Stats m_stats; double m_timeSum = 0.0;
};
