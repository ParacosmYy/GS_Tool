/**
 * @file FibonacciHeap2.h
 * @brief Fibonacci堆 — 延迟合并/O(1)减键/合并/标记裁剪级联
 *
 * 功能: 实现Fibonacci堆优先队列，支持O(1)插入/合并/decrease-key，
 *       consolidate合并度数树，mark/cut级联操作，extract-min amortized O(log n)。
 *
 * 协作: GraphPartition(图分割) / DataSmoother(数据平滑)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <functional>

/**
 * @brief Fibonacci堆优先队列
 */
class FibonacciHeap2 : public QObject {
    Q_OBJECT

public:
    /** @brief 堆节点ID类型 */
    using NodeId = int;

    /** @brief 统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalExtractMin = 0;       ///< 累计extract-min次数
        quint64 totalDecreaseKey = 0;      ///< 累计decrease-key次数
        quint64 totalMerges = 0;           ///< 累计合并次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit FibonacciHeap2(QObject* parent = nullptr);

    /** @brief 插入元素 @param key 键值 @return 节点ID */
    NodeId insert(double key);

    /** @brief 查找最小键值 @return 最小键值(堆空返回NaN) */
    double findMin() const;

    /** @brief 提取最小元素 @return 最小键值(堆空返回NaN) */
    double extractMin();

    /** @brief 减小节点键值 @param node 节点ID @param newKey 新键值(必须<=旧值) */
    void decreaseKey(NodeId node, double newKey);

    /** @brief 合并另一个堆 @param other 另一个FibonacciHeap2(合并后other变空) */
    void merge(FibonacciHeap2& other);

    /** @brief 堆是否为空 @return 空=true */
    bool isEmpty() const;

    /** @brief 获取堆大小 @return 节点数 */
    int size() const;

    /** @brief 清空堆 */
    void clear();

    /** @brief 遍历所有键值(中序) @return 键值列表 */
    QList<double> toList() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成 @param node 节点ID @param key 键值 */
    void nodeInserted(NodeId node, double key);

    /** @brief 提取最小完成 @param key 最小键值 */
    void minExtracted(double key);

private:
    /** @brief 内部堆节点 */
    struct FHNode {
        double key = 0.0;           ///< 键值
        int degree = 0;             ///< 度数(子节点数)
        bool mark = false;          ///< 是否被标记(失去过子节点)
        NodeId parent = -1;         ///< 父节点ID
        NodeId child = -1;          ///< 第一个子节点ID
        NodeId left = -1;           ///< 左兄弟(循环链表)
        NodeId right = -1;          ///< 右兄弟(循环链表)
    };

    void consolidate();
    void linkRoots(NodeId child, NodeId parent);
    void cutNode(NodeId node);
    void cascadingCut(NodeId node);
    void addToRootList(NodeId node);
    void removeFromList(NodeId node);

    NodeId m_minNode;               ///< 最小节点ID
    int m_rootCount;                ///< 根链表中节点数
    int m_size;                     ///< 堆中总节点数
    int m_maxDegree;                ///< 最大度数

    QVector<FHNode> m_nodes;       ///< 节点池

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
