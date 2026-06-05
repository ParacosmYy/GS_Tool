/**
 * @file ScapegoatTree.cpp
 * @brief 替罪羊树实现 — 自平衡二叉搜索树
 */

#include "utils/scapegoat/ScapegoatTree.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ScapegoatTree::ScapegoatTree(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_alpha(0.7)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
ScapegoatTree::~ScapegoatTree()
{
    destroyTree(m_root);
}

/** @brief 插入键 @param key 键值 */
void ScapegoatTree::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    bool didRebuild = false;
    m_root = insertAndBalance(m_root, key, didRebuild);
    ++m_size;
    ++m_stats.totalInserts;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInserts;
}

/** @brief 插入并平衡 @param node 当前节点 @param key 键 @param rebuilt 是否发生重建 @return 新根 */
ScapegoatTree::Node* ScapegoatTree::insertAndBalance(Node* node, double key,
                                                      bool& rebuilt)
{
    if (node == nullptr) return new Node(key);

    if (key < node->key) {
        node->left = insertAndBalance(node->left, key, rebuilt);
    } else if (key > node->key) {
        node->right = insertAndBalance(node->right, key, rebuilt);
    } else {
        /* 重复键: 不插入但增加子树大小 */
        return node;
    }

    node->subSize = 1 + calcSize(node->left) + calcSize(node->right);

    /* 检查是否需要重建 */
    if (!rebuilt && isUnbalanced(node)) {
        node = rebuildSubtree(node);
        ++m_stats.totalRebuilds;
        rebuilt = true;
        emit rebuilt(node->subSize);
    }

    return node;
}

/** @brief 检查节点是否alpha不平衡 @param node 节点 @return 是否不平衡 */
bool ScapegoatTree::isUnbalanced(Node* node) const
{
    if (node == nullptr) return false;
    int leftSize = calcSize(node->left);
    int rightSize = calcSize(node->right);
    int total = node->subSize;
    if (total < 2) return false;

    double threshold = m_alpha * static_cast<double>(total);
    return static_cast<double>(leftSize) > threshold ||
           static_cast<double>(rightSize) > threshold;
}

/** @brief 计算子树大小 @param node 节点 @return 子树大小 */
int ScapegoatTree::calcSize(Node* node) const
{
    return (node == nullptr) ? 0 : node->subSize;
}

/** @brief 删除键 @param key 键值 */
void ScapegoatTree::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = removeNode(m_root, key);
    if (m_size < oldSize) {
        --m_size;
        /* 更新子树大小 */
        if (m_root) {
            m_root->subSize = 1 + calcSize(m_root->left) + calcSize(m_root->right);
        }
    }

    m_timeSum += timer.elapsed();
    if (m_stats.totalInserts > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInserts;
    }
}

/** @brief 删除节点 @param node 当前节点 @param key 键 @return 新根 */
ScapegoatTree::Node* ScapegoatTree::removeNode(Node* node, double key)
{
    if (node == nullptr) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        /* 找到要删除的节点 */
        if (node->left == nullptr) {
            Node* right = node->right;
            delete node;
            return right;
        }
        if (node->right == nullptr) {
            Node* left = node->left;
            delete node;
            return left;
        }
        /* 两个子节点: 用后继替换 */
        Node* successor = findMin(node->right);
        node->key = successor->key;
        node->right = removeNode(node->right, successor->key);
    }

    if (node) {
        node->subSize = 1 + calcSize(node->left) + calcSize(node->right);
    }
    return node;
}

/** @brief 找最小节点 @param node 根节点 @return 最小节点 */
ScapegoatTree::Node* ScapegoatTree::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/** @brief 查询键是否存在 @param key 键值 @return 是否存在 */
bool ScapegoatTree::contains(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            return true;
        }
    }
    return false;
}

/** @brief 中序遍历 @return 有序列表 */
QVector<double> ScapegoatTree::inOrder() const
{
    QVector<double> result;
    result.reserve(m_size);
    inOrderCollect(m_root, result);
    return result;
}

/** @brief 中序收集 @param node 节点 @param result 结果列表 */
void ScapegoatTree::inOrderCollect(Node* node, QVector<double>& result) const
{
    if (node == nullptr) return;
    inOrderCollect(node->left, result);
    result.append(node->key);
    inOrderCollect(node->right, result);
}

/** @brief 重建子树 @param node 根节点 @return 新根 */
ScapegoatTree::Node* ScapegoatTree::rebuildSubtree(Node* node)
{
    QVector<Node*> sorted;
    flatten(node, sorted);
    return buildBalanced(sorted, 0, sorted.size() - 1);
}

/** @brief 扁平化子树 @param node 节点 @param sorted 输出数组 */
void ScapegoatTree::flatten(Node* node, QVector<Node*>& sorted)
{
    if (node == nullptr) return;
    flatten(node->left, sorted);
    sorted.append(node);
    flatten(node->right, sorted);
    node->left = nullptr;
    node->right = nullptr;
}

/** @brief 从有序数组构建平衡树 @param sorted 有序数组 @param l 左边界 @param r 右边界 @return 根节点 */
ScapegoatTree::Node* ScapegoatTree::buildBalanced(const QVector<Node*>& sorted,
                                                    int l, int r)
{
    if (l > r) return nullptr;
    int mid = (l + r) / 2;
    Node* node = sorted[mid];
    node->left = buildBalanced(sorted, l, mid - 1);
    node->right = buildBalanced(sorted, mid + 1, r);
    node->subSize = 1 + calcSize(node->left) + calcSize(node->right);
    return node;
}

/** @brief 递归销毁 @param node 节点 */
void ScapegoatTree::destroyTree(Node* node)
{
    if (node == nullptr) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

/** @brief 重置统计 */
void ScapegoatTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
