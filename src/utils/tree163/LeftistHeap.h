/**
 * @file LeftistHeap.h
 * @brief 左偏堆 — Leftist Heap with Rank-Based Merge and Decrease-Key
 *
 * 功能: 实现左偏堆(Leftist Heap)，基于秩(rank)的合并优先队列。
 *       支持O(log n)合并、插入、删除最小值和降键操作。
 *       适用于Dijkstra最短路径等需要decrease-key的场景。
 *
 * 协作: BinomialHeap(二项堆) / FibonacciHeap(斐波那契堆) / PairingHeap(配对堆)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QHash>

/**
 * @brief 左偏堆(Leftist Heap)
 */
class LeftistHeap : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalDeleteMins = 0;        ///< 累计删除最小次数
        quint64 totalMerges = 0;            ///< 累计合并次数
        quint64 totalDecreaseKeys = 0;      ///< 累计降键次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        int currentSize = 0;                ///< 当前元素数
        int height = 0;                     ///< 当前堆高度
    };

    explicit LeftistHeap(QObject* parent = nullptr);
    ~LeftistHeap() override;

    /**
     * @brief 插入键值对
     * @param key 优先级键
     * @param value 关联值
     * @return 元素ID(用于decreaseKey/remove)
     */
    int insert(double key, double value);

    /**
     * @brief 获取最小键值
     * @param key 输出最小键
     * @param value 输出关联值
     * @return 堆是否非空
     */
    bool findMin(double& key, double& value) const;

    /**
     * @brief 删除并返回最小元素
     * @param key 输出最小键
     * @param value 输出关联值
     * @return 是否成功
     */
    bool deleteMin(double& key, double& value);

    /**
     * @brief 降低指定元素的键值
     * @param id 元素ID(insert返回)
     * @param newKey 新键值(必须小于当前键)
     * @return 是否成功
     */
    bool decreaseKey(int id, double newKey);

    /**
     * @brief 合并另一个左偏堆到当前堆
     * @param other 另一个堆(合并后变为空)
     */
    void merge(LeftistHeap& other);

    /** @brief 堆是否为空 */
    bool isEmpty() const;

    /** @brief 元素数量 */
    int size() const;

    /** @brief 清空堆 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成 @param key 键 @param size 当前大小 */
    void insertCompleted(double key, int size);
    /** @brief 删除最小完成 @param key 键 */
    void deleteMinCompleted(double key);

private:
    /** @brief 堆节点 */
    struct Node {
        double key;            ///< 优先级键
        double value;          ///< 关联值
        int rank;              ///< 零路径长度(null path length)
        int id;                ///< 唯一标识
        Node* left;            ///< 左子树
        Node* right;           ///< 右子树
    };

    /** @brief 合并两棵子树(核心操作) */
    Node* mergeNodes(Node* a, Node* b);

    /** @brief 计算节点秩 */
    static int nodeRank(Node* node);

    /** @brief 递归释放 */
    void deleteTree(Node* node);

    /** @brief 更新高度统计 */
    void updateHeight();

    /** @brief 移除指定节点(用于decreaseKey) */
    Node* removeNode(Node* root, int id);

    Node* m_root = nullptr;
    int m_nextId = 0;
    QHash<int, Node*> m_nodeMap;    ///< id -> 节点映射

    Stats m_stats;
    double m_timeSum = 0.0;
};
