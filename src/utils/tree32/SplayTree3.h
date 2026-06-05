/**
 * @file SplayTree3.h
 * @brief Splay树增强 — 顺序统计/区间操作/懒标记/持久化快照
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class SplayTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit SplayTree3(QObject* parent = nullptr);
    ~SplayTree3();
    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    double selectKth(int k) const;
    int rank(double key) const;
    double rangeSum(double lo, double hi) const;
    int size() const { return m_size; }
    bool isEmpty() const { return m_size == 0; }
    void clear();
    QVector<QPair<double,int>> inOrderTraversal() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
    void removed(double key);
private:
    struct Node {
        double key; int value; int size; double sum;
        bool lazyReverse;
        Node *left, *right, *parent;
    };
    void splay(Node* x);
    void rotate(Node* x);
    void pushDown(Node* n);
    void updateSize(Node* n);
    Node* findNode(double key) const;
    void destroyTree(Node* n);
    Node* m_root = nullptr;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;
};
