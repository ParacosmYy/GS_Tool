/**
 * @file PairingHeap2.h
 * @brief 配对堆 — 简单高效的可合并堆
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 配对堆(Pairing Heap)
 * 两路合并+多路配对,decrease-key O(1)摊还
 */
class PairingHeap2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;          ///< 累计插入次数
        int totalDeletes = 0;             ///< 累计删除最小值次数
        int totalMelds = 0;               ///< 累计合并次数
        int totalDecreaseKeys = 0;        ///< 累计减键次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit PairingHeap2(QObject* parent = nullptr);
    ~PairingHeap2();

    /** @brief 插入元素 @param key 键 @param value 值 @return 节点句柄 */
    int insert(double key, int value);

    /** @brief 获取最小键 @return 最小键值 */
    QPair<double, int> findMin() const;

    /** @brief 删除最小元素 @return 被删除的(键,值) */
    QPair<double, int> deleteMin();

    /** @brief 减键 @param handle 节点句柄 @param newKey 新键(必须小于当前键) */
    void decreaseKey(int handle, double newKey);

    /** @brief 合并另一个堆 @param other 另一个PairingHeap2(合并后变空) */
    void meld(PairingHeap2& other);

    /** @brief 是否为空 */
    bool isEmpty() const { return m_root < 0; }

    /** @brief 获取元素数 */
    int size() const { return m_size; }

    /** @brief 清空 */
    void clear();

    /** @brief 获取所有元素(按键排序) */
    QVector<QPair<double, int>> toSortedVector() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 删除最小值 @param key 被删除的键 */
    void minDeleted(double key);

private:
    /** @brief 堆节点 */
    struct Node {
        double key;                ///< 键
        int value;                 ///< 值
        int child = -1;            ///< 最左子节点
        int sibling = -1;          ///< 右兄弟
        int parent = -1;           ///< 父节点
        bool active = true;        ///< 是否活跃
    };

    /** @brief 两路合并 */
    int mergePairs(int first);

    /** @brief 合并两个子树 */
    int link(int a, int b);

    QVector<Node> m_nodes;         ///< 节点池
    int m_root = -1;               ///< 根节点
    int m_size = 0;                ///< 元素数

    Stats m_stats;
    double m_timeSum = 0.0;
};
