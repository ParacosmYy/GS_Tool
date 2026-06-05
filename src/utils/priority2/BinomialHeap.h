/**
 * @file BinomialHeap.h
 * @brief 二项堆(Binomial Heap)优先队列
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class BinomialHeap
 * @brief 二项堆 — 支持O(log n)插入/删除/合并的优先队列
 *
 * 二项堆由一组二项树组成，支持高效合并(union)操作。
 * 适用于需要频繁合并优先队列的场景(如图算法)。
 */
class BinomialHeap : public QObject
{
    Q_OBJECT

public:
    /** @brief 节点结构 */
    struct Node {
        double key;                /**< 键值 */
        QVariant data;             /**< 关联数据 */
        int degree;                /**< 子树度数 */
        Node* child;               /**< 第一个子节点 */
        Node* sibling;             /**< 兄弟节点 */
        Node* parent;              /**< 父节点 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;       /**< 总插入数 */
        int totalExtracts = 0;      /**< 总提取数 */
        int totalMerges = 0;        /**< 总合并数 */
        int totalDecreaseKeys = 0;  /**< 总减键数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit BinomialHeap(QObject* parent = nullptr);
    ~BinomialHeap();

    /** @brief 插入元素 */
    void insert(double key, const QVariant& data = QVariant());

    /** @brief 获取最小键值 */
    double findMin() const;

    /** @brief 提取最小元素，返回键值 */
    QPair<double, QVariant> extractMin();

    /** @brief 合并另一个堆 */
    void merge(BinomialHeap& other);

    /** @brief 减键操作 */
    void decreaseKey(Node* node, double newKey);

    /** @brief 删除指定节点 */
    void remove(Node* node);

    /** @brief 是否为空 */
    bool isEmpty() const;

    /** @brief 元素数量 */
    int size() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 提取最小值信号 */
    void minExtracted(double key);

private:
    Node* m_head;              /**< 堆头指针 */
    int m_count;               /**< 元素数量 */
    Stats m_stats;             /**< 统计 */
    double m_timeSum;          /**< 累计时间 */

    Node* mergeRoots(Node* h1, Node* h2);
    Node* unionHeaps(Node* h1, Node* h2);
    void linkTree(Node* child, Node* parent);
    void clearTree(Node* node);
    Node* findMinNode() const;
    Node* reverseList(Node* node);
};
