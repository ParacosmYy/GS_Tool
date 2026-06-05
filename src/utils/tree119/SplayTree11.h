#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SplayTree11 - 伸展树第11代实现
 *
 * 自调整二叉搜索树，最近访问的节点会被伸展至根，
 * 支持Zig/Zig-Zig/Zig-Zag旋转操作及区间操作。
 */
class SplayTree11 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTreeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit SplayTree11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     */
    void insert(int key, double value);

    /**
     * @brief 查找指定键并伸展至根
     * @param key 待查找的键
     * @return 键对应的值，未找到返回0.0
     */
    double search(int key);

    /**
     * @brief 删除指定键
     * @param key 待删除的键
     * @return 是否删除成功
     */
    bool remove(int key);

    /**
     * @brief 查找区间 [minKey, maxKey] 内的所有键值对
     * @param minKey 区间左端
     * @param maxKey 区间右端
     * @return 范围内的键值对列表
     */
    QVector<QPair<int, double>> rangeQuery(int minKey, int maxKey);

signals:
    void treeOperationCompleted(int currentSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
