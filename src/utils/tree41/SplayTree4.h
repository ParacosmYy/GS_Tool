/**
 * @file SplayTree4.h
 * @brief 伸展树4 — 指针优化+局部性感知
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SplayTree4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalSplays = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree4(QObject* parent = nullptr);
    ~SplayTree4();

    void insert(double key, int value);
    bool remove(double key);
    int find(double key);
    double kth(int k) const;
    int rank(double key) const;

    int size() const { return m_size; }
    bool isEmpty() const { return m_size == 0; }
    void clear();
    QVector<QPair<double,int>> inOrderTraversal() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeModified(const QString& operation, int newSize);

private:
    struct Node {
        double key;
        int value;
        int subtreeSize;
        Node *left, *right, *parent;
    };

    void splay(Node* x);
    void leftRotate(Node* x);
    void rightRotate(Node* x);
    void updateSize(Node* x);
    Node* findNode(double key) const;
    void destroyTree(Node* n);

    Node* m_root;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
