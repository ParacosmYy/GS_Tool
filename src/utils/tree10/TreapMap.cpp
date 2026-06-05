/**
 * @file TreapMap.cpp
 * @brief Treap映射实现 — 基于随机优先级的平衡树
 */

#include "utils/tree10/TreapMap.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cstdlib>

/** @brief 构造函数 @param parent 父对象 */
TreapMap::TreapMap(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
{
}

/** @brief 析构函数 */
TreapMap::~TreapMap()
{
    destroyTree(m_root);
}

/** @brief 递归销毁子树 @param node 节点 */
void TreapMap::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

/** @brief 深拷贝子树 @param node 源节点 @return 新节点 */
TreapMap::Node* TreapMap::cloneTree(Node* node) const
{
    if (!node) return nullptr;
    Node* copy = new Node(node->key, node->value, node->priority);
    copy->subtreeSize = node->subtreeSize;
    copy->left = cloneTree(node->left);
    copy->right = cloneTree(node->right);
    return copy;
}

/** @brief 更新子树大小 @param node 节点 */
void TreapMap::updateSize(Node* node)
{
    if (!node) return;
    node->subtreeSize = 1;
    if (node->left) node->subtreeSize += node->left->subtreeSize;
    if (node->right) node->subtreeSize += node->right->subtreeSize;
}

/** @brief 右旋转 @param node 旋转根 @return 新根 */
TreapMap::Node* TreapMap::rotateRight(Node* node)
{
    if (!node || !node->left) return node;
    Node* leftChild = node->left;
    node->left = leftChild->right;
    leftChild->right = node;
    updateSize(node);
    updateSize(leftChild);
    m_stats.totalRotations++;
    return leftChild;
}

/** @brief 左旋转 @param node 旋转根 @return 新根 */
TreapMap::Node* TreapMap::rotateLeft(Node* node)
{
    if (!node || !node->right) return node;
    Node* rightChild = node->right;
    node->right = rightChild->left;
    rightChild->left = node;
    updateSize(node);
    updateSize(rightChild);
    m_stats.totalRotations++;
    return rightChild;
}

/** @brief 递归插入 @param node 当前节点 @param key 键 @param value 值 @param priority 优先级 @return 新根 */
TreapMap::Node* TreapMap::insertImpl(Node* node, int key, double value, int priority)
{
    if (!node) {
        m_size++;
        return new Node(key, value, priority);
    }

    if (key < node->key) {
        node->left = insertImpl(node->left, key, value, priority);
        if (node->left && node->left->priority > node->priority) {
            node = rotateRight(node);
        }
    } else if (key > node->key) {
        node->right = insertImpl(node->right, key, value, priority);
        if (node->right && node->right->priority > node->priority) {
            node = rotateLeft(node);
        }
    } else {
        /* 键已存在，更新值 */
        node->value = value;
    }

    updateSize(node);
    return node;
}

/** @brief 递归删除 @param node 当前节点 @param key 键 @return 新根 */
TreapMap::Node* TreapMap::removeImpl(Node* node, int key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeImpl(node->left, key);
    } else if (key > node->key) {
        node->right = removeImpl(node->right, key);
    } else {
        /* 找到目标节点 */
        if (!node->left && !node->right) {
            /* 叶子节点直接删除 */
            m_size--;
            delete node;
            return nullptr;
        }
        /* 将节点旋转到叶子位置 */
        if (!node->left || (node->right && node->right->priority > node->left->priority)) {
            node = rotateLeft(node);
            node->left = removeImpl(node->left, key);
        } else {
            node = rotateRight(node);
            node->right = removeImpl(node->right, key);
        }
    }

    updateSize(node);
    return node;
}

/** @brief 分裂实现 @param node 当前节点 @param threshold 阈值 @param left 输出左树 @param right 输出右树 */
void TreapMap::splitImpl(Node* node, int threshold, Node*& left, Node*& right)
{
    if (!node) {
        left = nullptr;
        right = nullptr;
        return;
    }

    if (node->key <= threshold) {
        splitImpl(node->right, threshold, node->right, right);
        left = node;
    } else {
        splitImpl(node->left, threshold, left, node->left);
        right = node;
    }
    updateSize(node);
}

