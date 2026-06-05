#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Treap6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit Treap6(QObject* parent = nullptr);
    void insert(double key, int value);
    void remove(double key);
    bool contains(double key) const;
    int rank(double key) const;
    double select(int k) const;
    void split(double key, Treap6& left, Treap6& right);
    void merge(Treap6& other);
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
private:
    int m_size = 0;
    struct TNode { double key; int val, priority, count; TNode *left, *right; };
    TNode* m_root = nullptr;
    TNode* insertNode(TNode* n, double key, int val);
    TNode* removeNode(TNode* n, double key);
    TNode* rotateLeft(TNode* n);
    TNode* rotateRight(TNode* n);
    void splitNode(TNode* n, double key, TNode*& l, TNode*& r);
    TNode* mergeNode(TNode* l, TNode* r);
    Stats m_stats; double m_timeSum = 0.0;
};
