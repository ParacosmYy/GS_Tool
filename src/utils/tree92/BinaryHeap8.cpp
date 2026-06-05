#include "BinaryHeap8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化二叉最小堆
 * @param parent 父对象指针
 */
BinaryHeap8::BinaryHeap8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 上浮操作，维护堆性质
 * @param index 需要上浮的节点索引
 */
void siftUp(QVector<double>& heap, int index)
{
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap[index] < heap[parent]) {
            std::swap(heap[index], heap[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

/**
 * @brief 下沉操作，维护堆性质
 * @param heap 堆数组
 * @param index 需要下沉的节点索引
 * @param size 堆的有效大小
 */
void siftDown(QVector<double>& heap, int index, int size)
{
    while (true) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < size && heap[left] < heap[smallest]) smallest = left;
        if (right < size && heap[right] < heap[smallest]) smallest = right;

        if (smallest != index) {
            std::swap(heap[index], heap[smallest]);
            index = smallest;
        } else {
            break;
        }
    }
}

/**
 * @brief 插入一个元素到堆中
 *
 * 将元素添加到末尾后执行上浮操作恢复堆性质。
 * 时间复杂度 O(log n)。
 *
 * @param value 待插入的值
 */
void BinaryHeap8::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    m_heap.append(value);
    ::siftUp(m_heap, m_heap.size() - 1);

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 提取并返回堆顶最小元素
 *
 * 交换堆顶与末尾元素，移除末尾后对堆顶执行下沉操作。
 * 时间复杂度 O(log n)。
 *
 * @return 堆顶最小值
 */
double BinaryHeap8::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    double minValue = 0.0;
    if (m_heap.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        return minValue;
    }

    minValue = m_heap[0];
    int lastIdx = m_heap.size() - 1;
    m_heap[0] = m_heap[lastIdx];
    m_heap.removeLast();

    if (!m_heap.isEmpty()) {
        ::siftDown(m_heap, 0, m_heap.size());
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit extracted(minValue);
    return minValue;
}

/**
 * @brief 降低指定位置元素的键值
 *
 * 将指定位置的值减小后执行上浮操作恢复堆性质。
 *
 * @param index 元素位置索引
 * @param newValue 新的键值(必须小于等于当前值)
 */
void BinaryHeap8::decreaseKey(int index, double newValue)
{
    QElapsedTimer timer;
    timer.start();

    if (index < 0 || index >= m_heap.size()) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        return;
    }

    if (newValue < m_heap[index]) {
        m_heap[index] = newValue;
        ::siftUp(m_heap, index);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 重置统计数据
 */
void BinaryHeap8::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
    m_heap.clear();
}
