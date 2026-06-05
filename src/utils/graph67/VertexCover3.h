#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class VertexCover3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit VertexCover3(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> solveApprox();
    QVector<int> solveExact();
    bool isVertexCover(const QVector<int>& cover) const;
    int vertices() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solved(int coverSize);
private:
    int m_n = 0;
    QVector<QVector<int>> m_adj; QVector<QPair<int,int>> m_edges;
    Stats m_stats; double m_timeSum = 0.0;
};
