#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief RedBlackTree12 - 红黑树第12代实现
 *
 * 提供红黑自平衡BST操作，保证O(log n)的最坏情况性能，
 * 支持插入、删除、查找、前驱/后继及顺序遍历。
 */
class RedBlackTree12 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTreeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit RedBlackTree12(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     */
    void insert(int key, double value);

    /**
     * @brief 删除指定键
     * @param key 待删除的键
     * @return 是否删除成功
     */
    bool remove(int key);

    /**
     * @brief 查找指定键对应的值
     * @param key 待查找的键
     * @return 键对应的值，未找到返回0.0
     */
    double search(int key) const;

    /**
     * @brief 查找指定键的前驱节点
     * @param key 目标键
     * @return 前驱键值对，不存在则返回 (-1, 0.0)
     */
    QPair<int, double> predecessor(int key) const;

    /**
     * @brief 中序遍历获取所有键值对
     * @return 有序键值对列表
     */
    QVector<QPair<int, double>> inOrderTraversal() const;

signals:
    void treeOperationCompleted(int currentSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
