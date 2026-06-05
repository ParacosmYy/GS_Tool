#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Treap11 - 树堆（Treap）第11代实现
 *
 * 结合二叉搜索树和堆性质的概率平衡数据结构，
 * 通过随机优先级维护平衡，支持分裂/合并操作。
 */
class Treap11 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTreeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit Treap11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 插入键值对（随机优先级）
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
     * @brief 按键值分裂为两棵树
     * @param splitKey 分裂键值
     * @return 两棵子树的键值对 (≤splitKey, >splitKey)
     */
    QPair<QVector<QPair<int, double>>, QVector<QPair<int, double>>> split(int splitKey);

    /**
     * @brief 获取第k小的键值对
     * @param k 排名（1-based）
     * @return 第k小的键值对
     */
    QPair<int, double> kthElement(int k) const;

signals:
    void treeOperationCompleted(int currentSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
