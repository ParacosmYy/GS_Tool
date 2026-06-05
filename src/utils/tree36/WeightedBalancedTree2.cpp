/**
 * @file WeightedBalancedTree2.cpp
 * @brief 加权平衡树增强实现 — alpha权重重构/中序遍历/排名选择
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree36/WeightedBalancedTree2.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param alpha 平衡因子（0 < alpha < 0.5，典型值0.29）
 * @param parent 父对象
 */
WeightedBalancedTree2::WeightedBalancedTree2(double alpha, QObject* parent)
    : QObject(parent)
    , m_alpha(qBound(0.1, alpha, 0.49))
{
    setObjectName(QStringLiteral("WeightedBalancedTree2"));
}

/**
 * @brief 插入键值对
 *
 * 沿BST路径插入新节点(权重=1)，沿路径更新祖先权重。
 * 若任一祖先节点违反alpha平衡条件，则以该节点为根重建子树。
 *
 * @param key 查找键
 * @param value 关联值
 */
void WeightedBalancedTree2::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node{key, value, 1, 1, nullptr, nullptr, nullptr};

    if (!m_root) {
        m_root = newNode;
        m_size = 1;
        m_stats.totalInsertions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
        emit inserted(key);
        return;
    }

    /* 标准BST插入 */
    Node* cur = m_root;
    Node* par = nullptr;
    QVector<Node*> path;

    while (cur) {
        path.append(cur);
        par = cur;
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            /* 键已存在，更新值 */
            cur->value = value;
            delete newNode;
            m_stats.totalInsertions++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
            return;
        }
    }

    newNode->parent = par;
    if (key < par->key) {
        par->left = newNode;
    } else {
        par->right = newNode;
    }

    m_size++;

    /* 沿路径向上更新权重并检查平衡 */
    for (int i = path.size() - 1; i >= 0; --i) {
        updateWeights(path[i]);
        if (needsRebalance(path[i])) {
            /* 重建以path[i]为根的子树 */
            Node* subRoot = path[i];
            Node* parent = subRoot->parent;

            Node* rebuilt = rebuildSubtree(subRoot);
            rebuilt->parent = parent;

            if (!parent) {
                m_root = rebuilt;
            } else if (parent->left == subRoot) {
                parent->left = rebuilt;
            } else {
                parent->right = rebuilt;
            }

            /* 更新重建节点以上祖先的权重 */
            Node* p = parent;
            while (p) {
                updateWeights(p);
                p = p->parent;
            }
            break; /* 一次重建即可恢复全局平衡 */
        }
    }

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
    emit inserted(key);
}

/**
 * @brief 删除键
 *
 * BST删除后沿路径更新权重，检查平衡条件并按需重建。
 *
 * @param key 要删除的键
 * @return 是否成功删除
 */
bool WeightedBalancedTree2::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* 查找节点 */
    Node* node = m_root;
    QVector<Node*> path;

    while (node && node->key != key) {
        path.append(node);
        if (key < node->key) {
            node = node->left;
        } else {
            node = node->right;
        }
    }

    if (!node) {
        return false;
    }

    /* BST删除: 找到中序后继 */
    Node* toDelete = node;
    if (node->left && node->right) {
        Node* succ = node->right;
        path.append(node);
        while (succ->left) {
            path.append(succ);
            succ = succ->left;
        }
        node->key = succ->key;
        node->value = succ->value;
        toDelete = succ;
    }

    /* 移除toDelete（最多一个子节点） */
    Node* child = toDelete->left ? toDelete->left : toDelete->right;
    Node* parent = toDelete->parent;

    if (child) {
        child->parent = parent;
    }

    if (!parent) {
        m_root = child;
    } else if (parent->left == toDelete) {
        parent->left = child;
    } else {
        parent->right = child;
    }

    delete toDelete;
    m_size--;

    /* 沿路径更新权重并检查平衡 */
    for (int i = path.size() - 1; i >= 0; --i) {
        updateWeights(path[i]);
        if (needsRebalance(path[i])) {
            Node* subRoot = path[i];
            Node* par = subRoot->parent;

            Node* rebuilt = rebuildSubtree(subRoot);
            rebuilt->parent = par;

            if (!par) {
                m_root = rebuilt;
            } else if (par->left == subRoot) {
                par->left = rebuilt;
            } else {
                par->right = rebuilt;
            }

            Node* p = par;
            while (p) {
                updateWeights(p);
                p = p->parent;
            }
            break;
        }
    }

    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
    emit removed(key);
    return true;
}

/**
 * @brief 查找键对应的值
 * @param key 查找键
 * @return 关联值，未找到返回0
 */
