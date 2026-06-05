/**
 * @file BinomialQueue.cpp
 * @brief 二项队列实现 — 二项树森林+合并链接+惰性删除+降键
 */

#include "utils/tree25/BinomialQueue.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
BinomialQueue::BinomialQueue(QObject* parent)
    : QObject(parent)
    , m_nextHandle(1)
{
}

/**
 * @brief 插入元素
 * @param value 元素值
 * @return 元素句柄(用于后续decrease-key/remove)
 */
int BinomialQueue::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    int nodeIdx = createNode(value);
    int handle = m_nodes[nodeIdx].handle;

    /* 创建单节点二项树并合并到森林 */
    QList<int> newRoots;
    newRoots.append(nodeIdx);

    /* 合并两个森林: 原有根列表 + 新节点 */
    QList<int> combined = m_roots;
    combined.append(nodeIdx);

    /* 按度数排序并合并相同度数的树 */
    std::sort(combined.begin(), combined.end(),
        [this](int a, int b) { return m_nodes[a].degree < m_nodes[b].degree; });

    m_roots.clear();
    int i = 0;
    while (i < combined.size()) {
        if (i + 1 < combined.size() && m_nodes[combined[i]].degree == m_nodes[combined[i+1]].degree) {
            /* 合并两棵相同度数的树 */
            int merged = mergeTrees(combined[i], combined[i+1]);
            combined[i+1] = merged;
            ++i;
        } else {
            m_roots.append(combined[i]);
            ++i;
        }
    }

    ++m_stats.totalInsertions;
    ++m_stats.currentSize;
    if (m_stats.currentSize > m_stats.peakSize) {
        m_stats.peakSize = m_stats.currentSize;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
            + m_stats.totalMerges + m_stats.totalDecreaseKeys);

    emit elementInserted(handle, value);
    return handle;
}

/**
 * @brief 查找最小元素
 * @return 最小元素结果
 */
BinomialQueue::OpResult BinomialQueue::findMin() const
{
    OpResult result;
    if (m_roots.isEmpty()) return result;

    int minRoot = findMinRoot();
    if (minRoot >= 0) {
        result.success = true;
        result.value = m_nodes[minRoot].value;
        result.handle = m_nodes[minRoot].handle;
    }
    return result;
}

/**
 * @brief 删除最小元素
 * @return 被删除元素结果
 */
BinomialQueue::OpResult BinomialQueue::deleteMin()
{
    QElapsedTimer timer;
    timer.start();

    OpResult result;
    if (m_roots.isEmpty()) return result;

    /* 找最小根 */
    int minIdx = -1;
    int minPos = -1;
    for (int i = 0; i < m_roots.size(); ++i) {
        if (minIdx < 0 || m_nodes[m_roots[i]].value < m_nodes[m_roots[minIdx]].value) {
            minIdx = m_roots[i];
            minPos = i;
        }
    }

    result.success = true;
    result.value = m_nodes[minIdx].value;
    result.handle = m_nodes[minIdx].handle;

    /* 移除最小根 */
    m_roots.removeAt(minPos);

    /* 收集最小根的子树(反转兄弟链以恢复度数顺序) */
    QList<int> childRoots;
    int child = m_nodes[minIdx].child;
    while (child >= 0) {
        int nextSib = m_nodes[child].sibling;
        m_nodes[child].parent = -1;
        m_nodes[child].sibling = -1;
        childRoots.prepend(child);
        child = nextSib;
    }

    /* 合并子树森林和原森林 */
    QList<int> combined = m_roots;
    combined.append(childRoots);

    std::sort(combined.begin(), combined.end(),
        [this](int a, int b) { return m_nodes[a].degree < m_nodes[b].degree; });

    m_roots.clear();
    int i = 0;
    while (i < combined.size()) {
        if (i + 1 < combined.size() && m_nodes[combined[i]].degree == m_nodes[combined[i+1]].degree) {
            int merged = mergeTrees(combined[i], combined[i+1]);
            combined[i+1] = merged;
            ++i;
        } else {
            m_roots.append(combined[i]);
            ++i;
        }
    }

    ++m_stats.totalDeletions;
    --m_stats.currentSize;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
            + m_stats.totalMerges + m_stats.totalDecreaseKeys);

    emit minDeleted(result.value);
    return result;
}

