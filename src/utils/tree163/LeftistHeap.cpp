/**
 * @file LeftistHeap.cpp
 * @brief LeftistHeap 实现
 *
 * 实现左偏堆：基于秩的mergeNodes核心操作、插入、删除最小值、
 * decreaseKey和树高度统计。
 */

#include "utils/tree163/LeftistHeap.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
LeftistHeap::LeftistHeap(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数：释放所有节点
 */
LeftistHeap::~LeftistHeap()
{
    clear();
}

/**
 * @brief 计算节点秩(null path length)
 *
 * 空节点秩为0，叶子节点秩为1。
 * 非空节点秩 = min(rank(left), rank(right)) + 1。
 */
int LeftistHeap::nodeRank(Node* node)
{
    if (!node) return 0;
    return node->rank;
}

/**
 * @brief 递归释放子树
 */
void LeftistHeap::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    m_nodeMap.remove(node->id);
    delete node;
}

/**
 * @brief 合并两棵左偏子树(核心操作)
 *
 * 1) 递归合并key较小的右子树与另一棵树
 * 2) 保证左子树的秩 >= 右子树的秩(左偏性质)
 * 3) 更新当前节点的秩 = rank(right) + 1
 */
LeftistHeap::Node* LeftistHeap::mergeNodes(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    /* 保证a的key <= b的key */
    if (a->key > b->key) {
        std::swap(a, b);
    }

    /* 递归合并a的右子树和b */
    a->right = mergeNodes(a->right, b);

    /* 左偏性质：rank(left) >= rank(right) */
    if (nodeRank(a->left) < nodeRank(a->right)) {
        std::swap(a->left, a->right);
    }

    /* 更新秩 */
    a->rank = nodeRank(a->right) + 1;

    return a;
}

/**
 * @brief 更新堆高度统计
 */
void LeftistHeap::updateHeight()
{
    m_stats.height = 0;
    if (!m_root) return;

    /* 递归计算高度 */
    QVector<QPair<Node*, int>> stack;
    stack.append({m_root, 1});

    while (!stack.isEmpty()) {
        auto [node, depth] = stack.takeLast();
        m_stats.height = qMax(m_stats.height, depth);

        if (node->left) stack.append({node->left, depth + 1});
        if (node->right) stack.append({node->right, depth + 1});
    }
}

/**
 * @brief 插入键值对
 *
 * 创建新节点，然后与当前堆合并。
 */
int LeftistHeap::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = new Node{key, value, 1, m_nextId, nullptr, nullptr};
    m_nodeMap[m_nextId] = node;

    m_root = mergeNodes(m_root, node);

    m_stats.totalInserts++;
    m_stats.currentSize++;
    updateHeight();
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    return m_nextId++;
}

/**
 * @brief 获取最小键值
 */
bool LeftistHeap::findMin(double& key, double& value) const
{
    if (!m_root) return false;
    key = m_root->key;
    value = m_root->value;
    return true;
}

/**
 * @brief 删除并返回最小元素
 *
 * 移除根节点，合并左右子树。
 */
bool LeftistHeap::deleteMin(double& key, double& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    key = m_root->key;
    value = m_root->value;

    Node* oldRoot = m_root;
    m_nodeMap.remove(oldRoot->id);

    /* 合并左右子树 */
    m_root = mergeNodes(oldRoot->left, oldRoot->right);
    delete oldRoot;

    m_stats.totalDeleteMins++;
    m_stats.currentSize--;
    updateHeight();
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit deleteMinCompleted(key);
    return true;
}

/**
 * @brief 移除指定节点(用于decreaseKey)
 *
 * 从子树中摘除指定id的节点，返回摘除后的子树根。
 */
LeftistHeap::Node* LeftistHeap::removeNode(Node* root, int id)
{
    if (!root) return nullptr;

    if (root->id == id) {
        /* 摘除此节点，合并其左右子树 */
        Node* merged = mergeNodes(root->left, root->right);
        /* 不delete root，decreaseKey会重新插入 */
        return merged;
    }

    /* 递归搜索 */
    root->left = removeNode(root->left, id);
    root->right = removeNode(root->right, id);

    /* 重新维护左偏性质 */
    if (nodeRank(root->left) < nodeRank(root->right)) {
        std::swap(root->left, root->right);
    }
    root->rank = nodeRank(root->right) + 1;

    return root;
}

/**
 * @brief 降低指定元素的键值
 *
 * 1) 从树中摘除该节点
 * 2) 更新key
 * 3) 重新合并到堆中
 */
bool LeftistHeap::decreaseKey(int id, double newKey)
{
    QElapsedTimer timer;
    timer.start();

    auto it = m_nodeMap.find(id);
    if (it == m_nodeMap.end()) return false;

    Node* node = it.value();
    if (newKey >= node->key) return false;

    /* 从树中摘除 */
    m_root = removeNode(m_root, id);

    /* 更新key */
    node->key = newKey;
    node->left = nullptr;
    node->right = nullptr;
    node->rank = 1;

    /* 重新合并 */
    m_root = mergeNodes(m_root, node);

    m_stats.totalDecreaseKeys++;
    updateHeight();
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeleteMins;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return true;
}

/**
 * @brief 合并另一个堆
 */
void LeftistHeap::merge(LeftistHeap& other)
{
    m_root = mergeNodes(m_root, other.m_root);

    /* 转移节点映射 */
    for (auto it = other.m_nodeMap.begin(); it != other.m_nodeMap.end(); ++it) {
        m_nodeMap.insert(it.key(), it.value());
    }
    m_stats.currentSize += other.m_stats.currentSize;
    m_stats.totalMerges++;

    other.m_root = nullptr;
    other.m_nodeMap.clear();
    other.m_stats.currentSize = 0;

    updateHeight();
}

bool LeftistHeap::isEmpty() const
{
    return m_root == nullptr;
}

int LeftistHeap::size() const
{
    return m_stats.currentSize;
}

void LeftistHeap::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.height = 0;
}

void LeftistHeap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
