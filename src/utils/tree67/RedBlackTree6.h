#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class RedBlackTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit RedBlackTree6(QObject* parent = nullptr);
    void insert(double key, int value);
    void remove(double key);
    bool contains(double key) const;
    int rank(double key) const;
    double select(int k) const;
    QVector<int> rangeQuery(double lo, double hi) const;
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
private:
    int m_size = 0;
    enum Color { RED, BLACK };
    struct RBNode { double key; int val; Color color; RBNode *left, *right, *parent; int count; };
    RBNode* m_root = nullptr; RBNode* m_nil = nullptr;
    RBNode* insertNode(RBNode* z);
    void insertFixup(RBNode* z);
    void leftRotate(RBNode* x);
    void rightRotate(RBNode* y);
    void rangeQueryNode(RBNode* n, double lo, double hi, QVector<int>& res) const;
    Stats m_stats; double m_timeSum = 0.0;
};