/**
 * @brief 合并另一个二项队列
 * @param other 另一个二项队列(合并后变空)
 */
void BinomialQueue::meld(BinomialQueue& other)
{
    QElapsedTimer timer;
    timer.start();

    /* 将other的节点池和根合并到this */
    QList<int> combined = m_roots;
    combined.append(other.m_roots);

    /* 合并句柄映射 */
    for (const auto& node : other.m_nodes) {
        if (node.handle > 0) {
            if (node.handle >= m_handleToNode.size()) {
                m_handleToNode.resize(node.handle + 1, -1);
            }
            m_handleToNode[node.handle] = m_nodes.size();
        }
    }

    /* 迁移节点 */
    int offset = m_nodes.size();
    for (const auto& n : other.m_nodes) {
        m_nodes.append(n);
    }
    /* 修正索引 */
    for (int i = offset; i < m_nodes.size(); ++i) {
        if (m_nodes[i].child >= 0) m_nodes[i].child += offset;
        if (m_nodes[i].sibling >= 0) m_nodes[i].sibling += offset;
        if (m_nodes[i].parent >= 0) m_nodes[i].parent += offset;
    }
    for (int& r : combined) {
        if (other.m_roots.contains(r)) r += offset;
    }

    /* 按度数合并 */
    std::sort(combined.begin(), combined.end(),
        [this](int a, int b) { return m_nodes[a].degree < m_nodes[b].degree; });

    m_roots.clear();
    int i = 0;
    while (i < combined.size()) {
        if (i + 1 < combined.size() && m_nodes[combined[i]].degree == m_nodes[combined[i+1]].degree) {
            int merged = mergeTrees(combined[i], combined[i+1]);
            combined[i+1] = merged;
            ++i;
        } else {
            m_roots.append(combined[i]);
            ++i;
        }
    }

    m_stats.currentSize += other.m_stats.currentSize;
    if (m_stats.currentSize > m_stats.peakSize) {
        m_stats.peakSize = m_stats.currentSize;
    }

    /* 清空other */
    other.m_nodes.clear();
    other.m_roots.clear();
    other.m_stats.currentSize = 0;

    ++m_stats.totalMerges;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
            + m_stats.totalMerges + m_stats.totalDecreaseKeys);
}

/**
 * @brief 降低指定元素的键值
 * @param handle 元素句柄
 * @param newValue 新值(必须小于当前值)
 * @return 0成功, -1失败
 */
int BinomialQueue::decreaseKey(int handle, double newValue)
{
    QElapsedTimer timer;
    timer.start();

    if (handle < 0 || handle >= m_handleToNode.size()) return -1;
    int nodeIdx = m_handleToNode[handle];
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return -1;

    double oldValue = m_nodes[nodeIdx].value;
    if (newValue > oldValue) return -1;

    m_nodes[nodeIdx].value = newValue;
    bubbleUp(nodeIdx);

    ++m_stats.totalDecreaseKeys;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
            + m_stats.totalMerges + m_stats.totalDecreaseKeys);

    emit keyDecreased(handle, oldValue, newValue);
    return 0;
}

/**
 * @brief 惰性删除指定元素
 * @param handle 元素句柄
 */
void BinomialQueue::remove(int handle)
{
    if (handle < 0 || handle >= m_handleToNode.size()) return;
    int nodeIdx = m_handleToNode[handle];
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;

    /* 标记为惰性删除 */
    m_nodes[nodeIdx].marked = true;
    m_nodes[nodeIdx].value = -std::numeric_limits<double>::max();
    bubbleUp(nodeIdx);

    /* 执行deleteMin将标记元素移除 */
    deleteMin();
}

/**
 * @brief 检查句柄是否存在
 * @param handle 元素句柄
 * @return 是否存在
 */
bool BinomialQueue::contains(int handle) const
{
    if (handle < 0 || handle >= m_handleToNode.size()) return false;
    int nodeIdx = m_handleToNode[handle];
    return nodeIdx >= 0 && nodeIdx < m_nodes.size();
}

