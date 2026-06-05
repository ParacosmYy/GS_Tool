#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BinaryHeap11 - 二叉堆第11代实现
 *
 * 提供最大堆/最小堆操作，支持动态建堆、插入、删除、
 * 堆排序及自定义比较器。
 */
class BinaryHeap11 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalHeapOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit BinaryHeap11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 从数组构建堆（线性时间建堆）
     * @param data 输入数据
     * @param isMinHeap true为最小堆，false为最大堆
     */
    void buildHeap(const QVector<double>& data, bool isMinHeap = true);

    /**
     * @brief 插入元素到堆中
     * @param value 待插入的值
     */
    void insert(double value);

    /**
     * @brief 弹出堆顶元素
     * @return 堆顶元素值
     */
    double extractTop();

    /**
     * @brief 堆排序，返回排序后的数组
     * @param data 待排序数据
     * @param ascending 是否升序排列
     * @return 排序后的数组
     */
    QVector<double> heapSort(const QVector<double>& data, bool ascending = true);

    /**
     * @brief 获取当前堆大小
     * @return 堆中元素数量
     */
    int size() const;

signals:
    void heapOperationCompleted(int currentSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
