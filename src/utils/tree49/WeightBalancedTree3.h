/**
 * @file WeightBalancedTree3.h
 * @brief 权重平衡树3 — BB[alpha]+加权旋转
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class WeightBalancedTree3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WeightBalancedTree3(double alpha = 0.288, QObject* parent = nullptr);
    ~WeightBalancedTree3();

    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    int rank(double key) const;
    double kth(int k) const;

    int size() const { return m_size; }
    bool isEmpty() const { return m_size == 0; }
    void clear();
    int height() const;
    QVector<QPair<double,int>> inOrderTraversal() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rebalanceCompleted(int rotations, int newSize);

private:
    struct Node {
        double key;
        int value;
        int weight;
        int subtreeWeight;
        Node *left, *right;
    };

    double m_alpha;
    Node* m_root = nullptr;
    int m_size = 0;

    Node* insertNode(Node* node, double key, int value);
    Node* removeNode(Node* node, double key);
    Node* balance(Node* node);
    Node* rotateLeft(Node* x);
    Node* rotateRight(Node* y);
    Node* buildBalanced(const QVector<QPair<double,int>>& items, int lo, int hi);
    void destroyTree(Node* n);

    Stats m_stats;
    double m_timeSum = 0.0;
};
