/**
 * @file FibonacciHeap2.cpp
 * @brief Fibonacci堆实现 — 延迟合并/consolidate/标记裁剪级联
 */

#include "utils/tree26/FibonacciHeap2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
FibonacciHeap2::FibonacciHeap2(QObject* parent)
    : QObject(parent)
    , m_minNode(-1)
    , m_rootCount(0)
    , m_size(0)
    , m_maxDegree(0)
{
}

/** @brief 插入元素 @param key 键值 @return 节点ID */
FibonacciHeap2::NodeId FibonacciHeap2::insert(double key)
{
    FHNode node;
    node.key = key;
    node.degree = 0;
    node.mark = false;
    node.parent = -1;
    node.child = -1;

    NodeId id = m_nodes.size();
    m_nodes.append(node);

    /* 自身形成循环链表 */
    m_nodes[id].left = id;
    m_nodes[id].right = id;

    addToRootList(id);

    /* 更新最小节点 */
    if (m_minNode < 0 || key < m_nodes[m_minNode].key) {
        m_minNode = id;
    }

    m_size++;
    m_stats.totalInserts++;
    emit nodeInserted(id, key);
    return id;
}

/** @brief 查找最小键值 @return 最小键值 */
double FibonacciHeap2::findMin() const
{
    if (m_minNode < 0) return qQNaN();
    return m_nodes[m_minNode].key;
}

/** @brief 提取最小元素 @return 最小键值 */
double FibonacciHeap2::extractMin()
{
    QElapsedTimer timer;
    timer.start();

    if (m_minNode < 0) return qQNaN();

    NodeId z = m_minNode;
    double minKey = m_nodes[z].key;

    /* 将z的所有子节点添加到根链表 */
    if (m_nodes[z].child >= 0) {
        NodeId child = m_nodes[z].child;
        QVector<NodeId> children;
        NodeId cur = child;
        do {
            children.append(cur);
            cur = m_nodes[cur].right;
        } while (cur != child);

        for (NodeId c : children) {
            m_nodes[c].parent = -1;
            m_nodes[c].mark = false;
            addToRootList(c);
        }
    }

    /* 从根链表中移除z */
    removeFromList(z);
    m_rootCount--;

    if (z == m_nodes[z].right) {
        /* 根链表只剩z一个节点 */
        m_minNode = -1;
        m_rootCount = 0;
    } else {
        m_minNode = m_nodes[z].right;
        consolidate();
    }

    m_size--;
    m_stats.totalExtractMin++;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalExtractMin + m_stats.totalMerges);

    emit minExtracted(minKey);
    return minKey;
}

/** @brief 减小节点键值 @param node 节点ID @param newKey 新键值 */
void FibonacciHeap2::decreaseKey(NodeId node, double newKey)
{
    if (node < 0 || node >= m_nodes.size()) return;
    if (newKey > m_nodes[node].key) return;

    m_nodes[node].key = newKey;
    NodeId parent = m_nodes[node].parent;

    if (parent >= 0 && newKey < m_nodes[parent].key) {
        cutNode(node);
        cascadingCut(parent);
    }

    if (newKey < m_nodes[m_minNode].key) {
        m_minNode = node;
    }

    m_stats.totalDecreaseKey++;
}

/** @brief 合并另一个堆 @param other 另一个Fibonacci堆 */
void FibonacciHeap2::merge(FibonacciHeap2& other)
{
    if (other.isEmpty()) return;

    /* 将other的根链表连接到this的根链表 */
    if (m_minNode < 0) {
        m_minNode = other.m_minNode;
    } else if (other.m_minNode >= 0) {
        /* 拼接两个循环链表 */
        NodeId thisRight = m_nodes[m_minNode].right;
        NodeId otherRight = other.m_nodes[other.m_minNode].right;

        m_nodes[m_minNode].right = other.m_minNode;
        other.m_nodes[other.m_minNode].left = m_minNode;

        m_nodes[thisRight].left = otherRight;
        other.m_nodes[otherRight].right = thisRight;

        if (other.m_nodes[other.m_minNode].key
            < m_nodes[m_minNode].key) {
            m_minNode = other.m_minNode;
        }
    }

    /* 转移所有节点 */
    int offset = m_nodes.size();
    m_nodes.resize(offset + other.m_nodes.size());

    for (int i = 0; i < other.m_nodes.size(); ++i) {
        auto& src = other.m_nodes[i];
        auto& dst = m_nodes[offset + i];
        dst = src;
        /* 修正所有ID引用 */
        if (dst.parent >= 0) dst.parent += offset;
        if (dst.child >= 0) dst.child += offset;
        if (dst.left >= 0) dst.left += offset;
        if (dst.right >= 0) dst.right += offset;
    }

    m_size += other.m_size;
    m_rootCount += other.m_rootCount;
    m_stats.totalMerges++;

    /* 清空other */
    other.m_nodes.clear();
    other.m_minNode = -1;
    other.m_size = 0;
    other.m_rootCount = 0;
}

/** @brief 是否为空 @return 空=true */
bool FibonacciHeap2::isEmpty() const
{
    return m_size == 0;
}

/** @brief 堆大小 @return 节点数 */
int FibonacciHeap2::size() const
{
    return m_size;
}

