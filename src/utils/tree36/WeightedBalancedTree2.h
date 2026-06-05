/**
 * @file WeightedBalancedTree2.h
 * @brief Weighted balanced tree enhanced - weight-based rebalance/scalar context
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class WeightedBalancedTree2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit WeightedBalancedTree2(double alpha = 0.29, QObject* parent = nullptr);
    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    double selectKth(int k) const;
    int rank(double key) const;
    int size() const { return m_size; }
    void clear();
    QVector<QPair<double,int>> inOrderTraversal() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
    void removed(double key);
private:
    struct Node { double key; int value; int weight; int totalWeight;
        Node *left, *right, *parent; };
    Node* rebuildSubtree(Node* node);
    void flatten(Node* node, QVector<Node*>& nodes);
    Node* buildBalanced(QVector<Node*>& nodes, int start, int end);
    void updateWeights(Node* n);
    bool needsRebalance(Node* n) const;
    void destroyTree(Node* n);
    Node* m_root = nullptr; int m_size = 0; double m_alpha = 0.29;
    Stats m_stats; double m_timeSum = 0.0;
};
