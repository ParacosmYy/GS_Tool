#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BPlusTree13 - B+树第13代实现
 *
 * 提供磁盘友好的B+树索引结构，支持有序插入/删除、
 * 范围查询、顺序遍历及节点分裂/合并。
 */
class BPlusTree13 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTreeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit BPlusTree13(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 设置B+树阶数（每个节点最大子节点数）
     * @param order 阶数
     */
    void setOrder(int order);

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     * @return 是否插入成功
     */
    bool insert(int key, double value);

    /**
     * @brief 删除指定键
     * @param key 待删除的键
     * @return 是否删除成功
     */
    bool remove(int key);

    /**
     * @brief 精确查找指定键
     * @param key 待查找的键
     * @return 键对应的值，未找到返回0.0
     */
    double search(int key) const;

    /**
     * @brief 范围查询 [minKey, maxKey]
     * @param minKey 最小键
     * @param maxKey 最大键
     * @return 范围内的键值对列表
     */
    QVector<QPair<int, double>> rangeQuery(int minKey, int maxKey) const;

    /**
     * @brief 获取B+树的层级数
     * @return 树的高度
     */
    int height() const;

signals:
    void treeOperationCompleted(int currentSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
