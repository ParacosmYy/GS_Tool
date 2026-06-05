#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class WeightBalancedTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit WeightBalancedTree3(QObject* parent = nullptr);
    void setBalanceFactor(double alpha);
    void insert(double key, int value);
    void remove(double key);
    bool contains(double key) const;
    int rank(double key) const;
    int size() const { return m_size; }
    int height() const { return m_height; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
private:
    int m_size = 0; int m_height = 0; double m_alpha = 0.29;
    struct WBNode { double key; int val, count, height; WBNode *left, *right; };
    WBNode* m_root = nullptr;
    WBNode* insertNode(WBNode* n, double key, int val);
    WBNode* balance(WBNode* n);
    WBNode* rotateLeft(WBNode* n);
    WBNode* rotateRight(WBNode* n);
    int nodeSize(WBNode* n) const;
    Stats m_stats; double m_timeSum = 0.0;
};
