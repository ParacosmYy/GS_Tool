#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BPlusTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit BPlusTree6(int order = 16, QObject* parent = nullptr);
    ~BPlusTree6();
    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    QVector<QPair<double,int>> rangeScan(double lo, double hi) const;
    void clear();
    int size() const { return m_size; }
    int height() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeModified(int size);
private:
    struct Node { bool leaf; QVector<double> keys; QVector<int> vals; QVector<Node*> kids; Node* next = nullptr; };
    int m_order; int m_size = 0; Node* m_root = nullptr;
    Node* findLeaf(double key) const;
    void splitLeaf(Node* n); void splitInternal(Node* n);
    void destroyTree(Node* n);
    Stats m_stats; double m_timeSum = 0.0;
};
