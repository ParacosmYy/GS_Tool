/**
 * @file TopologicalSort2.h
 * @brief 拓扑排序2 — Kahn+BFS+最长路径
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class TopologicalSort2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSorts = 0;
        int totalVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TopologicalSort2(QObject* parent = nullptr);

    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> sort();
    QVector<int> kahnSort() const;
    QVector<int> dfsSort() const;
    bool hasCycle() const;
    QVector<QVector<int>> allTopologicalSorts() const;
    int longestPath() const;
    QVector<int> criticalPath() const;

    int vertices() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sortCompleted(int vertices, bool hasCycle);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    QVector<int> m_inDegree;

    void dfsVisit(int u, QVector<bool>& visited, QVector<bool>& onStack,
                  QVector<int>& result, bool& cycle) const;
    void allSortsDFS(QVector<int>& result, QVector<bool>& visited,
                      QVector<int>& inDeg,
                      QVector<QVector<int>>& allResults, int limit) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
