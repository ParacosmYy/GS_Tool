#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BPlusTree7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit BPlusTree7(QObject* parent = nullptr);
    void setOrder(int order);
    void insert(double key, int value);
    void remove(double key);
    bool contains(double key) const;
    QVector<int> rangeQuery(double lo, double hi) const;
    int size() const { return m_size; }
    int height() const { return m_height; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void inserted(double key);
private:
    int m_order = 64; int m_size = 0; int m_height = 0;
    struct BPNode { bool leaf; QVector<double> keys; QVector<int> vals; QVector<BPNode*> children; BPNode* parent; BPNode* next; };
    BPNode* m_root = nullptr;
    BPNode* findLeaf(double key) const;
    void splitLeaf(BPNode* leaf);
    void splitInternal(BPNode* node);
    void rangeQueryLeaf(BPNode* leaf, double hi, QVector<int>& res) const;
    Stats m_stats; double m_timeSum = 0.0;
};
