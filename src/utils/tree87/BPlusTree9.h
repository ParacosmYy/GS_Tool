#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B+树索引结构
 *
 * 磁盘友好的有序索引，所有值存储在叶子节点，支持范围查询。
 */
class BPlusTree9 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalInsertions = 0;
        int totalSplits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree9(int order = 4, QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(int key, const QVariant& value);

    /** @brief 范围查询 [lo, hi] */
    QVector<QPair<int, QVariant>> rangeQuery(int lo, int hi) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nodeSplit(int level, int keysPromoted);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_order = 4;
    struct BPlusNode* m_root = nullptr;
};
