/**
 * @file GraphColoring4.h
 * @brief 图着色4 — DSATUR+回溯精确着色
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class GraphColoring4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalColorings = 0;
        int totalVertices = 0;
        int chromaticNumber = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring4(QObject* parent = nullptr);

    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> colorDSATUR();
    QVector<int> colorGreedy();
    QVector<int> colorBacktrack();
    bool isValidColoring(const QVector<int>& coloring) const;

    int chromaticNumber() const { return m_chromatic; }
    int vertices() const { return m_n; }
    int numColors() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colors, int vertices);

private:
    int m_n = 0;
    int m_chromatic = 0;
    QVector<QVector<int>> m_adj;
    QVector<int> m_bestColoring;

    int selectDSATUR(const QVector<int>& colors,
                      const QVector<int>& saturation) const;
    bool backtrackTry(QVector<int>& colors, int vertex, int maxColors);

    Stats m_stats;
    double m_timeSum = 0.0;
};
