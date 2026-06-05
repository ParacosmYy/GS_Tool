/**
 * @file FibonacciHeap.h
 * @brief 斐波那契堆(Fibonacci Heap)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <limits>

/**
 * @class FibonacciHeap
 * @brief 斐波那契堆 — 支持decrease-key的优先队列
 *
 * 支持插入/提取最小值/decrease-key/删除/合并操作。
 * decrease-key和insert均为O(1)摊还复杂度。
 * 适用于Dijkstra最短路径、最小生成树等算法。
 */
class FibonacciHeap : public QObject
{
    Q_OBJECT

public:
    /** @brief 节点句柄 */
    struct Node {
        double key;            /**< 键值 */
        int data;              /**< 关联数据 */
        Node* parent;          /**< 父节点 */
        Node* child;           /**< 子节点 */
        Node* left;            /**< 左兄弟 */
        Node* right;           /**< 右兄弟 */
        int degree;            /**< 度数 */
        bool marked;           /**< 是否被标记 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;       /**< 总插入次数 */
        int totalExtracts = 0;      /**< 总提取次数 */
        int totalDecreaseKeys = 0;  /**< 总降键次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit FibonacciHeap(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~FibonacciHeap();

    /**
     * @brief 插入元素
     * @param key 键值
     * @param data 关联数据
     * @return 节点句柄
     */
    Node* insert(double key, int data = 0);

    /**
     * @brief 获取最小元素
     * @return 最小节点(不存在返回nullptr)
     */
    Node* min() const;

    /**
     * @brief 提取最小元素
     * @return 最小节点的数据
     */
    QPair<double, int> extractMin();

    /**
     * @brief 降低键值
     * @param node 目标节点
     * @param newKey 新键值(必须小于当前值)
     */
    void decreaseKey(Node* node, double newKey);

    /**
     * @brief 删除节点
     * @param node 目标节点
     */
    void remove(Node* node);

    /** @brief 堆是否为空 */
    bool isEmpty() const;

    /** @brief 获取元素数 */
    int size() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 提取完成信号 */
    void extracted(double key);

private:
    void consolidate();
    void link(Node* y, Node* x);
    void cut(Node* x, Node* y);
    void cascadingCut(Node* y);
    void destroyTree(Node* root);

    Node* m_min;
    int m_n;
    Stats m_stats;
    double m_timeSum;
};
