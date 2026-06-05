/**
 * @file PairingHeap.cpp
 * @brief 配对堆实现 — 支持merge/decrease-key的优先队列
 */

#include "utils/heap7/PairingHeap.h"

#include <QElapsedTimer>
#include <QStack>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PairingHeap::PairingHeap(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构函数 — 释放所有节点 */
PairingHeap::~PairingHeap()
{
    destroyTree(m_root);
}

/** @brief 插入元素 @param key 键值 @param data 关联数据 @return 节点指针 */
PairingHeap::Node* PairingHeap::insert(double key, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = new Node();
    node->key = key;
    node->data = data;

    m_root = mergeTrees(m_root, node);
    ++m_size;

    ++m_stats.totalInserts;
    ++m_stats.totalNodesCreated;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalDeleteMins
           + m_stats.totalMerges + m_stats.totalDecreaseKeys);

    return node;
}

/** @brief 合并另一个堆 @param other 被合并的堆 */
void PairingHeap::merge(PairingHeap* other)
{
    if (!other || other == this) return;

    QElapsedTimer timer;
    timer.start();

    m_root = mergeTrees(m_root, other->m_root);
    m_size += other->m_size;

    m_stats.totalNodesCreated += other->m_stats.totalNodesCreated;

    /* 清空被合并的堆 */
    other->m_root = nullptr;
    other->m_size = 0;

    ++m_stats.totalMerges;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalDeleteMins
           + m_stats.totalMerges + m_stats.totalDecreaseKeys);

    emit heapMerged(other->m_size);
}

/** @brief 查找最小键值 @return 最小节点指针 */
PairingHeap::Node* PairingHeap::findMin() const
{
    return m_root;
}

/** @brief 删除最小元素 @return 被删除节点的数据 */
QVariant PairingHeap::deleteMin()
{
    if (!m_root) return QVariant();

    QElapsedTimer timer;
    timer.start();

    Node* minNode = m_root;
    QVariant data = minNode->data;
    double key = minNode->key;

    /* 对子节点执行两趟合并 */
    m_root = twoPassMerge(minNode->child);
    --m_size;

    delete minNode;

    ++m_stats.totalDeleteMins;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalDeleteMins
           + m_stats.totalMerges + m_stats.totalDecreaseKeys);

    emit minDeleted(key);
    return data;
}

/** @brief 减小节点键值 @param node 目标节点 @param newKey 新键值 */
void PairingHeap::decreaseKey(Node* node, double newKey)
{
    if (!node || newKey >= node->key) return;

    QElapsedTimer timer;
    timer.start();

    node->key = newKey;

    /* 如果不是根节点，需要从父节点处断开并重新合并 */
    if (node->parent) {
        /* 从兄弟链表中移除 */
        Node* p = node->parent;
        if (p->child == node) {
            p->child = node->sibling;
        } else {
            Node* prev = p->child;
            while (prev && prev->sibling != node) {
                prev = prev->sibling;
            }
            if (prev) {
                prev->sibling = node->sibling;
            }
        }
        node->sibling = nullptr;
        node->parent = nullptr;

        /* 重新合并到堆 */
        m_root = mergeTrees(m_root, node);
    }

    ++m_stats.totalDecreaseKeys;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalDeleteMins
           + m_stats.totalMerges + m_stats.totalDecreaseKeys);
}

/** @brief 清空堆 */
void PairingHeap::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 将所有元素排序输出 @return 排序列表 */
QVector<QPair<double, QVariant>> PairingHeap::toSortedList()
{
    QVector<QPair<double, QVariant>> result;
    result.reserve(m_size);

    /* 逐个删除最小值来排序 */
    PairingHeap temp;
    temp.m_root = m_root;
    temp.m_size = m_size;

    while (!temp.isEmpty()) {
        Node* min = temp.findMin();
        result.append({min->key, min->data});
        temp.deleteMin();
    }

    return result;
}

/** @brief 重置统计 */
void PairingHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 合并两棵树 @param a 树A @param b 树B @return 合并后的根 */
PairingHeap::Node* PairingHeap::mergeTrees(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    /* 较小键值作为根 */
    if (a->key <= b->key) {
        b->sibling = a->child;
        b->parent = a;
        a->child = b;
        a->sibling = nullptr;
        a->parent = nullptr;
        return a;
    } else {
        a->sibling = b->child;
        a->parent = b;
        b->child = a;
        b->sibling = nullptr;
        b->parent = nullptr;
        return b;
    }
}

/** @brief 两趟合并 @param first 第一个子节点 @return 合并后的根 */
PairingHeap::Node* PairingHeap::twoPassMerge(Node* first)
{
    if (!first || !first->sibling) return first;

    /* 第一趟: 从左到右两两配对合并 */
    QVector<Node*> pairs;
    Node* current = first;
    while (current) {
        Node* next = current->sibling;
        current->sibling = nullptr;
        current->parent = nullptr;

        if (next) {
            Node* nextNext = next->sibling;
            next->sibling = nullptr;
            next->parent = nullptr;
            pairs.append(mergeTrees(current, next));
            current = nextNext;
        } else {
            pairs.append(current);
            current = nullptr;
        }
    }

    /* 第二趟: 从右到左依次合并 */
    Node* result = pairs.last();
    for (int i = pairs.size() - 2; i >= 0; --i) {
        result = mergeTrees(result, pairs[i]);
    }

    return result;
}

/** @brief 递归删除子树 @param node 根节点 */
void PairingHeap::destroyTree(Node* node)
{
    if (!node) return;
    /* 迭代删除避免栈溢出 */
    QStack<Node*> stack;
    stack.push(node);
    while (!stack.isEmpty()) {
        Node* n = stack.pop();
        if (n->child) stack.push(n->child);
        if (n->sibling) stack.push(n->sibling);
        delete n;
    }
}

/** @brief 计算子树大小 @param node 根节点 @return 节点数 */
int PairingHeap::countNodes(Node* node) const
{
    if (!node) return 0;
    int count = 0;
    QStack<Node*> stack;
    stack.push(node);
    while (!stack.isEmpty()) {
        Node* n = stack.pop();
        ++count;
        if (n->child) stack.push(n->child);
        if (n->sibling) stack.push(n->sibling);
    }
    return count;
}
