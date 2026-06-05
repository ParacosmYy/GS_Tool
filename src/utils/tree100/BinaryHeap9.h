#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二叉最小堆
 *
 * 支持插入、提取最小值、键值减少和堆合并操作，
 * 用于优先队列和图算法的最短路径计算。
 */
class BinaryHeap9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalInserts = 0;        ///< 插入操作总数
        int totalExtracts = 0;       ///< 提取操作总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit BinaryHeap9(QObject* parent = nullptr);

    /** @brief 插入元素(键值, 数据) */
    void insert(double key, int data);
    /** @brief 提取最小元素 */
    QPair<double, int> extractMin();
    /** @brief 减少指定元素的键值 */
    void decreaseKey(int data, double newKey);
    /** @brief 合并另一个堆 */
    void merge(BinaryHeap9* other);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 提取完成，返回键值 */
    void extracted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QPair<double, int>> m_heap;
};
