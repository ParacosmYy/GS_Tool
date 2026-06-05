/**
 * @file SkewHeap.h
 * @brief 斜堆合并优先队列
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 斜堆(Skew Heap)
 *
 * 自调整的可合并堆,所有操作均摊O(log n)。
 * 支持insert/extractMin/merge操作。
 */
class SkewHeap : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;       ///< 插入次数
        int totalExtracts = 0;      ///< 提取次数
        int totalMerges = 0;        ///< 合并次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit SkewHeap(QObject* parent = nullptr);
    ~SkewHeap();

    /**
     * @brief 插入元素
     * @param key 键值
     * @param value 附加数据
     */
    void insert(double key, int value = 0);

    /**
     * @brief 提取最小元素
     * @param key 输出键值
     * @param value 输出附加数据
     * @return 是否成功
     */
    bool extractMin(double& key, int& value);

    /**
     * @brief 查看最小元素
     */
    double peekMin() const;

    /**
     * @brief 合并另一个斜堆
     */
    void merge(SkewHeap& other);

    /**
     * @brief 当前大小
     */
    int size() const;

    /**
     * @brief 是否为空
     */
    bool isEmpty() const;

    /**
     * @brief 清空堆
     */
    void clear();

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 合并完成信号 */
    void mergeCompleted(int newSize);

private:
    struct Node {
        double key;
        int value;
        Node* left;
        Node* right;
        Node(double k, int v) : key(k), value(v), left(nullptr), right(nullptr) {}
    };

    Node* m_root;
    int m_size;
    Stats m_stats;
    double m_timeSum = 0.0;

    Node* mergeNodes(Node* a, Node* b);
    void destroyTree(Node* node);
};
