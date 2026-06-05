#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二叉最小堆工具类
 *
 * 提供最小堆数据结构操作，支持插入、提取最小值、
 * 降低键值等操作，可用于优先队列和排序。
 */
class BinaryHeap8 : public QObject {
    Q_OBJECT
public:
    /// 操作统计信息
    struct Stats {
        int totalOperations = 0;    ///< 总操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit BinaryHeap8(QObject* parent = nullptr);

    /** @brief 插入一个元素到堆中 */
    void insert(double value);

    /** @brief 提取并返回堆顶最小元素 */
    double extractMin();

    /** @brief 降低指定位置元素的键值 */
    void decreaseKey(int index, double newValue);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 提取完成信号，返回提取的最小值 */
    void extracted(double minValue);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_heap;
};
