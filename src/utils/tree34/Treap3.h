/**
 * @file Treap3.h
 * @brief Treap增强 — 顺序统计/区间加/区间求和/懒标记下推
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class Treap3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit Treap3(QObject* parent = nullptr);
    ~Treap3();
    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    double selectKth(int k) const;
    int rank(double key) const;
    double rangeSum(double lo, double hi) const;
    void rangeAdd(double lo, double hi, double delta);
    int size() const { return m_size; }
    void clear();
    QVector<QPair<double,int>> inOrderTraversal() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
    void removed(double key);
private:
    struct Node { double key; int value; int priority; int size; double sum; double lazy;
        Node *left, *right; };
    void pushDown(Node* n);
    void pullUp(Node* n);
    Node* rotateRight(Node* y);
    Node* rotateLeft(Node* x);
    Node* insert(Node* root, double key, int value);
    Node* remove(Node* root, double key);
    void destroyTree(Node* n);
    Node* m_root = nullptr; int m_size = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
