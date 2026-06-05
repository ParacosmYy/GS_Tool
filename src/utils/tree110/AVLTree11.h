#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVL自平衡二叉搜索树实现 (版本11)
 *
 * 通过严格的平衡因子(|bf|<=1)和四种旋转操作保持O(log n)操作复杂度，
 * 支持插入、删除、查找和范围查询，适用于有序数据的高效动态维护。
 */
class AVLTree11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit AVLTree11(QObject* parent = nullptr);

    /** @brief 插入键值对，插入后自动平衡 */
    void insert(int key, const QVariant& value);

    /** @brief 查找指定键并返回关联值 */
    QVariant search(int key) const;

    /** @brief 删除指定键，删除后自动平衡 */
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
