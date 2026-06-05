#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVLTree12 - AVL平衡二叉搜索树第12代实现
 *
 * 提供自平衡BST操作，支持插入、删除、查找及范围查询，
 * LL/RR/LR/RL四种旋转自动维护平衡因子。
 */
class AVLTree12 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTreeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit AVLTree12(QObject* parent = nullptr);
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
     * @brief 范围查询 [minKey, maxKey] 内的所有键值对
     * @param minKey 最小键
     * @param maxKey 最大键
     * @return 范围内的键值对列表
     */
    QVector<QPair<int, double>> rangeQuery(int minKey, int maxKey) const;

    /**
     * @brief 获取树的节点总数
     * @return 节点数量
     */
    int size() const;

signals:
    void treeOperationCompleted(int currentSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
