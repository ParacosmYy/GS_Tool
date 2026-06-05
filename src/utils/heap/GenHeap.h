/**
 * @file Heap.h
 * @brief 泛型堆 — 最大堆/最小堆/优先队列
 */
#ifndef GENHEAP_H
#define GENHEAP_H

#include <QVector>
#include <QRandomGenerator>
#include <algorithm>

template<typename T>
class Heap {
public:
    struct Stats {
        quint64 totalInsertions = 0;
        quint64 totalExtractions = 0;
        quint64 totalHeapifies = 0;
    };

    enum class Type { MinHeap, MaxHeap };

    explicit Heap(Type type = Type::MinHeap) : m_type(type) {}

    void insert(const T& value)
    {
        m_data.append(value);
        siftUp(m_data.size() - 1);
        ++m_stats.totalInsertions;
    }

    T extractTop()
    {
        if (m_data.isEmpty()) return T{};
        T top = m_data[0];
        m_data[0] = m_data.last();
        m_data.removeLast();
        if (!m_data.isEmpty()) siftDown(0);
        ++m_stats.totalExtractions;
        return top;
    }

    const T& top() const { return m_data[0]; }
    bool isEmpty() const { return m_data.isEmpty(); }
    int size() const { return m_data.size(); }
    void clear() { m_data.clear(); }

    void heapify(const QVector<T>& data)
    {
        m_data = data;
        for (int i = m_data.size() / 2 - 1; i >= 0; --i) siftDown(i);
        ++m_stats.totalHeapifies;
    }

    QVector<T> toSortedVector()
    {
        QVector<T> result;
        Heap<T> copy = *this;
        while (!copy.isEmpty()) result.append(copy.extractTop());
        return result;
    }

    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats = Stats{}; }

private:
    bool compare(int a, int b) const
    {
        return (m_type == Type::MinHeap) ? m_data[a] < m_data[b] : m_data[a] > m_data[b];
    }

    void siftUp(int idx)
    {
        while (idx > 0) {
            int parent = (idx - 1) / 2;
            if (compare(idx, parent)) {
                std::swap(m_data[idx], m_data[parent]);
                idx = parent;
            } else break;
        }
    }

    void siftDown(int idx)
    {
        int n = m_data.size();
        while (true) {
            int best = idx;
            int left = 2 * idx + 1;
            int right = 2 * idx + 2;
            if (left < n && compare(left, best)) best = left;
            if (right < n && compare(right, best)) best = right;
            if (best != idx) { std::swap(m_data[idx], m_data[best]); idx = best; }
            else break;
        }
    }

    Type m_type;
    QVector<T> m_data;
    Stats m_stats;
};

#endif // GENHEAP_H
