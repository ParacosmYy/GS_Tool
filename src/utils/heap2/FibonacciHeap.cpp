/**
 * @file FibonacciHeap.cpp
 * @brief 斐波那契堆 — 可合并优先队列
 */

#include "FibonacciHeap.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

FibonacciHeap::FibonacciHeap(QObject* parent)
    : QObject(parent)
    , m_minIndex(-1)
    , m_size(0)
    , m_timeSum(0.0)
{
}

FibonacciHeap::~FibonacciHeap() = default;

int FibonacciHeap::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node node;
    node.key = key;
    node.parent = -1;
    node.child = -1;
    node.left = m_nodes.size();
    node.right = m_nodes.size();
    node.degree = 0;
    node.mark = false;

    int idx = m_nodes.size();
    m_nodes.append(node);

    if (m_minIndex < 0) {
        m_minIndex = idx;
    } else {
        /* 插入到根链表 */
        m_nodes[idx].left = m_nodes[m_minIndex].left;
        m_nodes[idx].right = m_minIndex;
        m_nodes[m_nodes[idx].left].right = idx;
        m_nodes[m_minIndex].left = idx;

        if (key < m_nodes[m_minIndex].key) m_minIndex = idx;
    }

    m_stats.totalInserts++;
    m_size++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalDecreaseKeys + m_stats.totalMerges);

    return idx;
}

double FibonacciHeap::findMin() const
{
    if (m_minIndex < 0) return 0.0;
    return m_nodes[m_minIndex].key;
}

double FibonacciHeap::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    if (m_minIndex < 0) return 0.0;

    double minKey = m_nodes[m_minIndex].key;
    int z = m_minIndex;

    /* 将子节点加入根链表 */
    if (m_nodes[z].child >= 0) {
        int child = m_nodes[z].child;
        QVector<int> children;
        int start = child;
        do {
            children.append(child);
            m_nodes[child].parent = -1;
            child = m_nodes[child].right;
        } while (child != start);

        for (int c : children) {
            m_nodes[c].left = m_nodes[m_minIndex].left;
            m_nodes[c].right = m_minIndex;
            m_nodes[m_nodes[c].left].right = c;
            m_nodes[m_minIndex].left = c;
        }
    }

    /* 从根链表中移除z */
    if (m_nodes[z].left == z) {
        m_minIndex = -1;
    } else {
        m_nodes[m_nodes[z].left].right = m_nodes[z].right;
        m_nodes[m_nodes[z].right].left = m_nodes[z].left;
        m_minIndex = m_nodes[z].right;
        consolidate();
    }

    m_nodes[z].key = std::numeric_limits<double>::quiet_NaN();
    m_size--;

    m_stats.totalExtracts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalDecreaseKeys + m_stats.totalMerges);

    emit minExtracted(minKey);
    return minKey;
}

void FibonacciHeap::consolidate()
{
    int maxDeg = static_cast<int>(std::log2(m_size + 1)) + 2;
    QVector<int> A(maxDeg + 1, -1);

    /* 收集根链表节点 */
    QVector<int> roots;
    int start = m_minIndex;
    int current = start;
    do {
        roots.append(current);
        current = m_nodes[current].right;
    } while (current != start);

    for (int w : roots) {
        int x = w;
        int d = m_nodes[x].degree;

        while (d < A.size() && A[d] != -1) {
            int y = A[d];
            if (m_nodes[x].key > m_nodes[y].key) std::swap(x, y);
            link(y, x);
            A[d] = -1;
            d++;
        }
        if (d >= A.size()) A.resize(d + 1, -1);
        A[d] = x;
    }

    m_minIndex = -1;
    for (int i = 0; i < A.size(); ++i) {
        if (A[i] != -1) {
            if (m_minIndex < 0) {
                m_minIndex = A[i];
            } else if (m_nodes[A[i]].key < m_nodes[m_minIndex].key) {
                m_minIndex = A[i];
            }
        }
    }
}

