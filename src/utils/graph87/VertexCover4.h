#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class VertexCover4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit VertexCover4(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<int> solve();
    int coverSize() const { return m_coverSize; }
    bool isVertexCover(const QVector<int>& cover) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int size);
private:
    int m_n = 0; int m_coverSize = 0;
    QVector<QVector<int>> m_adj;
    Stats m_stats; double m_timeSum = 0.0;
};
