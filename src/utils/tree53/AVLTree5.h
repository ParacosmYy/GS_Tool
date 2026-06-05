#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class AVLTree5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalRotations = 0; double avgProcessingTimeMs = 0.0; };
    explicit AVLTree5(QObject* parent = nullptr);
    ~AVLTree5();
    void insert(double key, int value);
    bool remove(double key);
    int find(double key) const;
    int size() const { return m_size; }
    void clear();
    int height() const;
    QVector<QPair<double,int>> inOrderTraversal() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeModified(int size);
private:
    struct Node { double key; int val; int ht; Node *l, *r; };
    Node* m_root = nullptr; int m_size = 0;
    Node* ins(Node* n, double k, int v);
    Node* rem(Node* n, double k);
    Node* bal(Node* n);
    Node* rotL(Node* x); Node* rotR(Node* y);
    int ht(Node* n) const;
    void destroy(Node* n);
    Stats m_stats; double m_timeSum = 0.0;
};
