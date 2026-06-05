#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SplayTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit SplayTree6(QObject* parent = nullptr);
    void insert(double key, int value);
    void remove(double key);
    bool contains(double key);
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
    struct SNode { double key; int val, count; SNode *left, *right, *parent; };
    SNode* m_root = nullptr;
    void splay(SNode* n);
    void zig(SNode* n);
    void zigZig(SNode* n);
    void zigZag(SNode* n);
    void rangeQueryNode(SNode* n, double lo, double hi, QVector<int>& res) const;
    Stats m_stats; double m_timeSum = 0.0;
};
