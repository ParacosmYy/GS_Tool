/**
 * @file SteinerTree2.h
 * @brief Steiner树2 — SPH近似+度量闭包
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class SteinerTree2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalVertices = 0;
        int totalSteinerNodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SteinerTree2(QObject* parent = nullptr);

    void setGraph(int n, const QVector<QPair<QPair<int,int>,double>>& edges);
    void setTerminals(const QVector<int>& terminals);
    double solve();
    QVector<QPair<int,int>> treeEdges() const { return m_treeEdges; }
    double treeCost() const { return m_cost; }

    int vertices() const { return m_n; }
    int numTerminals() const { return m_terminals.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(double cost, int edges, int steinerNodes);

private:
    int m_n = 0;
    QVector<int> m_terminals;
    QVector<QVector<QPair<int,double>>> m_adj;
    QVector<QPair<int,int>> m_treeEdges;
    double m_cost = 0.0;

    QVector<double> dijkstra(int src) const;
    QVector<QVector<double>> metricClosure() const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