/** @brief 合并实现 @param left 左树根 @param right 右树根 @return 合并后的根 */
TreapMap::Node* TreapMap::mergeImpl(Node* left, Node* right)
{
    if (!left) return right;
    if (!right) return left;

    if (left->priority > right->priority) {
        left->right = mergeImpl(left->right, right);
        updateSize(left);
        return left;
    } else {
        right->left = mergeImpl(left, right->left);
        updateSize(right);
        return right;
    }
}

/** @brief 插入键值对 @param key 键 @param value 值 */
void TreapMap::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    int priority = qrand();
    m_root = insertImpl(m_root, key, value, priority);

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalFinds
           + m_stats.totalSplits + m_stats.totalMerges);
}

/** @brief 删除键 @param key 键 @return 是否成功 */
bool TreapMap::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = removeImpl(m_root, key);
    bool removed = (m_size < oldSize);

    m_stats.totalRemoves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalFinds
           + m_stats.totalSplits + m_stats.totalMerges);
    return removed;
}

/** @brief 查找键 @param key 键 @param value 输出值 @return 是否找到 */
bool TreapMap::find(int key, double& value) const
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            value = cur->value;
            m_stats.totalFinds++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum
                / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalFinds
                   + m_stats.totalSplits + m_stats.totalMerges);
            return true;
        }
    }

    m_stats.totalFinds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalFinds
           + m_stats.totalSplits + m_stats.totalMerges);
    return false;
}

/** @brief 按阈值分裂 @param threshold 阈值 @param left 输出左树 @param right 输出右树 */
void TreapMap::split(int threshold, TreapMap& left, TreapMap& right)
{
    QElapsedTimer timer;
    timer.start();

    /* 清空目标树 */
    left.destroyTree(left.m_root);
    right.destroyTree(right.m_root);
    left.m_root = nullptr;
    right.m_root = nullptr;
    left.m_size = 0;
    right.m_size = 0;

    splitImpl(m_root, threshold, left.m_root, right.m_root);

    /* 更新大小 */
    left.m_size = left.m_root ? left.m_root->subtreeSize : 0;
    right.m_size = right.m_root ? right.m_root->subtreeSize : 0;

    m_root = nullptr;
    m_size = 0;

    m_stats.totalSplits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalFinds
           + m_stats.totalSplits + m_stats.totalMerges);
}

/** @brief 合并两棵树 @param left 左树 @param right 右树 @return 合并后的树 */
TreapMap* TreapMap::merge(TreapMap* left, TreapMap* right)
{
    TreapMap* result = new TreapMap(left->parent());
    result->m_root = result->mergeImpl(left->m_root, right->m_root);
    result->m_size = result->m_root ? result->m_root->subtreeSize : 0;

    left->m_root = nullptr;
    left->m_size = 0;
    right->m_root = nullptr;
    right->m_size = 0;

    result->m_stats.totalMerges++;
    return result;
}

/** @brief 递归中序遍历 @param node 节点 @param result 结果 */
void TreapMap::inorderImpl(Node* node, QVector<std::pair<int, double>>& result) const
{
    if (!node) return;
    inorderImpl(node->left, result);
    result.append({node->key, node->value});
    inorderImpl(node->right, result);
}

/** @brief 中序遍历 @return 按键排序的键值对列表 */
QVector<std::pair<int, double>> TreapMap::inorderTraversal() const
{
    QVector<std::pair<int, double>> result;
    result.reserve(m_size);
    inorderImpl(m_root, result);
    return result;
}

/** @brief 求高度 @return 高度 */
int TreapMap::height() const
{
    return heightImpl(m_root);
}

/** @brief 递归求高度 @param node 节点 @return 高度 */
int TreapMap::heightImpl(Node* node) const
{
    if (!node) return 0;
    int lh = heightImpl(node->left);
    int rh = heightImpl(node->right);
    return 1 + std::max(lh, rh);
}

/** @brief 获取第k小元素 @param k 排名(0-based) @param key 输出键 @param value 输出值 @return 是否成功 */
bool TreapMap::kthElement(int k, int& key, double& value) const
{
    if (k < 0 || k >= m_size) return false;

    Node* cur = m_root;
    while (cur) {
        int leftSize = cur->left ? cur->left->subtreeSize : 0;
        if (k < leftSize) {
            cur = cur->left;
        } else if (k == leftSize) {
            key = cur->key;
            value = cur->value;
            return true;
        } else {
            k -= leftSize + 1;
            cur = cur->right;
        }
    }
    return false;
}

/** @brief 清空树 */
void TreapMap::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 重置统计 */
void TreapMap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
