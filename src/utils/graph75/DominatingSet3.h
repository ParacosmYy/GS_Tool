#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DominatingSet3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit DominatingSet3(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<int> solve();
    int setSize() const { return m_setSize; }
    bool isDominating(const QVector<int>& set) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int size);
private:
    int m_n = 0; int m_setSize = 0;
    QVector<QVector<int>> m_adj;
    QVector<int> greedyDominating();
    Stats m_stats; double m_timeSum = 0.0;
};
