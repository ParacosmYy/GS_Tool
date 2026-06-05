#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树(Red-Black Tree)实现
 *
 * 自平衡二叉搜索树，通过节点着色和旋转规则保证O(log n)操作复杂度，
 * 适用于有序关联容器、内存管理和区间查询等需要稳定性能的场景。
 */
class RedBlackTree11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit RedBlackTree11(QObject* parent = nullptr);

    /** @brief 插入键值对，插入后通过旋转和重新着色保持平衡 */
    void insert(int key, const QVariant& value);

    /** @brief 查找指定键，返回关联值(未找到返回无效QVariant) */
    QVariant search(int key) const;

    /** @brief 删除指定键，删除后自动修复红黑性质 */
    bool remove(int key);

    /** @brief 范围查询，返回[minKey, maxKey]区间内的所有键值对 */
    QVector<QPair<int, QVariant>> rangeQuery(int minKey, int maxKey) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 操作完成信号，返回操作类型和当前树大小 */
    void operationCompleted(const QString& operationType, int treeSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
