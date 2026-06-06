/**
 * @file PairingHeap.h
 * @brief 配对堆(两趟合并) — Pairing Heap with Two-Pass Merge and Amortized O(log n) Delete-Min
 *
 * 功能: 实现配对堆(Pairing Heap)，支持O(1)插入和find-min、
 *       amortized O(log n) delete-min(两趟合并)。
 *       支持decrease-key和合并操作。适用于Dijkstra等贪心算法。
 *
 * 协作: LeftistHeap(左偏堆) / BinomialHeap(二项堆) / FibonacciHeap(斐波那契堆)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

/**
 * @brief 配对堆(Pairing Heap)
 */
class PairingHeap : public QObject {
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
    };

    explicit PairingHeap(QObject* parent = nullptr);
    ~PairingHeap() override;

    /**
     * @brief 插入键值对
     * @param key 优先级键
     * @param value 关联值
     * @return 元素ID(用于decreaseKey)
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
     * @param id 元素ID
     * @param newKey 新键值(必须小于当前键)
     * @return 是否成功
     */
    bool decreaseKey(int id, double newKey);

    /**
     * @brief 合并另一个配对堆
     * @param other 另一个堆(合并后变为空)
     */
    void merge(PairingHeap& other);

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
        int id;                ///< 唯一标识
        Node* child;           ///< 最左子节点
        Node* sibling;         ///< 右兄弟节点
    };

    /** @brief 两趟合并: 先从左到右两两配对，再从右到左累积合并 */
    Node* twoPassMerge(Node* firstChild);

    /** @brief 合并两棵子树 */
    static Node* mergeNodes(Node* a, Node* b);

    /** @brief 递归释放 */
    void deleteTree(Node* node);

    Node* m_root = nullptr;
    int m_nextId = 0;
    QHash<int, Node*> m_nodeMap;    ///< id -> 节点映射

    Stats m_stats;
    double m_timeSum = 0.0;
};
