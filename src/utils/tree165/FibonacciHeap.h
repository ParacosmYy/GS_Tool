/**
 * @file FibonacciHeap.h
 * @brief 斐波那契堆(懒惰合并+O(1)降键) — Fibonacci Heap with Lazy Consolidation and O(1) Amortized Decrease-Key
 *
 * 功能: 实现斐波那契堆，支持O(1)插入、查找最小、降键和合并。
 *       delete-min使用懒惰合并(consolidate)摊还O(log n)。
 *       支持按ID降键和删除指定元素。
 *
 * 协作: PairingHeap(配对堆) / LeftistHeap(左偏堆) / BinomialHeap(二项堆)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

/**
 * @brief 斐波那契堆
 */
class FibonacciHeap : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalDeleteMins = 0;        ///< 累计删除最小次数
        quint64 totalDecreaseKeys = 0;      ///< 累计降键次数
        quint64 totalConsolidations = 0;    ///< 累计合并轮次
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        int currentSize = 0;                ///< 当前元素数
    };

    explicit FibonacciHeap(QObject* parent = nullptr);
    ~FibonacciHeap() override;

    /**
     * @brief 插入键值对
     * @param key 优先级键
     * @param value 关联值
     * @return 元素ID(用于decreaseKey/remove)
     */
    int insert(double key, double value);

    /** @brief 获取最小键值 */
    bool findMin(double& key, double& value) const;

    /** @brief 删除并返回最小元素 */
    bool deleteMin(double& key, double& value);

    /**
     * @brief 降低指定元素的键值(O(1)摊还)
     * @param id 元素ID
     * @param newKey 新键值(必须小于当前键)
     */
    bool decreaseKey(int id, double newKey);

    /**
     * @brief 删除指定元素
     * @param id 元素ID
     */
    bool remove(int id);

    /** @brief 合并另一个斐波那契堆 */
    void merge(FibonacciHeap& other);

    bool isEmpty() const;
    int size() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteMinCompleted(double key);

private:
    /** @brief 堆节点 */
    struct Node {
        double key;
        double value;
        int id;
        int degree;         ///< 子节点数
        bool marked;        ///< 是否被标记(失去子节点)
        Node* parent;
        Node* child;        ///< 任意一个子节点
        Node* left;         ///< 双向链表左兄弟
        Node* right;        ///< 双向链表右兄弟
    };

    /** @brief 合并(懒惰): 将other根链表拼接到当前根链表 */
    void lazyMerge(FibonacciHeap& other);

    /** @brief 合并: delete-min后整理根链表 */
    void consolidate();

    /** @brief 链接: 将y挂到x的子列表 */
    void link(Node* child, Node* parent);

    /** @brief 切断: 将节点从父节点剪下 */
    void cut(Node* x, Node* parent);

    /** @brief 级联切断 */
    void cascadingCut(Node* x);

    /** @brief 将节点插入根链表 */
    void insertIntoRootList(Node* node);

    /** @brief 从链表中移除节点 */
    void removeFromList(Node* node);

    /** @brief 释放子树 */
    void deleteTree(Node* node);

    /** @brief 更新最小指针 */
    void updateMin();

    Node* m_min = nullptr;
    int m_nextId = 0;
    QHash<int, Node*> m_nodeMap;

    Stats m_stats;
    double m_timeSum = 0.0;
};
