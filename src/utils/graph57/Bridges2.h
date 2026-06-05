/**
 * @file Bridges2.h
 * @brief 桥和割点2 — 双连通分量+边双缩点
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class Bridges2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSearches = 0;
        int totalVertices = 0;
        int totalBridges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Bridges2(QObject* parent = nullptr);

    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<QPair<int,int>> findBridges();
    QVector<int> findArticulationPoints();
    QVector<QVector<int>> biconnectedComponents();
    QVector<QVector<int>> edgeBiconnectedComponents();
    QVector<QPair<int,int>> bridgeTree() const;

    int vertices() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int bridges, int articPoints);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    QVector<QPair<int,int>> m_edges;

    void tarjanDFS(int u, int parent, QVector<int>& disc, QVector<int>& low,
                   int& timer, QVector<QPair<int,int>>& bridges,
                   QVector<int>& articPoints);

    Stats m_stats;
    double m_timeSum = 0.0;
};
