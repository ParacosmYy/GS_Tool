/**
 * @file GraphColoring3.h
 * @brief 图着色增强 — DSATUR/回溯/贪心/色数下界
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class GraphColoring3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalColorings = 0; int totalVerticesProcessed = 0; double avgProcessingTimeMs = 0.0; int bestColors = 0; };
    explicit GraphColoring3(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> greedyColor();
    QVector<int> dsaturColor();
    QVector<int> backtrackingColor(int maxColors);
    int chromaticLowerBound() const;
    int numColors(const QVector<int>& coloring) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void coloringComplete(int colors);
private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    Stats m_stats; double m_timeSum = 0.0;
};