int WeightedBalancedTree2::find(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            return cur->value;
        }
    }
    return 0;
}

/**
 * @brief 选择第k小的键（加权顺序统计量）
 *
 * 利用每个节点的totalWeight快速定位第k小元素。
 *
 * @param k 排名（0-indexed）
 * @return 第k小的键
 */
double WeightedBalancedTree2::selectKth(int k) const
{
    if (k < 0 || k >= m_size) return 0.0;

    Node* cur = m_root;
    while (cur) {
        int leftWeight = cur->left ? cur->left->totalWeight : 0;
        if (k < leftWeight) {
            cur = cur->left;
        } else if (k == leftWeight) {
            return cur->key;
        } else {
            k -= leftWeight + 1;
            cur = cur->right;
        }
    }
    return 0.0;
}

/**
 * @brief 计算键的排名
 * @param key 查找键
 * @return 排名（0-indexed），不存在返回-1
 */
int WeightedBalancedTree2::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += (cur->left ? cur->left->totalWeight : 0) + 1;
            cur = cur->right;
        } else {
            r += cur->left ? cur->left->totalWeight : 0;
            return r;
        }
    }
    return -1;
}

/**
 * @brief 清空整棵树
 */
void WeightedBalancedTree2::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 中序遍历
 * @return (键, 值) 有序向量
 */
QVector<QPair<double, int>> WeightedBalancedTree2::inOrderTraversal() const
{
    QVector<QPair<double, int>> result;
    result.reserve(m_size);

    /* 迭代中序遍历 */
    QVector<Node*> stack;
    Node* cur = m_root;
    while (cur || !stack.isEmpty()) {
        while (cur) {
            stack.append(cur);
            cur = cur->left;
        }
        cur = stack.takeLast();
        result.append({cur->key, cur->value});
        cur = cur->right;
    }
    return result;
}

/**
 * @brief 检查节点是否需要重建
 *
 * alpha平衡条件: 左/右子树的权重占比不超过 (1-alpha)，
 * 即子树权重 <= (1-alpha) * 父节点总权重。
 *
 * @param n 待检查节点
 * @return 是否违反平衡条件
 */
bool WeightedBalancedTree2::needsRebalance(Node* n) const
{
    if (!n) return false;
    int leftW = n->left ? n->left->totalWeight : 0;
    int rightW = n->right ? n->right->totalWeight : 0;
    int total = leftW + rightW + 1;
    if (total <= 2) return false;

    double leftRatio = static_cast<double>(leftW) / total;
    double rightRatio = static_cast<double>(rightW) / total;
    double limit = 1.0 - m_alpha;

    return (leftRatio > limit || rightRatio > limit);
}

/**
 * @brief 重建子树为完美平衡
 *
 * 中序展平 → 递归二分建树，保证重建后的子树完美平衡。
 *
 * @param node 子树根节点
 * @return 新的子树根节点
 */
WeightedBalancedTree2::Node* WeightedBalancedTree2::rebuildSubtree(Node* node)
{
    QVector<Node*> nodes;
    flatten(node, nodes);

    /* 断开所有父子关系 */
    for (auto* n : nodes) {
        n->left = nullptr;
        n->right = nullptr;
        n->parent = nullptr;
    }

    return buildBalanced(nodes, 0, nodes.size() - 1);
}

/**
 * @brief 中序展平子树为节点数组
 */
void WeightedBalancedTree2::flatten(Node* node, QVector<Node*>& nodes)
{
    if (!node) return;
    flatten(node->left, nodes);
    nodes.append(node);
    flatten(node->right, nodes);
}

/**
 * @brief 从有序节点数组递归构建平衡子树
 */
WeightedBalancedTree2::Node* WeightedBalancedTree2::buildBalanced(QVector<Node*>& nodes, int start, int end)
{
    if (start > end) return nullptr;

    int mid = start + (end - start) / 2;
    Node* root = nodes[mid];

    root->left = buildBalanced(nodes, start, mid - 1);
    if (root->left) root->left->parent = root;

    root->right = buildBalanced(nodes, mid + 1, end);
    if (root->right) root->right->parent = root;

    updateWeights(root);
    return root;
}

/**
 * @brief 更新节点的权重统计
 */
void WeightedBalancedTree2::updateWeights(Node* n)
{
    if (!n) return;
    n->weight = 1;
    n->totalWeight = 1;
    if (n->left) n->totalWeight += n->left->totalWeight;
    if (n->right) n->totalWeight += n->right->totalWeight;
}

/**
 * @brief 递归销毁子树
 */
void WeightedBalancedTree2::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/**
 * @brief 重置所有累积统计信息
 */
void WeightedBalancedTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
