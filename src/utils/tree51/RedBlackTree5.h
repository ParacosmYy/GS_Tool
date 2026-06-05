#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class RedBlackTree5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit RedBlackTree5(QObject* parent = nullptr);
    ~RedBlackTree5();
    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    int size() const { return m_size; }
    void clear();
    QVector<QPair<double,int>> inOrderTraversal() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeModified(int size);
private:
    enum Color { Red, Black };
    struct Node { double key; int value; Color color; int subsize; Node *left, *right, *parent; };
    Node* m_nil; Node* m_root; int m_size = 0;
    void leftRotate(Node* x); void rightRotate(Node* y);
    void insertFixup(Node* z); void deleteFixup(Node* x);
    void transplant(Node* u, Node* v); Node* minimum(Node* x) const;
    void destroyTree(Node* n);
    Stats m_stats; double m_timeSum = 0.0;
};
