/**
 * @file CircularPriorityQueue.cpp
 * @brief 环形优先队列实现
 */

#include "utils/circqueue/CircularPriorityQueue.h"

#include <QElapsedTimer>
#include <algorithm>

CprioQueue::CprioQueue(int capacity, QObject* parent)
    : QObject(parent), m_capacity(qMax(16, capacity)), m_size(0),
      m_timeSum(0.0)
{
    m_heap.resize(m_capacity);
}

bool CprioQueue::push(double priority, double value)
{
    if (m_size >= m_capacity) {
        m_stats.totalOverflows++;
        emit overflowDiscarded(priority);
        return false;
    }

    m_heap[m_size] = {priority, value};
    heapifyUp(m_size);
    m_size++;

    m_stats.totalPushes++;
    return true;
}

bool CprioQueue::pop(double& priority, double& value)
{
    if (m_size == 0) return false;

    priority = m_heap[0].first;
    value = m_heap[0].second;

    m_heap[0] = m_heap[m_size - 1];
    m_size--;
    if (m_size > 0) heapifyDown(0);

    m_stats.totalPops++;
    emit elementPopped(priority);
    return true;
}

QPair<double, double> CprioQueue::peek() const
{
    if (m_size == 0) return {0.0, 0.0};
    return m_heap[0];
}

QVector<QPair<double, double>> CprioQueue::popBatch(int maxCount)
{
    QVector<QPair<double, double>> result;
    result.reserve(qMin(maxCount, m_size));

    double p, v;
    while (result.size() < maxCount && pop(p, v)) {
        result.append({p, v});
    }
    return result;
}

int CprioQueue::size() const { return m_size; }
int CprioQueue::capacity() const { return m_capacity; }
bool CprioQueue::isEmpty() const { return m_size == 0; }
bool CprioQueue::isFull() const { return m_size >= m_capacity; }

void CprioQueue::clear()
{
    m_size = 0;
}

void CprioQueue::heapifyUp(int idx)
{
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (m_heap[idx].first > m_heap[parent].first) {
            std::swap(m_heap[idx], m_heap[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void CprioQueue::heapifyDown(int idx)
{
    while (true) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int largest = idx;

        if (left < m_size && m_heap[left].first > m_heap[largest].first)
            largest = left;
        if (right < m_size && m_heap[right].first > m_heap[largest].first)
            largest = right;

        if (largest != idx) {
            std::swap(m_heap[idx], m_heap[largest]);
            idx = largest;
        } else {
            break;
        }
    }
}

void CprioQueue::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
