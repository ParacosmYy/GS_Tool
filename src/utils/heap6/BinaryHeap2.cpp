/**
 * @file BinaryHeap2.cpp
 * @brief 增强二叉堆实现
 */

#include "utils/heap6/BinaryHeap2.h"

#include <QElapsedTimer>
#include <algorithm>

BinaryHeap2::BinaryHeap2(QObject* parent)
    : QObject(parent)
{
}

BinaryHeap2::Handle BinaryHeap2::insert(double key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    int handle = m_nextHandle++;
    int nodeIdx = m_nodes.size();
    m_nodes.append({key, value, m_heap.size()});
    m_heap.append(nodeIdx);

    siftUp(m_heap.size() - 1);

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges) : 0.0;

    emit elementInserted(handle);
    return handle;
}

QVector<BinaryHeap2::Handle> BinaryHeap2::buildHeap(
    const QVector<QPair<double, QVariant>>& items)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Handle> handles;
    handles.reserve(items.size());

    /* 直接放入数组 */
    for (const auto& item : items) {
        int handle = m_nextHandle++;
        m_nodes.append({item.first, item.second, m_heap.size()});
        m_heap.append(m_nodes.size() - 1);
        handles.append(handle);
    }

    /* 从最后一个非叶节点开始下沉建堆 */
    int n = m_heap.size();
    for (int i = n / 2 - 1; i >= 0; --i) {
        siftDown(i);
    }

    m_stats.totalInserts += items.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges) : 0.0;

    return handles;
}

QPair<double, QVariant> BinaryHeap2::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    if (m_heap.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {0.0, QVariant()};
    }

    int minNodeIdx = m_heap[0];
    auto result = qMakePair(m_nodes[minNodeIdx].key,
                            m_nodes[minNodeIdx].value);

    /* 将最后一个元素移到堆顶 */
    int lastIdx = m_heap.size() - 1;
    if (lastIdx > 0) {
        m_heap[0] = m_heap[lastIdx];
        m_nodes[m_heap[0]].heapIndex = 0;
    }
    m_heap.removeLast();

    if (!m_heap.isEmpty()) {
        siftDown(0);
    }

    m_stats.totalExtracts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges) : 0.0;

    emit elementExtracted(result.first);
    return result;
}

void BinaryHeap2::decreaseKey(Handle handle, double newKey)
{
    if (handle < 0 || handle >= m_nodes.size()) return;
    Node& node = m_nodes[handle];
    if (newKey >= node.key || node.heapIndex < 0) return;

    node.key = newKey;
    siftUp(node.heapIndex);
}

void BinaryHeap2::increaseKey(Handle handle, double newKey)
{
    if (handle < 0 || handle >= m_nodes.size()) return;
    Node& node = m_nodes[handle];
    if (newKey <= node.key || node.heapIndex < 0) return;

    node.key = newKey;
    siftDown(node.heapIndex);
}

void BinaryHeap2::merge(BinaryHeap2& other)
{
    QElapsedTimer timer;
    timer.start();

    int count = other.m_heap.size();

    /* 将other的所有节点迁移到当前堆 */
    for (int i = 0; i < other.m_nodes.size(); ++i) {
        other.m_nodes[i].heapIndex = m_heap.size();
        m_nodes.append(other.m_nodes[i]);
        m_heap.append(m_nodes.size() - 1);
    }

    /* 重新建堆 */
    int n = m_heap.size();
    for (int i = n / 2 - 1; i >= 0; --i) {
        siftDown(i);
    }

    /* 清空other */
    other.m_nodes.clear();
    other.m_heap.clear();

    m_stats.totalMerges++;
    m_stats.totalInserts += count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalMerges) : 0.0;

    emit mergeCompleted(count);
}

QPair<double, QVariant> BinaryHeap2::peek() const
{
    if (m_heap.isEmpty()) return {0.0, QVariant()};
    int idx = m_heap[0];
    return {m_nodes[idx].key, m_nodes[idx].value};
}

bool BinaryHeap2::isEmpty() const { return m_heap.isEmpty(); }
int BinaryHeap2::size() const { return m_heap.size(); }

void BinaryHeap2::siftUp(int index)
{
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (m_nodes[m_heap[index]].key >= m_nodes[m_heap[parent]].key)
            break;
        swapNodes(index, parent);
        index = parent;
    }
}

void BinaryHeap2::siftDown(int index)
{
    int n = m_heap.size();
    while (true) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < n && m_nodes[m_heap[left]].key < m_nodes[m_heap[smallest]].key)
            smallest = left;
        if (right < n && m_nodes[m_heap[right]].key < m_nodes[m_heap[smallest]].key)
            smallest = right;

        if (smallest == index) break;
        swapNodes(index, smallest);
        index = smallest;
    }
}

void BinaryHeap2::swapNodes(int i, int j)
{
    m_nodes[m_heap[i]].heapIndex = j;
    m_nodes[m_heap[j]].heapIndex = i;
    std::swap(m_heap[i], m_heap[j]);
}

BinaryHeap2::Stats BinaryHeap2::stats() const { return m_stats; }

void BinaryHeap2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