/** @brief 清空堆 */
void FibonacciHeap2::clear()
{
    m_nodes.clear();
    m_minNode = -1;
    m_rootCount = 0;
    m_size = 0;
    m_maxDegree = 0;
}

/** @brief 遍历所有键值 @return 键值列表 */
QList<double> FibonacciHeap2::toList() const
{
    QList<double> result;
    for (const auto& node : m_nodes) {
        result.append(node.key);
    }
    return result;
}

/** @brief 重置统计 */
void FibonacciHeap2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 合并根链表中度数相同的树 */
void FibonacciHeap2::consolidate()
{
    if (m_rootCount == 0) return;

    int maxDeg = static_cast<int>(
        std::floor(std::log2(static_cast<double>(m_size)))) + 2;
    if (maxDeg < m_maxDegree) maxDeg = m_maxDegree;
    m_maxDegree = maxDeg;

    QVector<NodeId> degreeToRoot(maxDeg + 1, -1);

    /* 收集所有根节点 */
    QVector<NodeId> roots;
    NodeId cur = m_minNode;
    do {
        roots.append(cur);
        cur = m_nodes[cur].right;
    } while (cur != m_minNode);

    for (NodeId w : roots) {
        NodeId x = w;
        int d = m_nodes[x].degree;

        while (d <= maxDeg && degreeToRoot[d] >= 0) {
            NodeId y = degreeToRoot[d];
            if (m_nodes[x].key > m_nodes[y].key) {
                std::swap(x, y);
            }
            linkRoots(y, x);
            degreeToRoot[d] = -1;
            d++;
        }

        if (d <= maxDeg) degreeToRoot[d] = x;
    }

    /* 重建根链表并找最小 */
    m_minNode = -1;
    m_rootCount = 0;

    for (int d = 0; d <= maxDeg; ++d) {
        if (degreeToRoot[d] >= 0) {
            m_nodes[degreeToRoot[d]].left = degreeToRoot[d];
            m_nodes[degreeToRoot[d]].right = degreeToRoot[d];
            if (m_minNode < 0) {
                m_minNode = degreeToRoot[d];
            } else {
                addToRootList(degreeToRoot[d]);
                if (m_nodes[degreeToRoot[d]].key
                    < m_nodes[m_minNode].key) {
                    m_minNode = degreeToRoot[d];
                }
            }
            m_rootCount++;
        }
    }
}

/** @brief 将child链接为parent的子节点 @param child 子树根 @param parent 父树根 */
void FibonacciHeap2::linkRoots(NodeId child, NodeId parent)
{
    removeFromList(child);
    m_nodes[child].parent = parent;
    m_nodes[child].mark = false;

    if (m_nodes[parent].child < 0) {
        m_nodes[parent].child = child;
        m_nodes[child].left = child;
        m_nodes[child].right = child;
    } else {
        NodeId firstChild = m_nodes[parent].child;
        m_nodes[child].right = firstChild;
        m_nodes[child].left = m_nodes[firstChild].left;
        m_nodes[m_nodes[firstChild].left].right = child;
        m_nodes[firstChild].left = child;
    }

    m_nodes[parent].degree++;
}

/** @brief 从父节点剪切子节点 @param node 被剪切的节点 */
void FibonacciHeap2::cutNode(NodeId node)
{
    NodeId parent = m_nodes[node].parent;
    if (parent < 0) return;

    /* 从父节点的子链表中移除 */
    m_nodes[parent].degree--;
    if (m_nodes[parent].child == node) {
        if (m_nodes[node].right == node) {
            m_nodes[parent].child = -1;
        } else {
            m_nodes[parent].child = m_nodes[node].right;
        }
    }
    removeFromList(node);

    /* 添加到根链表 */
    m_nodes[node].parent = -1;
    m_nodes[node].mark = false;
    addToRootList(node);
}

/** @brief 级联剪切 @param node 起始节点 */
void FibonacciHeap2::cascadingCut(NodeId node)
{
    NodeId parent = m_nodes[node].parent;
    if (parent < 0) return;

    if (!m_nodes[node].mark) {
        m_nodes[node].mark = true;
    } else {
        cutNode(node);
        cascadingCut(parent);
    }
}

/** @brief 添加节点到根链表 @param node 节点ID */
void FibonacciHeap2::addToRootList(NodeId node)
{
    if (m_minNode < 0) {
        m_minNode = node;
        m_nodes[node].left = node;
        m_nodes[node].right = node;
        m_rootCount = 1;
        return;
    }

    NodeId rightOfMin = m_nodes[m_minNode].right;
    m_nodes[node].left = m_minNode;
    m_nodes[node].right = rightOfMin;
    m_nodes[m_minNode].right = node;
    m_nodes[rightOfMin].left = node;
    m_rootCount++;
}

/** @brief 从循环链表中移除节点 @param node 节点ID */
void FibonacciHeap2::removeFromList(NodeId node)
{
    if (m_nodes[node].left == node && m_nodes[node].right == node) return;

    m_nodes[m_nodes[node].left].right = m_nodes[node].right;
    m_nodes[m_nodes[node].right].left = m_nodes[node].left;
    m_nodes[node].left = node;
    m_nodes[node].right = node;
}
