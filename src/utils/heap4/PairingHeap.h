/**
 * @file PairingHeap.h
 * @brief 配对堆(Pairing Heap)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <functional>

/**
 * @class PairingHeap
 * @brief 配对堆 — 支持高效合并/减键的简单优先队列
 *
 * 配对堆结构简单，实际性能优异。
 * 合并: O(1), 插入: O(1), 减键: O(1) amortized, 删除最小: O(log n) amortized。
 */
class PairingHeap : public QObject
{
    Q_OBJECT

public:
    /** @brief 节点句柄(不透明指针) */
    using Handle = void*;

    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;       /**< 总插入数 */
        int totalExtracts = 0;      /**< 总提取数 */
        int totalMerges = 0;        /**< 总合并数 */
        int totalDecreaseKeys = 0;  /**< 总减键数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit PairingHeap(QObject* parent = nullptr);
    ~PairingHeap();

    /** @brief 插入元素，返回句柄 */
    Handle insert(double key, const QVariant& data = QVariant());

    /** @brief 获取最小键值 */
    double findMin() const;

    /** @brief 提取最小元素 */
    QPair<double, QVariant> extractMin();

    /** @brief 减键 */
    void decreaseKey(Handle h, double newKey);

    /** @brief 删除指定元素 */
    void remove(Handle h);

    /** @brief 合并另一个堆(另一个堆被清空) */
    void merge(PairingHeap& other);

    /** @brief 是否为空 */
    bool isEmpty() const;

    /** @brief 元素数量 */
    int size() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 最小值提取信号 */
    void minExtracted(double key);

private:
    struct Node {
        double key;
        QVariant data;
        Node* child;
        Node* sibling;
        Node* prev;     /* 父节点或左兄弟 */
    };

    Node* m_root;
    int m_count;
    Stats m_stats;
    double m_timeSum;

    Node* mergeTrees(Node* a, Node* b);
    Node* twoPassMerge(Node* node);
    void deleteTree(Node* node);
};
