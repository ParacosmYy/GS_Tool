#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class WeightBalancedTree2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit WeightBalancedTree2(double alpha = 0.29, QObject* parent = nullptr);
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
    struct Node { double key; int value; int weight; int totalWeight; Node *left, *right, *parent; };
    Node* rebalance(Node* n);
    void destroyTree(Node* n);
    Node* m_root = nullptr; int m_size = 0; double m_alpha = 0.29;
    Stats m_stats; double m_timeSum = 0.0;
};
