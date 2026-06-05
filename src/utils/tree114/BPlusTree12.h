#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B+树实现
 *
 * 多路平衡搜索树，所有数据存储在叶子节点并通过链表串联，
 * 内部节点仅存储键用于导航，支持高效范围查询和顺序遍历，
 * 广泛用于数据库索引和文件系统组织。
 */
class BPlusTree12 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit BPlusTree12(QObject* parent = nullptr);

    /** @brief 设置B+树的阶数(每个内部节点最多包含order个子节点) */
    void setOrder(int order);

    /** @brief 插入键值对，必要时分裂节点并向上传播 */
    void insert(int key, const QVariant& value);

    /** @brief 查找指定键，返回关联值 */
    QVariant search(int key) const;

    /** @brief 范围查询，返回[minKey, maxKey]内的所有键值对 */
    QVector<QPair<int, QVariant>> rangeQuery(int minKey, int maxKey) const;

    /** @brief 删除指定键，必要时合并或 redistribution 节点 */
    bool remove(int key);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 操作完成信号，返回操作类型和当前树高度 */
    void operationCompleted(const QString& operationType, int treeHeight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_order = 64;
};
