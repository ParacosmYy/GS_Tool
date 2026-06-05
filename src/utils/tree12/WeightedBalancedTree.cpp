/**
 * @file WeightedBalancedTree.cpp
 * @brief 加权平衡树(BB[alpha])实现 — 动态维护节点权重的平衡二叉搜索树
 */

#include "utils/tree12/WeightedBalancedTree.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
WeightedBalancedTree::WeightedBalancedTree(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_count(0)
{
}

/** @brief 设置平衡因子alpha @param alpha 平衡因子 */
void WeightedBalancedTree::setAlpha(double alpha)
{
    m_alpha = qBound(0.5, alpha, 1.0);
}

/** @brief 插入键值对 @param key 键 @param weight 权重 */
void WeightedBalancedTree::insert(double key, double weight)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, weight);
    m_count++;

    m_stats.totalInsertions++;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
                              + m_stats.totalSearches);

    emit inserted(key, m_count);
}

/** @brief 删除键 @param key 键 @return 是否删除成功 */
bool WeightedBalancedTree::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* found = findNode(key);
    if (!found) return false;

    m_root = deleteNode(m_root, key);
    m_count--;

    m_stats.totalDeletions++;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions + m_stats.totalDeletions
                              + m_stats.totalSearches);
    return true;
}

/** @brief 搜索键 @param key 键 @return (是否存在, 权重) */
QPair<bool, double> WeightedBalancedTree::search(double key) const
{
    Node* node = findNode(key);
    m_stats.totalSearches++;
    if (node) {
        return {true, node->weight};
    }
    return {false, 0.0};
}

/** @brief 查询第k小的键(按权重排名) @param rank 排名 @return 键值 */
double WeightedBalancedTree::selectByRank(int rank) const
{
    if (rank < 1 || rank > m_count || !m_root) return qQNaN();

    Node* current = m_root;
    while (current) {
        int leftCount = current->left ? current->left->subCount : 0;
        if (rank <= leftCount) {
            current = current->left;
        } else if (rank == leftCount + 1) {
            return current->key;
        } else {
            rank -= leftCount + 1;
            current = current->right;
        }
    }
    return qQNaN();
}

/** @brief 查询键的排名 @param key 键 @return 排名 */
int WeightedBalancedTree::rankOf(double key) const
{
    if (!m_root) return -1;

    int rank = 0;
    Node* current = m_root;
    while (current) {
        if (key < current->key) {
            current = current->left;
        } else if (key > current->key) {
            int leftCount = current->left ? current->left->subCount : 0;
            rank += leftCount + 1;
            current = current->right;
        } else {
            int leftCount = current->left ? current->left->subCount : 0;
            return rank + leftCount + 1;
        }
    }
    return -1;
}

/** @brief 范围查询 @param lo 下界 @param hi 上界 @return (键, 权重)列表 */
QList<QPair<double, double>> WeightedBalancedTree::rangeQuery(double lo,
                                                               double hi) const
{
    QList<QPair<double, double>> result;
    if (!m_root) return result;

    /* 中序遍历收集 */
    std::function<void(Node*)> traverse = [&](Node* node) {
        if (!node) return;
        if (node->key > lo) traverse(node->left);
        if (node->key >= lo && node->key <= hi) {
            result.append({node->key, node->weight});
        }
        if (node->key < hi) traverse(node->right);
    };
    traverse(m_root);
    return result;
}

/** @brief 前驱: 小于key的最大键 @param key 键 @return 前驱键 */
double WeightedBalancedTree::predecessor(double key) const
{
    Node* current = m_root;
    Node* pred = nullptr;

    while (current) {
        if (current->key < key) {
            pred = current;
            current = current->right;
        } else {
            current = current->left;
        }
    }
    return pred ? pred->key : qQNaN();
}

/** @brief 后继: 大于key的最小键 @param key 键 @return 后继键 */
double WeightedBalancedTree::successor(double key) const
{
    Node* current = m_root;
    Node* succ = nullptr;

    while (current) {
        if (current->key > key) {
            succ = current;
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return succ ? succ->key : qQNaN();
}

/** @brief 树中元素总数 @return 元素数 */
int WeightedBalancedTree::size() const { return m_count; }

/** @brief 总权重 @return 权重和 */
double WeightedBalancedTree::totalWeight() const
{
    return m_root ? m_root->subWeight : 0.0;
}

/** @brief 树高度 @return 高度 */
int WeightedBalancedTree::height() const
{
    return treeHeight(m_root);
}

/** @brief 检查树是否平衡 @return 是否满足BB[alpha]条件 */
bool WeightedBalancedTree::isBalanced() const
{
    return checkBalance(m_root);
}

/** @brief 中序遍历 @return (键, 权重)列表 */
QList<QPair<double, double>> WeightedBalancedTree::inOrderTraversal() const
{
    QList<QPair<double, double>> result;
    std::function<void(Node*)> traverse = [&](Node* node) {
        if (!node) return;
        traverse(node->left);
        result.append({node->key, node->weight});
        traverse(node->right);
    };
    traverse(m_root);
    return result;
}

/** @brief 清空树 */
void WeightedBalancedTree::clear()
{
    freeTree(m_root);
    m_root = nullptr;
    m_count = 0;
}

/** @brief 重置统计 */
void WeightedBalancedTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 递归插入 */
WeightedBalancedTree::Node* WeightedBalancedTree::insertNode(Node* node, double key,
                                                              double weight)
{
    if (!node) {
        Node* n = new Node;
        n->key = key;
        n->weight = weight;
        return n;
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, weight);
        node->left->parent = node;
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, weight);
        node->right->parent = node;
    } else {
        /* 键已存在: 更新权重 */
        node->weight = weight;
        updateNode(node);
        return node;
    }

    updateNode(node);

    /* 检查是否需要再平衡 */
    if (!checkBalance(node)) {
        node = rebuildSubtree(node);
        m_stats.totalRebalances++;
        emit rebalanced(node->subCount, key);
    }
    return node;
}

