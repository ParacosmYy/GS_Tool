#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class EulerTour3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalBuilds = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit EulerTour3(QObject* parent = nullptr);
    void setTree(int n, int root, const QVector<QPair<int,int>>& edges);
    int lca(int u, int v) const;
    int distance(int u, int v) const;
    QVector<int> path(int u, int v) const;
    int depth(int node) const { return m_depth[node]; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void buildCompleted(int n);
private:
    int m_n = 0; int m_root = 0;
    QVector<QVector<int>> m_adj;
    QVector<int> m_euler; QVector<int> m_depth;
    QVector<int> m_first; QVector<QVector<int>> m_sparse;
    void dfs(int u, int p, int d);
    int rmq(int i, int j) const;
    Stats m_stats; double m_timeSum = 0.0;
};
