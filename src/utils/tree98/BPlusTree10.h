#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief B+树实现
 *
 * 多路平衡搜索树,所有数据存储在叶子节点并通过链表串联,
 * 支持高效范围查询,广泛应用于数据库索引与文件系统。
 */
class BPlusTree10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit BPlusTree10(QObject* parent = nullptr);

    /** @brief 设置树的阶数(最大子节点数) */
    void setOrder(int order);

    /** @brief 插入键值对 */
    void insert(double key, int value);

    /** @brief 移除指定键 */
    void remove(double key);

    /** @brief 范围查询,返回[minKey, maxKey]内的所有键值对 */
    void rangeQuery(double minKey, double maxKey);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void inserted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_order = 4;
};
