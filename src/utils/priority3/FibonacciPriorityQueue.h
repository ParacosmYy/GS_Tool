/**
 * @file FibonacciPriorityQueue.h
 * @brief 斐波那契堆优先队列 — 支持decrease-key的优先队列
 *
 * 功能:
 *   - 斐波那契堆实现，支持O(1)的insert和decrease-key
 *   - extract-min为O(log n)摊还复杂度
 *   - 支持merge操作(O(1))
 *   - 统计插入/提取/合并/decrease-key次数
 *   - 适用于Dijkstra、Prim等需要频繁decrease-key的算法
 */

#pragma once

#include <QObject>
#include <QVariant>
#include <QVector>
#include <QMap>
#include <functional>
#include <cmath>

/**
 * @class FibonacciPriorityQueue
 * @brief 斐波那契堆优先队列 — 最小堆
 *
 * 斐波那契堆是一种可合并堆，支持高效decrease-key操作。
 * 通过维护双向循环链表的度标记树集合实现。
 * 适合图算法中需要频繁更新键值的场景。
 */
class FibonacciPriorityQueue : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;       /**< 总插入次数 */
        int totalExtractions = 0;      /**< 总提取次数 */
        int totalDecreaseKeys = 0;     /**< 总decrease-key次数 */
        int totalMerges = 0;           /**< 总合并次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit FibonacciPriorityQueue(QObject* parent = nullptr);

    /** @brief 析构函数 — 释放所有节点 */
    ~FibonacciPriorityQueue();

    /** @brief 禁止拷贝 */
    FibonacciPriorityQueue(const FibonacciPriorityQueue&) = delete;
    FibonacciPriorityQueue& operator=(const FibonacciPriorityQueue&) = delete;

    /**
     * @brief 插入元素
     * @param key 优先级键值
     * @param payload 关联数据
     * @return 节点句柄(用于decrease-key)
     */
    void* insert(double key, const QVariant& payload = QVariant());

    /**
     * @brief 提取最小元素
     * @param key 输出键值
     * @param payload 输出关联数据
     * @return 是否成功提取(队列非空)
     */
    bool extractMin(double* key = nullptr, QVariant* payload = nullptr);

    /**
     * @brief 降低节点键值
     * @param handle insert返回的句柄
     * @param newKey 新键值(必须小于当前键值)
     * @return 是否成功
     */
    bool decreaseKey(void* handle, double newKey);

    /**
     * @brief 合并另一个堆到当前堆
     * @param other 另一个斐波那契堆(合并后other为空)
     */
    void merge(FibonacciPriorityQueue& other);

    /**
     * @brief 查看最小元素键值(不提取)
     * @return 最小键值(空堆返回NaN)
     */
    double peekMinKey() const;

    /**
     * @brief 查看最小元素数据(不提取)
     * @return 最小元素的payload
     */
    QVariant peekMinPayload() const;

    /** @brief 堆是否为空 */
    bool isEmpty() const { return m_minNode == nullptr; }

    /** @brief 堆中元素数 */
    int size() const { return m_size; }

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void elementInserted(double key, int totalSize);

    /** @brief 提取完成信号 */
    void elementExtracted(double key, int remainingSize);

private:
    /** @brief 斐波那契堆节点 */
    struct FibNode {
        double key;              /**< 键值 */
        QVariant payload;       /**< 关联数据 */
        FibNode* parent = nullptr;  /**< 父节点 */
        FibNode* child = nullptr;   /**< 子节点 */
        FibNode* left = nullptr;    /**< 左兄弟 */
        FibNode* right = nullptr;   /**< 右兄弟 */
        int degree = 0;         /**< 度(子节点数) */
        bool marked = false;    /**< 是否被标记(失去过子节点) */

        FibNode(double k, const QVariant& p)
            : key(k), payload(p) {}
    };

    /** @brief 将节点加入根链表 */
    void addToRootList(FibNode* node);

    /** @brief 从链表中移除节点 */
    void removeFromList(FibNode* node);

    /** @brief 合并度相同的根树 */
    void consolidate();

    /** @brief 链接两个树(child挂到parent下) */
    void linkTrees(FibNode* child, FibNode* parent);

    /** @brief 级联切断 */
    void cascadingCut(FibNode* node);

    /** @brief 递归释放节点 */
    void freeNode(FibNode* node);

    /** @brief 释放所有节点 */
    void freeAll();

    FibNode* m_minNode;         /**< 最小节点指针 */
    int m_size;                 /**< 元素总数 */
    mutable Stats m_stats;      /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
};
