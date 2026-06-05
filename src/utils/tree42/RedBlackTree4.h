/**
 * @file RedBlackTree4.h
 * @brief 红黑树4 — 内存池分配+迭代器支持
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class RedBlackTree4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalFinds = 0;
        int poolCapacity = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree4(int poolSize = 4096, QObject* parent = nullptr);
    ~RedBlackTree4();

    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    bool contains(double key) const;

    int size() const { return m_size; }
    bool isEmpty() const { return m_size == 0; }
    void clear();
    QVector<QPair<double,int>> inOrderTraversal() const;

    int poolUsed() const { return m_poolUsed; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeModified(const QString& op, int size);

private:
    enum Color { Red, Black };
    struct Node {
        double key;
        int value;
        Color color;
        int subtreeSize;
        int left, right, parent;
    };

    int allocNode();
    void freeNode(int idx);
    void leftRotate(int x);
    void rightRotate(int y);
    void insertFixup(int z);
    void deleteFixup(int x);
    void transplant(int u, int v);
    int treeMinimum(int x) const;
    void updateSize(int n);
    void destroyTree();

    QVector<Node> m_pool;
    QVector<int> m_freeList;
    int m_root = -1;
    int m_poolUsed = 0;
    int m_size = 0;
    int m_nil = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
