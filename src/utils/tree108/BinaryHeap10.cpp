#include "BinaryHeap10.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化二叉堆
 * @param type 堆类型(MinHeap或MaxHeap)
 * @param parent 父对象指针
 */
BinaryHeap10::BinaryHeap10(HeapType type, QObject* parent)
    : QObject(parent)
    , m_type(type)
{
}

/**
 * @brief 重置所有统计信息
 */
void BinaryHeap10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_heap.clear();
}

/**
 * @brief 比较函数，根据堆类型决定优先级
 * @param a 第一个元素的优先级
 * @param b 第二个元素的优先级
 * @return a是否应该优先于b
 */
bool BinaryHeap10::shouldSwap(double a, double b) const
{
    return (m_type == MinHeap) ? (a > b) : (a < b);
}

/**
 * @brief 上浮操作（插入后维护堆性质）
 * @param index 需要上浮的节点索引
 */
void BinaryHeap10::siftUp(int index)
{
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (shouldSwap(m_heap[parent].first, m_heap[index].first)) {
            std::swap(m_heap[parent], m_heap[index]);
            index = parent;
        } else {
            break;
        }
    }
}

/**
 * @brief 下沉操作（删除后维护堆性质）
 * @param index 需要下沉的节点索引
 */
void BinaryHeap10::siftDown(int index)
{
    const int n = m_heap.size();
    while (true) {
        int target = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < n && shouldSwap(m_heap[target].first, m_heap[left].first)) {
            target = left;
        }
        if (right < n && shouldSwap(m_heap[target].first, m_heap[right].first)) {
            target = right;
        }

        if (target != index) {
            std::swap(m_heap[index], m_heap[target]);
            index = target;
        } else {
            break;
        }
    }
}

/**
 * @brief 插入元素
 *
 * 将元素添加到堆末尾，然后上浮到正确位置。
 * 时间复杂度O(log n)。
 *
 * @param priority 优先级键值
 * @param data 关联数据
 */
void BinaryHeap10::insert(double priority, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    m_heap.append({priority, data});
    siftUp(m_heap.size() - 1);

    m_stats.currentSize = m_heap.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("insert"), m_heap.size());
}

/**
 * @brief 弹出堆顶元素
 *
 * 交换堆顶与末尾元素，移除末尾，然后从堆顶下沉。
 * 时间复杂度O(log n)。
 *
 * @return 堆顶元素的键值和数据
 */
QPair<double, QVariant> BinaryHeap10::extractTop()
{
    QElapsedTimer timer;
    timer.start();

    if (m_heap.isEmpty()) {
        return {0.0, QVariant()};
    }

    QPair<double, QVariant> top = m_heap[0];
    m_heap[0] = m_heap.last();
    m_heap.removeLast();

    if (!m_heap.isEmpty()) {
        siftDown(0);
    }

    m_stats.currentSize = m_heap.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("extractTop"), m_heap.size());
    return top;
}

/**
 * @brief 查看堆顶元素（不弹出）
 * @return 堆顶元素的键值和数据
 */
QPair<double, QVariant> BinaryHeap10::peekTop() const
{
    if (m_heap.isEmpty()) return {0.0, QVariant()};
    return m_heap[0];
}

/**
 * @brief 修改指定元素的优先级
 * @param index 元素索引
 * @param newPriority 新优先级
 * @return 修改是否成功
 */
bool BinaryHeap10::changePriority(int index, double newPriority)
{
    if (index < 0 || index >= m_heap.size()) return false;

    double oldPriority = m_heap[index].first;
    m_heap[index].first = newPriority;

    /* 根据新旧优先级的关系决定上浮或下沉 */
    if (shouldSwap(oldPriority, newPriority)) {
        siftUp(index);
    } else {
        siftDown(index);
    }

    m_stats.totalOperations++;
    emit operationCompleted(QStringLiteral("changePriority"), m_heap.size());
    return true;
}

/**
 * @brief 堆排序输出
 *
 * 反复弹出堆顶元素，得到有序序列。
 * 最小堆输出升序，最大堆输出降序。
 *
 * @return 排序后的键值对序列
 */
QVector<QPair<double, QVariant>> BinaryHeap10::heapSort()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, QVariant>> result;
    QVector<QPair<double, QVariant>> backup = m_heap;

    while (!m_heap.isEmpty()) {
        result.append(extractTop());
    }

    m_heap = backup; /* 恢复堆 */

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("heapSort"), result.size());
    return result;
}
