/**
 * @file CartesianTree3.h
 * @brief 笛卡尔树增强 — Treap风格/区间最值/可持久化/RMQ
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class CartesianTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalBuilds = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit CartesianTree3(QObject* parent = nullptr);
    ~CartesianTree3();
    void build(const QVector<double>& values);
    double rangeMinimum(int lo, int hi) const;
    double rangeMaximum(int lo, int hi) const;
    int lca(int i, int j) const;
    QVector<QPair<double,int>> inorder() const;
    int size() const { return m_size; }
    void clear();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void buildComplete(int n);
    void queryComplete(int lo, int hi, double result);
private:
    struct Node { double value; int index; Node *left, *right, *parent; };
    void destroyTree(Node* n);
    void inorderHelper(Node* n, QVector<QPair<double,int>>& result) const;
    Node* m_root = nullptr; int m_size = 0;
    QVector<Node*> m_nodes;
    Stats m_stats; double m_timeSum = 0.0;
};