void FibonacciHeap::link(int childIdx, int parentIdx)
{
    /* 从根链表移除childIdx */
    m_nodes[m_nodes[childIdx].left].right = m_nodes[childIdx].right;
    m_nodes[m_nodes[childIdx].right].left = m_nodes[childIdx].left;

    m_nodes[childIdx].parent = parentIdx;
    m_nodes[childIdx].mark = false;

    if (m_nodes[parentIdx].child < 0) {
        m_nodes[parentIdx].child = childIdx;
        m_nodes[childIdx].left = childIdx;
        m_nodes[childIdx].right = childIdx;
    } else {
        int first = m_nodes[parentIdx].child;
        m_nodes[childIdx].left = m_nodes[first].left;
        m_nodes[childIdx].right = first;
        m_nodes[m_nodes[first].left].right = childIdx;
        m_nodes[first].left = childIdx;
    }

    m_nodes[parentIdx].degree++;
}

void FibonacciHeap::decreaseKey(int handle, double newKey)
{
    if (handle < 0 || handle >= m_nodes.size()) return;

    QElapsedTimer timer;
    timer.start();

    double oldKey = m_nodes[handle].key;
    m_nodes[handle].key = newKey;

    int parent = m_nodes[handle].parent;
    if (parent >= 0 && newKey < m_nodes[parent].key) {
        cut(handle, parent);
        cascadingCut(parent);
    }

    if (newKey < m_nodes[m_minIndex].key) m_minIndex = handle;

    m_stats.totalDecreaseKeys++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalDecreaseKeys + m_stats.totalMerges);

    emit keyDecreased(handle, oldKey, newKey);
}

void FibonacciHeap::cut(int childIdx, int parentIdx)
{
    /* 从父节点的子链表中移除 */
    if (m_nodes[parentIdx].child == childIdx) {
        if (m_nodes[childIdx].right == childIdx) {
            m_nodes[parentIdx].child = -1;
        } else {
            m_nodes[parentIdx].child = m_nodes[childIdx].right;
        }
    }

    m_nodes[m_nodes[childIdx].left].right = m_nodes[childIdx].right;
    m_nodes[m_nodes[childIdx].right].left = m_nodes[childIdx].left;

    m_nodes[parentIdx].degree--;
    m_nodes[childIdx].parent = -1;
    m_nodes[childIdx].mark = false;

    /* 加入根链表 */
    m_nodes[childIdx].left = m_nodes[m_minIndex].left;
    m_nodes[childIdx].right = m_minIndex;
    m_nodes[m_nodes[m_minIndex].left].right = childIdx;
    m_nodes[m_minIndex].left = childIdx;
}

void FibonacciHeap::cascadingCut(int idx)
{
    int parent = m_nodes[idx].parent;
    if (parent >= 0) {
        if (!m_nodes[idx].mark) {
            m_nodes[idx].mark = true;
        } else {
            cut(idx, parent);
            cascadingCut(parent);
        }
    }
}

void FibonacciHeap::merge(FibonacciHeap& other)
{
    QElapsedTimer timer;
    timer.start();

    if (other.m_minIndex < 0) return;

    int offset = m_nodes.size();
    for (int i = 0; i < other.m_nodes.size(); ++i) {
        m_nodes.append(other.m_nodes[i]);
        if (m_nodes[offset + i].parent >= 0) m_nodes[offset + i].parent += offset;
        if (m_nodes[offset + i].child >= 0) m_nodes[offset + i].child += offset;
        if (m_nodes[offset + i].left >= 0) m_nodes[offset + i].left += offset;
        if (m_nodes[offset + i].right >= 0) m_nodes[offset + i].right += offset;
    }

    /* 合并根链表 */
    if (m_minIndex >= 0) {
        int myLeft = m_nodes[m_minIndex].left;
        int otherRoot = other.m_minIndex + offset;
        int otherLeft = m_nodes[otherRoot].left;

        m_nodes[myLeft].right = otherRoot;
        m_nodes[otherRoot].left = myLeft;
        m_nodes[otherLeft].right = m_minIndex;
        m_nodes[m_minIndex].left = otherLeft;

        if (other.m_nodes[other.m_minIndex].key < m_nodes[m_minIndex].key)
            m_minIndex = otherRoot;
    } else {
        m_minIndex = other.m_minIndex + offset;
    }

    m_size += other.m_size;
    other.m_minIndex = -1;
    other.m_size = 0;

    m_stats.totalMerges++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalExtracts + m_stats.totalDecreaseKeys + m_stats.totalMerges);
}

void FibonacciHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
