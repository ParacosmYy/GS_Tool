#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B+树实现
 *
 * 多路平衡搜索树，所有数据存储在叶子节点并通过链表串联，
 * 广泛用于数据库索引和文件系统。
 */
class BPlusTree11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit BPlusTree11(QObject* parent = nullptr);

    /** @brief 设置B+树阶数(最大子节点数) */
    void setOrder(int order);

    /** @brief 插入键值对 */
    void insert(double key, int value);

    /** @brief 删除指定键 */
    void remove(double key);

    /** @brief 范围查询[minKey, maxKey]内的所有键值对 */
    QVector<QPair<double,int>> rangeQuery(double minKey, double maxKey);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void inserted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_order = 4;
};
