#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二叉堆实现 (10种操作)
 *
 * 提供最小堆/最大堆数据结构，支持优先队列操作和堆排序。
 */
class BinaryHeap10 : public QObject {
    Q_OBJECT
public:
    /// 堆类型
    enum HeapType { MinHeap = 0, MaxHeap = 1 };

    /// 统计信息结构
    struct Stats {
        int totalOperations = 0;        ///< 总操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int currentSize = 0;            ///< 当前堆大小
    };

    explicit BinaryHeap10(HeapType type = MinHeap, QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 插入元素
     * @param priority 优先级键值
     * @param data 关联数据
     */
    void insert(double priority, const QVariant& data);

    /**
     * @brief 弹出堆顶元素
     * @return 堆顶元素的键值和数据
     */
    QPair<double, QVariant> extractTop();

    /**
     * @brief 查看堆顶元素（不弹出）
     * @return 堆顶元素的键值和数据
     */
    QPair<double, QVariant> peekTop() const;

    /**
     * @brief 修改指定元素的优先级
     * @param index 元素索引
     * @param newPriority 新优先级
     * @return 修改是否成功
     */
    bool changePriority(int index, double newPriority);

    /**
     * @brief 堆排序输出
     * @return 排序后的键值对序列
     */
    QVector<QPair<double, QVariant>> heapSort();

signals:
    /// 堆操作完成信号
    void operationCompleted(const QString& operation, int newSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    HeapType m_type;
    QVector<QPair<double, QVariant>> m_heap;
};
