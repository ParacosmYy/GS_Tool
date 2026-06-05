#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class EdgeColoring2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalColorings = 0; int totalEdgesProcessed = 0; double avgProcessingTimeMs = 0.0; int bestColors = 0; };
    explicit EdgeColoring2(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> color();
    int numColors() const;
    bool isValid(const QVector<int>& colors) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void coloringComplete(int colors);
private:
    int m_n = 0;
    QVector<QPair<int,int>> m_edges;
    Stats m_stats; double m_timeSum = 0.0;
};
