/**
 * @file AVLTree4.h
 * @brief AVL树4 — 批量构建+范围查询优化
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class AVLTree4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalRangeQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AVLTree4(QObject* parent = nullptr);
    ~AVLTree4();

    void insert(double key, int value);
    void insertBatch(const QVector<QPair<double,int>>& items);
    bool remove(double key);
    int find(double key) const;

    QVector<QPair<double,int>> rangeQuery(double low, double high) const;
    int rangeCount(double low, double high) const;
    double kth(int k) const;
    int rank(double key) const;

    int size() const { return m_size; }
    bool isEmpty() const { return m_size == 0; }
    void clear();
    int height() const;
    QVector<QPair<double,int>> inOrderTraversal() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void batchInsertCompleted(int count, int treeHeight);

private:
    struct Node {
        double key;
        int value;
        int height;
        int subtreeSize;
        Node *left, *right;
    };

    Node* insertNode(Node* node, double key, int value);
    Node* removeNode(Node* node, double key);
    Node* balance(Node* node);
    Node* rotateLeft(Node* x);
    Node* rotateRight(Node* y);
    int getBalance(Node* node) const;
    void updateHeight(Node* node);
    void updateSize(Node* node);
    Node* buildBalanced(const QVector<QPair<double,int>>& sorted, int lo, int hi);
    void destroyTree(Node* n);

    Node* m_root = nullptr;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
