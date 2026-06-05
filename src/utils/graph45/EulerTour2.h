/**
 * @file EulerTour2.h
 * @brief 欧拉环游树增强 — 链接/剪切/路径查询/森林操作
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class EulerTour2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };
    explicit EulerTour2(int n, QObject* parent = nullptr);
    void link(int u, int v);
    void cut(int u, int v);
    bool connected(int u, int v) const;
    int componentSize(int u) const;
    int lca(int u, int v) const;
    double pathAggregate(int u, int v) const;
    int numTrees() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeModified(const QString& operation, int u, int v);
private:
    int findRoot(int u) const;
    int nodeDepth(int u) const;
    int computeSubtreeSize(int u);
    void updateAncestorSizes(int u, int delta);
    void resetSubtreeParent(int u);
    int m_n;
    QVector<int> m_parent; QVector<int> m_rank; QVector<int> m_size;
    QVector<QVector<int>> m_children;
    Stats m_stats; double m_timeSum = 0.0;
};
