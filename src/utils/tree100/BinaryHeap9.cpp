#include "BinaryHeap9.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file BinaryHeap9.cpp
 * @brief 二叉最小堆实现
 *
 * 基于数组实现的最小堆，支持O(logN)的插入、提取最小值、
 * 键值减少和堆合并操作。
 */

/**
 * @brief 构造函数，初始化空堆
 * @param parent 父QObject对象指针
 */
BinaryHeap9::BinaryHeap9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入元素到堆中
 *
 * 将新元素添加到数组末尾，然后通过上浮(sift up)操作
 * 恢复堆性质。
 *
 * @param key 键值(用于排序)
 * @param data 关联的数据
 */
void BinaryHeap9::insert(double key, int data)
{
    QElapsedTimer timer;
    timer.start();

    m_heap.append(qMakePair(key, data));
    int idx = m_heap.size() - 1;

    // 上浮操作: 将新元素向上移动到正确位置
    while (idx > 0) {
        const int parent = (idx - 1) / 2;
        if (m_heap[idx].first < m_heap[parent].first) {
            std::swap(m_heap[idx], m_heap[parent]);
            idx = parent;
        } else {
            break;
        }
    }

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalInserts + m_stats.totalExtracts;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
}

/**
 * @brief 提取最小元素
 *
 * 返回堆顶元素(最小值)，用数组最后一个元素替换堆顶，
 * 然后通过下沉(sift down)恢复堆性质。
 *
 * @return 键值最小的元素(key, data)
 */
QPair<double, int> BinaryHeap9::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    if (m_heap.isEmpty()) {
        m_timeSum += timer.elapsed();
        return qMakePair(0.0, -1);
    }

    QPair<double, int> minItem = m_heap[0];
    m_heap[0] = m_heap.last();
    m_heap.removeLast();

    // 下沉操作: 将堆顶元素向下移动到正确位置
    int idx = 0;
    const int n = m_heap.size();
    while (true) {
        int smallest = idx;
        const int left = 2 * idx + 1;
        const int right = 2 * idx + 2;

        if (left < n && m_heap[left].first < m_heap[smallest].first) {
            smallest = left;
        }
        if (right < n && m_heap[right].first < m_heap[smallest].first) {
            smallest = right;
        }

        if (smallest != idx) {
            std::swap(m_heap[idx], m_heap[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }

    m_stats.totalExtracts++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalInserts + m_stats.totalExtracts;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit extracted(minItem.first);
    return minItem;
}

/**
 * @brief 减少指定元素的键值
 *
 * 找到指定数据的元素，将其键值更新为newKey，
 * 然后通过上浮操作恢复堆性质。
 *
 * @param data 要更新的元素数据标识
 * @param newKey 新的键值(必须小于原键值)
 */
void BinaryHeap9::decreaseKey(int data, double newKey)
{
    QElapsedTimer timer;
    timer.start();

    // 线性搜索目标元素
    for (int i = 0; i < m_heap.size(); ++i) {
        if (m_heap[i].second == data) {
            m_heap[i].first = newKey;

            // 上浮恢复堆性质
            int idx = i;
            while (idx > 0) {
                const int parent = (idx - 1) / 2;
                if (m_heap[idx].first < m_heap[parent].first) {
                    std::swap(m_heap[idx], m_heap[parent]);
                    idx = parent;
                } else {
                    break;
                }
            }
            break;
        }
    }

    m_timeSum += timer.elapsed();
}

/**
 * @brief 合并另一个堆
 *
 * 将另一个堆的所有元素插入到当前堆中，
 * 然后重新建堆(heapify)。
 *
 * @param other 待合并的堆指针
 */
void BinaryHeap9::merge(BinaryHeap9* other)
{
    if (!other) return;

    QElapsedTimer timer;
    timer.start();

    // 将另一个堆的元素全部加入
    m_heap.append(other->m_heap);

    // 自底向上建堆(Floyd方法)
    const int n = m_heap.size();
    for (int i = n / 2 - 1; i >= 0; --i) {
        int idx = i;
        while (true) {
            int smallest = idx;
            const int left = 2 * idx + 1;
            const int right = 2 * idx + 2;

            if (left < n && m_heap[left].first < m_heap[smallest].first) {
                smallest = left;
            }
            if (right < n && m_heap[right].first < m_heap[smallest].first) {
                smallest = right;
            }

            if (smallest != idx) {
                std::swap(m_heap[idx], m_heap[smallest]);
                idx = smallest;
            } else {
                break;
            }
        }
    }

    m_timeSum += timer.elapsed();
}

/**
 * @brief 重置所有统计信息
 */
void BinaryHeap9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_heap.clear();
}