/** @brief 递归删除 */
WeightedBalancedTree::Node* WeightedBalancedTree::deleteNode(Node* node, double key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = deleteNode(node->left, key);
    } else if (key > node->key) {
        node->right = deleteNode(node->right, key);
    } else {
        /* 找到节点 */
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        /* 找中序后继 */
        Node* succ = node->right;
        while (succ->left) succ = succ->left;
        node->key = succ->key;
        node->weight = succ->weight;
        node->right = deleteNode(node->right, succ->key);
    }

    updateNode(node);

    if (!checkBalance(node)) {
        node = rebuildSubtree(node);
        m_stats.totalRebalances++;
    }
    return node;
}

/** @brief 查找节点 */
WeightedBalancedTree::Node* WeightedBalancedTree::findNode(double key) const
{
    Node* current = m_root;
    while (current) {
        if (key < current->key) current = current->left;
        else if (key > current->key) current = current->right;
        else return current;
    }
    return nullptr;
}

/** @brief 全局重建子树 */
WeightedBalancedTree::Node* WeightedBalancedTree::rebuildSubtree(Node* node)
{
    QVector<Node*> nodes;
    collectInOrder(node, nodes);

    /* 清除父子指针 */
    for (auto* n : nodes) {
        n->left = nullptr;
        n->right = nullptr;
        n->parent = nullptr;
    }

    return buildBalanced(nodes, 0, nodes.size() - 1);
}

/** @brief 中序收集节点 */
void WeightedBalancedTree::collectInOrder(Node* node, QVector<Node*>& nodes)
{
    if (!node) return;
    collectInOrder(node->left, nodes);
    nodes.append(node);
    collectInOrder(node->right, nodes);
}

/** @brief 从有序数组构建平衡树 */
WeightedBalancedTree::Node* WeightedBalancedTree::buildBalanced(QVector<Node*>& nodes,
                                                                 int lo, int hi)
{
    if (lo > hi) return nullptr;

    int mid = (lo + hi) / 2;
    Node* node = nodes[mid];

    node->left = buildBalanced(nodes, lo, mid - 1);
    if (node->left) node->left->parent = node;

    node->right = buildBalanced(nodes, mid + 1, hi);
    if (node->right) node->right->parent = node;

    updateNode(node);
    return node;
}

/** @brief 更新节点聚合信息 */
void WeightedBalancedTree::updateNode(Node* node)
{
    if (!node) return;
    int leftCount = node->left ? node->left->subCount : 0;
    int rightCount = node->right ? node->right->subCount : 0;
    node->subCount = 1 + leftCount + rightCount;

    double leftWeight = node->left ? node->left->subWeight : 0.0;
    double rightWeight = node->right ? node->right->subWeight : 0.0;
    node->subWeight = node->weight + leftWeight + rightWeight;

    int leftH = node->left ? node->left->height : 0;
    int rightH = node->right ? node->right->height : 0;
    node->height = 1 + qMax(leftH, rightH);
}

/** @brief 检查BB[alpha]平衡条件 */
bool WeightedBalancedTree::checkBalance(Node* node) const
{
    if (!node) return true;
    int leftCount = node->left ? node->left->subCount : 0;
    int rightCount = node->right ? node->right->subCount : 0;
    int total = node->subCount;

    /* BB[alpha]: 左/右子树节点数 <= alpha * 总节点数 */
    if (leftCount > m_alpha * total || rightCount > m_alpha * total) {
        return false;
    }
    return checkBalance(node->left) && checkBalance(node->right);
}

/** @brief 递归释放 */
void WeightedBalancedTree::freeTree(Node* node)
{
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

/** @brief 递归计算高度 */
int WeightedBalancedTree::treeHeight(Node* node) const
{
    if (!node) return -1;
    return node->height - 1;
}
