/**
 * @file IntervalHeap.cpp
 * @brief 区间堆(双端优先队列)实现
 *
 * 实现区间堆数据结构：每个节点存储一对(min,max)值，
 * 支持O(log n)的insert/deleteMin/deleteMax操作。
 *
 * 结构性质：
 * - 每个节点有两个槽位(left <= right)
 * - 父节点的left <= 所有子节点的left (min-heap性质)
 * - 父节点的right >= 所有子节点的right (max-heap性质)
 */

#include "utils/tree161/IntervalHeap.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
IntervalHeap::IntervalHeap(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入元素
 *
 * 如果最后一个节点只有一个值(half-full)，填入第二个槽位；
 * 否则创建新节点。插入后执行bubbleUp维护堆性质。
 */
void IntervalHeap::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    int nodeCount = m_heap.size();

    if (m_size % 2 == 0) {
        /* 需要新节点 */
        IntervalNode node;
        node.left = value;
        node.right = value;
        node.full = false;
        m_heap.append(node);
    } else {
        /* 最后一个节点只有一个值，填入第二个槽位 */
        IntervalNode& last = m_heap.last();
        if (value < last.left) {
            last.right = last.left;
            last.left = value;
        } else {
            last.right = value;
        }
        last.full = true;
    }
    m_size++;

    /* 向上修正 */
    bubbleUp(m_heap.size() - 1);

    /* 统计 */
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeletes
        + m_stats.totalMinQueries + m_stats.totalMaxQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementInserted(value, m_size);
}

/**
 * @brief 向上修正堆性质
 *
 * 沿父链向上比较，确保当前节点的left不小于父节点left中
 * 该有的值，且right不超过父节点right。
 */
void IntervalHeap::bubbleUp(int index)
{
    while (index > 0) {
        int p = parent(index);
        bool changed = false;

        /* min-heap性质：当前left >= 父left */
        if (m_heap[index].left < m_heap[p].left) {
            std::swap(m_heap[index].left, m_heap[p].left);
            changed = true;
        }

        /* max-heap性质：当前right <= 父right */
        if (m_heap[index].full && m_heap[p].full) {
            if (m_heap[index].right > m_heap[p].right) {
                std::swap(m_heap[index].right, m_heap[p].right);
                changed = true;
            }
        }

        /* 确保节点内部left <= right */
        if (m_heap[p].left > m_heap[p].right && m_heap[p].full) {
            std::swap(m_heap[p].left, m_heap[p].right);
        }
        if (m_heap[index].left > m_heap[index].right && m_heap[index].full) {
            std::swap(m_heap[index].left, m_heap[index].right);
        }

        if (!changed) break;
        index = p;
    }
}

/**
 * @brief 向下修正堆性质
 */
void IntervalHeap::trickleDown(int index)
{
    int nodeCount = m_heap.size();
    while (true) {
        int lc = leftChild(index);
        int rc = lc + 1;
        int smallest = index;
        int largest = index;

        /* 维护min-heap性质(对left值) */
        if (lc < nodeCount && m_heap[lc].left < m_heap[smallest].left)
            smallest = lc;
        if (rc < nodeCount && m_heap[rc].left < m_heap[smallest].left)
            smallest = rc;

        if (smallest != index) {
            std::swap(m_heap[index].left, m_heap[smallest].left);
            /* 确保内部有序 */
            if (m_heap[smallest].full && m_heap[smallest].left > m_heap[smallest].right)
                std::swap(m_heap[smallest].left, m_heap[smallest].right);
            index = smallest;
        }

        /* 维护max-heap性质(对right值) */
        if (m_heap[lc < nodeCount ? lc : index].full) {
            if (lc < nodeCount && m_heap[lc].full && m_heap[lc].right > m_heap[largest].right)
                largest = lc;
            if (rc < nodeCount && m_heap[rc].full && m_heap[rc].right > m_heap[largest].right)
                largest = rc;

            if (largest != index) {
                std::swap(m_heap[index].right, m_heap[largest].right);
                if (m_heap[largest].left > m_heap[largest].right && m_heap[largest].full)
                    std::swap(m_heap[largest].left, m_heap[largest].right);
            }
        }

        if (smallest == index && largest == index) break;
    }
}

