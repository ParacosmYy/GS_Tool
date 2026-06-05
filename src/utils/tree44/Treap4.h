/**
 * @file Treap4.h
 * @brief Treap4 — 持久化Treap+可分裂合并
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class Treap4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalSplits = 0;
        int totalMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap4(QObject* parent = nullptr);
    ~Treap4();

    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;

    QPair<Treap4*, Treap4*> split(double key);
    static Treap4* merge(Treap4* left, Treap4* right);

    int size() const { return m_size; }
    bool isEmpty() const { return m_size == 0; }
    void clear();
    double kth(int k) const;
    int rank(double key) const;
    QVector<QPair<double,int>> inOrderTraversal() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void splitCompleted(int leftSize, int rightSize);
    void mergeCompleted(int totalSize);

private:
    struct Node {
        double key;
        int value;
        int priority;
        int subtreeSize;
        Node *left, *right;
    };

    Node* insertNode(Node* root, double key, int value);
    Node* removeNode(Node* root, double key);
    QPair<Node*, Node*> splitNode(Node* root, double key);
    Node* mergeNode(Node* left, Node* right);
    void updateSize(Node* n);
    Node* rotateLeft(Node* x);
    Node* rotateRight(Node* y);
    void destroyTree(Node* n);

    Node* m_root = nullptr;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