/** @brief 队列是否为空 @return 空则true */
bool BinomialQueue::isEmpty() const { return m_roots.isEmpty(); }

/** @brief 当前元素数量 @return 元素数 */
int BinomialQueue::size() const { return m_stats.currentSize; }

/** @brief 重置统计信息 */
void BinomialQueue::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 创建新节点
 * @param value 节点值
 * @return 节点索引
 */
int BinomialQueue::createNode(double value)
{
    int handle = m_nextHandle++;
    int idx = m_nodes.size();
    Node n;
    n.value = value;
    n.handle = handle;
    m_nodes.append(n);

    if (handle >= m_handleToNode.size()) {
        m_handleToNode.resize(handle + 1, -1);
    }
    m_handleToNode[handle] = idx;
    return idx;
}

/**
 * @brief 合并两棵同度数二项树
 * @param root1 树1根索引
 * @param root2 树2根索引
 * @return 合并后的根索引
 */
int BinomialQueue::mergeTrees(int root1, int root2)
{
    /* 保证root1值更小(最小堆) */
    if (m_nodes[root1].value > m_nodes[root2].value) {
        std::swap(root1, root2);
    }

    /* root2成为root1的最左子节点 */
    m_nodes[root2].sibling = m_nodes[root1].child;
    m_nodes[root2].parent = root1;
    m_nodes[root1].child = root2;
    ++m_nodes[root1].degree;

    return root1;
}

/**
 * @brief 收集以root为根的所有子树
 * @param root 根节点索引
 * @param trees [out] 子树根列表
 */
void BinomialQueue::collectTrees(int root, QList<int>& trees) const
{
    int child = m_nodes[root].child;
    while (child >= 0) {
        trees.append(child);
        child = m_nodes[child].sibling;
    }
}

/**
 * @brief 找到森林中最小根
 * @return 最小根索引，-1表示空
 */
int BinomialQueue::findMinRoot() const
{
    if (m_roots.isEmpty()) return -1;
    int minIdx = 0;
    for (int i = 1; i < m_roots.size(); ++i) {
        if (m_nodes[m_roots[i]].value < m_nodes[m_roots[minIdx]].value) {
            minIdx = i;
        }
    }
    return m_roots[minIdx];
}

/**
 * @brief 将节点上浮(用于decrease-key)
 * @param nodeIdx 节点索引
 */
void BinomialQueue::bubbleUp(int nodeIdx)
{
    while (m_nodes[nodeIdx].parent >= 0) {
        int parent = m_nodes[nodeIdx].parent;
        if (m_nodes[nodeIdx].value >= m_nodes[parent].value) break;

        /* 交换节点值和句柄(而非交换节点位置) */
        std::swap(m_nodes[nodeIdx].value, m_nodes[parent].value);
        std::swap(m_nodes[nodeIdx].handle, m_nodes[parent].handle);

        /* 更新句柄映射 */
        m_handleToNode[m_nodes[nodeIdx].handle] = nodeIdx;
        m_handleToNode[m_nodes[parent].handle] = parent;

        nodeIdx = parent;
    }
}

/**
 * @brief 从父节点断开(用于某些删除操作)
 * @param nodeIdx 节点索引
 */
void BinomialQueue::cutFromParent(int nodeIdx)
{
    int parent = m_nodes[nodeIdx].parent;
    if (parent < 0) return;

    /* 从父节点的子链中移除 */
    int child = m_nodes[parent].child;
    if (child == nodeIdx) {
        m_nodes[parent].child = m_nodes[nodeIdx].sibling;
    } else {
        int prev = child;
        while (prev >= 0 && m_nodes[prev].sibling != nodeIdx) {
            prev = m_nodes[prev].sibling;
        }
        if (prev >= 0) {
            m_nodes[prev].sibling = m_nodes[nodeIdx].sibling;
        }
    }
    --m_nodes[parent].degree;
    m_nodes[nodeIdx].parent = -1;
    m_nodes[nodeIdx].sibling = -1;
    m_roots.append(nodeIdx);
}
