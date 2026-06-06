/**
 * @file PairingHeap.cpp
 * @brief PairingHeap 实现
 *
 * 实现配对堆：O(1) merge/insert、两趟合并delete-min、
 * decrease-key(剪切+重合并)。
 */

#include "utils/tree164/PairingHeap.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
PairingHeap::PairingHeap(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数：释放所有节点
 */
PairingHeap::~PairingHeap()
{
    clear();
}

/**
 * @brief 递归释放子树
 */
void PairingHeap::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->child);
    deleteTree(node->sibling);
    m_nodeMap.remove(node->id);
    delete node;
}

/**
 * @brief 合并两棵子树
 *
 * 取key较小的作为根，较大的作为其最左子节点的兄弟。
 */
PairingHeap::Node* PairingHeap::mergeNodes(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    if (a->key > b->key) {
        std::swap(a, b);
    }

    /* b becomes a's child: insert as leftmost child */
    b->sibling = a->child;
    a->child = b;
    a->sibling = nullptr;
    return a;
}

/**
 * @brief 两趟合并(核心操作)
 *
 * 用于delete-min后合并根的所有子节点。
 * 第一趟: 从左到右，两两配对合并。
 * 第二趟: 从右到左，依次累积合并。
 */
PairingHeap::Node* PairingHeap::twoPassMerge(Node* firstChild)
{
    if (!firstChild || !firstChild->sibling) return firstChild;

    /* First pass: pair-wise merge from left to right */
    QVector<Node*> pairs;
    Node* current = firstChild;
    while (current) {
        Node* next = current->sibling;
        current->sibling = nullptr;

        if (next) {
            Node* nextNext = next->sibling;
            next->sibling = nullptr;
            pairs.append(mergeNodes(current, next));
            current = nextNext;
        } else {
            pairs.append(current);
            current = nullptr;
        }
    }

    /* Second pass: merge from right to left */
    Node* result = pairs.last();
    for (int i = pairs.size() - 2; i >= 0; --i) {
        result = mergeNodes(pairs[i], result);
    }

    return result;
}

/**
 * @brief 插入键值对
 *
 * 创建新节点并与当前堆合并。
 */
int PairingHeap::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = new Node{key, value, m_nextId, nullptr, nullptr};
    m_nodeMap[m_nextId] = node;

    m_root = mergeNodes(m_root, node);

    m_stats.totalInserts++;
    m_stats.currentSize++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    return m_nextId++;
}

/**
 * @brief 获取最小键值
 */
bool PairingHeap::findMin(double& key, double& value) const
{
    if (!m_root) return false;
    key = m_root->key;
    value = m_root->value;
    return true;
}

/**
 * @brief 删除并返回最小元素
 *
 * 移除根节点，用两趟合并处理其子节点。
 */
bool PairingHeap::deleteMin(double& key, double& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    key = m_root->key;
    value = m_root->value;

    Node* oldRoot = m_root;
    m_nodeMap.remove(oldRoot->id);

    /* Two-pass merge the children */
    m_root = twoPassMerge(oldRoot->child);
    oldRoot->child = nullptr;
    delete oldRoot;

    m_stats.totalDeleteMins++;
    m_stats.currentSize--;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit deleteMinCompleted(key);
    return true;
}

/**
 * @brief 降低指定元素的键值
 *
 * 从树中剪切该节点(断开与其父节点的连接)，
 * 更新key后重新合并到根。
 */
bool PairingHeap::decreaseKey(int id, double newKey)
{
    QElapsedTimer timer;
    timer.start();

    auto it = m_nodeMap.find(id);
    if (it == m_nodeMap.end()) return false;

    Node* node = it.value();
    if (newKey >= node->key) return false;

    if (node == m_root) {
        /* Root node, just update key */
        node->key = newKey;
    } else {
        /* Cut node from tree: we need to find and unlink it */
        /* For pairing heap, we simply detach and re-merge */
        /* The node must be a child of some parent or a sibling */
        /* We search the tree to unlink this node */

        /* Find parent by searching (BFS) */
        Node* parent = nullptr;
        bool found = false;
        QVector<Node*> queue;
        if (m_root) queue.append(m_root);

        while (!queue.isEmpty() && !found) {
            Node* curr = queue.takeFirst();
            Node* prev = nullptr;
            Node* child = curr->child;

            while (child && !found) {
                if (child == node) {
                    /* Unlink child from parent */
                    if (prev) {
                        prev->sibling = child->sibling;
                    } else {
                        curr->child = child->sibling;
                    }
                    child->sibling = nullptr;
                    found = true;
                    break;
                }
                queue.append(child);
                prev = child;
                child = child->sibling;
            }
        }

        if (!found) return false;

        /* Update key and re-merge */
        node->key = newKey;
        node->sibling = nullptr;
        m_root = mergeNodes(m_root, node);
    }

    m_stats.totalDecreaseKeys++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return true;
}

/**
 * @brief 合并另一个堆
 */
void PairingHeap::merge(PairingHeap& other)
{
    m_root = mergeNodes(m_root, other.m_root);

    /* Transfer node map */
    for (auto it = other.m_nodeMap.begin(); it != other.m_nodeMap.end(); ++it) {
        m_nodeMap.insert(it.key(), it.value());
    }
    m_stats.currentSize += other.m_stats.currentSize;
    m_stats.totalMerges++;

    other.m_root = nullptr;
    other.m_nodeMap.clear();
    other.m_stats.currentSize = 0;
}

bool PairingHeap::isEmpty() const
{
    return m_root == nullptr;
}

int PairingHeap::size() const
{
    return m_stats.currentSize;
}

void PairingHeap::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
}

void PairingHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