/**
 * @brief 删除并返回最小值
 */
double IntervalHeap::deleteMin()
{
    QElapsedTimer timer;
    timer.start();

    if (m_size == 0) return 0.0;

    double minValue = m_heap[0].left;

    if (m_size == 1) {
        m_heap.clear();
        m_size = 0;
    } else if (m_size == 2) {
        /* 只剩一个节点，移除left，保留right */
        m_heap[0].left = m_heap[0].right;
        m_heap[0].full = false;
        m_size = 1;
    } else {
        /* 用最后一个节点的值替换根的left */
        if (m_size % 2 == 0) {
            /* 最后一个节点full */
            m_heap[0].left = m_heap.last().left;
            /* 移除最后节点 */
            m_heap.removeLast();
        } else {
            /* 最后一个节点half-full */
            m_heap[0].left = m_heap.last().right;
            m_heap.last().full = true;
            if (m_heap.last().left > m_heap.last().right)
                std::swap(m_heap.last().left, m_heap.last().right);
        }
        m_size--;

        /* 确保根节点内部有序 */
        if (m_heap[0].full && m_heap[0].left > m_heap[0].right)
            std::swap(m_heap[0].left, m_heap[0].right);

        trickleDown(0);
    }

    m_stats.totalDeletes++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeletes
        + m_stats.totalMinQueries + m_stats.totalMaxQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementDeleted(minValue, true);
    return minValue;
}

/**
 * @brief 删除并返回最大值
 */
double IntervalHeap::deleteMax()
{
    QElapsedTimer timer;
    timer.start();

    if (m_size == 0) return 0.0;

    int maxIdx = 0;

    if (m_size == 1) {
        double val = m_heap[0].left;
        m_heap.clear();
        m_size = 0;

        m_stats.totalDeletes++;
        m_timeSum += timer.elapsed();
        emit elementDeleted(val, false);
        return val;
    }

    /* 最大值在根或根的子节点的right中 */
    double maxValue = m_heap[0].right;
    maxIdx = 0;
    int lc = leftChild(0);
    int rc = lc + 1;
    int nodeCount = m_heap.size();

    if (lc < nodeCount && m_heap[lc].full && m_heap[lc].right > maxValue) {
        maxValue = m_heap[lc].right;
        maxIdx = lc;
    }
    if (rc < nodeCount && m_heap[rc].full && m_heap[rc].right > maxValue) {
        maxValue = m_heap[rc].right;
        maxIdx = rc;
    }

    /* 移除maxIdx处的right */
    if (m_size == 2) {
        m_heap[0].full = false;
        m_size = 1;
    } else {
        /* 用最后节点的值替换 */
        if (m_size % 2 == 0) {
            m_heap[maxIdx].right = m_heap.last().right;
            m_heap.removeLast();
        } else {
            m_heap[maxIdx].right = m_heap.last().left;
            m_heap.last().full = false;
        }
        m_size--;

        if (m_heap[maxIdx].full && m_heap[maxIdx].left > m_heap[maxIdx].right)
            std::swap(m_heap[maxIdx].left, m_heap[maxIdx].right);

        trickleDown(maxIdx);
    }

    m_stats.totalDeletes++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeletes
        + m_stats.totalMinQueries + m_stats.totalMaxQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementDeleted(maxValue, false);
    return maxValue;
}

/**
 * @brief 获取最小值(不删除)
 */
double IntervalHeap::getMin() const
{
    if (m_size == 0) return 0.0;
    return m_heap[0].left;
}

/**
 * @brief 获取最大值(不删除)
 */
double IntervalHeap::getMax() const
{
    if (m_size == 0) return 0.0;
    if (m_size == 1) return m_heap[0].left;

    double maxVal = m_heap[0].full ? m_heap[0].right : m_heap[0].left;
    int lc = leftChild(0);
    if (lc < m_heap.size() && m_heap[lc].full)
        maxVal = qMax(maxVal, m_heap[lc].right);
    int rc = lc + 1;
    if (rc < m_heap.size() && m_heap[rc].full)
        maxVal = qMax(maxVal, m_heap[rc].right);

    return maxVal;
}

void IntervalHeap::clear()
{
    m_heap.clear();
    m_size = 0;
}

void IntervalHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
