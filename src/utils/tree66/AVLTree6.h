#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class AVLTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit AVLTree6(QObject* parent = nullptr);
    void insert(double key, int value);
    void remove(double key);
    bool contains(double key) const;
    QVector<int> rangeQuery(double lo, double hi) const;
    int rank(double key) const;
    double select(int k) const;
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
private:
    int m_size = 0;
    struct AVLNode { double key; int val, count, height; AVLNode *left, *right; };
    AVLNode* m_root = nullptr;
    AVLNode* insertNode(AVLNode* n, double key, int val);
    AVLNode* removeNode(AVLNode* n, double key);
    AVLNode* balance(AVLNode* n);
    int height(AVLNode* n) const;
    int balanceFactor(AVLNode* n) const;
    AVLNode* rotateLeft(AVLNode* n);
    AVLNode* rotateRight(AVLNode* n);
    void rangeQueryNode(AVLNode* n, double lo, double hi, QVector<int>& res) const;
    Stats m_stats; double m_timeSum = 0.0;
};
