/**
 * @file BPlusTree5.h
 * @brief B+树5 — 批量加载+范围扫描优化
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class BPlusTree5 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalRangeScans = 0;
        int totalSplits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree5(int order = 32, QObject* parent = nullptr);
    ~BPlusTree5();

    void insert(double key, int value);
    void insertBatch(const QVector<QPair<double,int>>& items);
    bool remove(double key);
    int find(double key) const;

    QVector<QPair<double,int>> rangeScan(double low, double high) const;
    int countRange(double low, double high) const;
    void clear();

    int order() const { return m_order; }
    int height() const;
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void batchLoadCompleted(int count, int height);

private:
    struct Node {
        bool isLeaf;
        QVector<double> keys;
        QVector<int> values;
        QVector<Node*> children;
        Node* next = nullptr;
    };

    int m_order;
    int m_size = 0;
    Node* m_root = nullptr;

    Node* findLeaf(double key) const;
    void splitLeaf(Node* leaf);
    void splitInternal(Node* internal);
    Node* bulkLoad(const QVector<QPair<double,int>>& sorted);
    void destroyTree(Node* node);

    Stats m_stats;
    double m_timeSum = 0.0;
};
