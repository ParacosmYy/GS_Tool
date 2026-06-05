#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SplayTree5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalSplays = 0; double avgProcessingTimeMs = 0.0; };
    explicit SplayTree5(QObject* parent = nullptr);
    ~SplayTree5();
    void insert(double key, int value);
    bool remove(double key);
    int find(double key);
    int size() const { return m_size; }
    void clear();
    QVector<QPair<double,int>> inOrderTraversal() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeModified(int size);
private:
    struct Node { double key; int val; Node *l, *r, *p; };
    Node* m_root = nullptr; int m_size = 0;
    void splay(Node* x);
    void rotL(Node* x); void rotR(Node* x);
    void destroy(Node* n);
    Stats m_stats; double m_timeSum = 0.0;
};
