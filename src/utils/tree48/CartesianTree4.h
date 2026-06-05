/**
 * @file CartesianTree4.h
 * @brief 笛卡尔树4 — 可持久化+RMQ/LCA
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class CartesianTree4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalBuilds = 0;
        int totalQueries = 0;
        int totalNodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CartesianTree4(QObject* parent = nullptr);
    ~CartesianTree4();

    void build(const QVector<double>& values);
    int rangeMinimum(int lo, int hi) const;
    double rangeMinimumValue(int lo, int hi) const;
    int lca(int u, int v) const;
    QVector<int> eulerTour() const;
    QVector<int> depthArray() const;

    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int n, int height);

private:
    struct Node {
        int parent, left, right;
        double value;
        int index;
    };

    int m_n = 0;
    int m_root = -1;
    QVector<Node> m_nodes;
    QVector<int> m_euler;
    QVector<int> m_depth;
    QVector<int> m_first;
    QVector<QVector<int>> m_sparTable;

    void buildRMQ();
    int rmqQuery(int i, int j) const;
    void destroyTree();

    Stats m_stats;
    double m_timeSum = 0.0;
};
