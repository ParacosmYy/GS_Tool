/**
 * @file ScapegoatTree2.cpp
 * @brief 替罪羊树实现 — 带重建统计的自平衡BST
 */

#include "utils/tree13/ScapegoatTree2.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param alpha 不平衡阈值 @param parent 父对象 */
ScapegoatTree2::ScapegoatTree2(double alpha, QObject* parent)
    : QObject(parent)
    , m_alpha(qBound(0.5, alpha, 1.0))
{
}

/**
 * @brief 插入键值对
 * 标准BST插入后检查路径上的祖先是否不平衡
 */
bool ScapegoatTree2::insert(double key, double value)
{
    m_timer.start();

    /* 插入到BST */
    Node** ptr = &m_root;
    QVector<Node**> path; /* 记录路径用于查找替罪羊 */

    while (*ptr) {
        path.append(ptr);
        if (key < (*ptr)->key) {
            ptr = &(*ptr)->left;
        } else if (key > (*ptr)->key) {
            ptr = &(*ptr)->right;
        } else {
            /* 键已存在 */
            m_timeSum += m_timer.elapsed();
            return false;
        }
    }

    /* 创建新节点 */
    *ptr = new Node{key, value, nullptr, nullptr};
    ++m_size;
    m_maxSize = qMax(m_maxSize, m_size);

    /* 查找替罪羊: 从叶子向上检查不平衡度 */
    bool didRebuild = false;
    int triggerDepth = 0;

    for (int i = path.size() - 1; i >= 0; --i) {
        Node* candidate = *path[i];
        int leftSz = subtreeSize(candidate->left);
        int rightSz = subtreeSize(candidate->right);
        int total = leftSz + rightSz + 1;

        if (isUnbalanced(candidate, leftSz, rightSz)) {
            triggerDepth = path.size() - i;

            /* 记录重建事件 */
            RebuildEvent event;
            event.triggerKey = key;
            event.subtreeSize = total;
            event.depth = triggerDepth;
            event.wasInsert = true;
            m_rebuildHistory.append(event);

            /* 重建子树 */
            *path[i] = rebuildSubtree(candidate);
            didRebuild = true;

            ++m_stats.totalRebuilds;
            m_stats.totalRebuiltNodes += total;

            emit rebuildTriggered(total, triggerDepth);
            break;
        }
    }

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalInserts;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInserts);

    emit insertCompleted(key, didRebuild);
    return true;
}

/**
 * @brief 删除键
 * 惰性删除: 直接移除节点，不立即重建
 * 当 m_size < m_alpha * m_maxSize 时触发全局重建
 */
bool ScapegoatTree2::remove(double key)
{
    m_timer.start();

    Node** ptr = &m_root;
    while (*ptr) {
        if (key < (*ptr)->key) {
            ptr = &(*ptr)->left;
        } else if (key > (*ptr)->key) {
            ptr = &(*ptr)->right;
        } else {
            /* 找到节点 */
            Node* target = *ptr;

            if (!target->left) {
                *ptr = target->right;
                delete target;
            } else if (!target->right) {
                *ptr = target->left;
                delete target;
            } else {
                /* 找中序后继 */
                Node** succPtr = &target->right;
                while ((*succPtr)->left) {
                    succPtr = &(*succPtr)->left;
                }
                Node* succ = *succPtr;

                /* 替换数据 */
                target->key = succ->key;
                target->value = succ->value;

                /* 删除后继 */
                *succPtr = succ->right;
                delete succ;
            }

            --m_size;

            /* 全局重建条件: m_size < alpha * m_maxSize */
            if (m_size > 0 && m_maxSize > 0 &&
                static_cast<double>(m_size) < m_alpha * m_maxSize) {
                RebuildEvent event;
                event.triggerKey = key;
                event.subtreeSize = m_size;
                event.depth = 0;
                event.wasInsert = false;
                m_rebuildHistory.append(event);

                m_root = rebuildSubtree(m_root);
                m_maxSize = m_size;

                ++m_stats.totalRebuilds;
                m_stats.totalRebuiltNodes += m_size;
                emit rebuildTriggered(m_size, 0);
            }

            m_timeSum += m_timer.elapsed();
            ++m_stats.totalDeletes;
            m_stats.avgProcessingTimeMs = m_timeSum
                / static_cast<double>(m_stats.totalDeletes);

            emit removeCompleted(key);
            return true;
        }
    }

    m_timeSum += m_timer.elapsed();
    return false;
}

/**
 * @brief 查找键对应的值
 */
bool ScapegoatTree2::find(double key, double& value) const
{
    return findHelper(m_root, key, value);
}

/**
 * @brief 范围查询 [lo, hi]
 */
QVector<QPair<double, double>> ScapegoatTree2::rangeQuery(double lo,
                                                           double hi) const
{
    QVector<QPair<double, double>> result;
    rangeHelper(m_root, lo, hi, result);
    return result;
}

/**
 * @brief 中序遍历
 */
void ScapegoatTree2::inorderTraversal(
    const std::function<void(double, double)>& visitor) const
{
    if (!m_root) return;

    /* 迭代式中序遍历(使用栈模拟递归) */
    QVector<Node*> stack;
    Node* current = m_root;

    while (current || !stack.empty()) {
        while (current) {
            stack.append(current);
            current = current->left;
        }

        current = stack.back();
        stack.pop_back();

        visitor(current->key, current->value);
        current = current->right;
    }
}

/** @brief 重置统计 */
void ScapegoatTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_rebuildHistory.clear();
}

/**
 * @brief 计算子树大小
 */
int ScapegoatTree2::subtreeSize(Node* node) const
{
    if (!node) return 0;
    return 1 + subtreeSize(node->left) + subtreeSize(node->right);
}

/**
 * @brief 判断是否不平衡
 * 当左子树或右子树的大小 > alpha * 总大小时不平衡
 */
bool ScapegoatTree2::isUnbalanced(Node* node, int leftSize,
                                    int rightSize) const
{
    Q_UNUSED(node);
    int total = leftSize + rightSize + 1;
    if (total <= 1) return false;

    double threshold = m_alpha * static_cast<double>(total);
    return static_cast<double>(leftSize) > threshold ||
           static_cast<double>(rightSize) > threshold;
}

/**
 * @brief 查找替罪羊节点(沿路径查找最浅的不平衡祖先)
 */
ScapegoatTree2::Node* ScapegoatTree2::findScapegoat(Node* root, double key,
                                                      int& outDepth) const
{
    Q_UNUSED(root);
    Q_UNUSED(key);
    outDepth = 0;
    /* 实际逻辑已在insert中直接实现 */
    return nullptr;
}

/**
 * @brief 重建子树为完美平衡
 * 中序收集 -> 中间分割构建平衡树
 */
ScapegoatTree2::Node* ScapegoatTree2::rebuildSubtree(Node* root)
{
    QVector<Node*> nodes;
    collectInorder(root, nodes);

    /* 断开所有连接(不释放节点) */
    for (Node* n : nodes) {
        n->left = nullptr;
        n->right = nullptr;
    }

    return buildBalanced(nodes, 0, static_cast<int>(nodes.size()) - 1);
}

/**
 * @brief 中序收集节点到数组
 */
void ScapegoatTree2::collectInorder(Node* node, QVector<Node*>& nodes) const
{
    if (!node) return;
    collectInorder(node->left, nodes);
    nodes.append(node);
    collectInorder(node->right, nodes);
}

/**
 * @brief 从有序数组构建平衡子树
 * 中间元素作为根，递归构建左右子树
 */
ScapegoatTree2::Node* ScapegoatTree2::buildBalanced(
    const QVector<Node*>& nodes, int start, int end)
{
    if (start > end) return nullptr;

    int mid = start + (end - start) / 2;
    Node* root = nodes[mid];

    root->left = buildBalanced(nodes, start, mid - 1);
    root->right = buildBalanced(nodes, mid + 1, end);

    return root;
}

/**
 * @brief 递归释放子树
 */
void ScapegoatTree2::freeSubtree(Node* node)
{
    if (!node) return;
    freeSubtree(node->left);
    freeSubtree(node->right);
    delete node;
}

/**
 * @brief 递归查找
 */
bool ScapegoatTree2::findHelper(Node* node, double key, double& value) const
{
    while (node) {
        if (key < node->key) {
            node = node->left;
        } else if (key > node->key) {
            node = node->right;
        } else {
            value = node->value;
            return true;
        }
    }
    return false;
}

/**
 * @brief 递归范围查询
 */
void ScapegoatTree2::rangeHelper(Node* node, double lo, double hi,
                                  QVector<QPair<double, double>>& result) const
{
    if (!node) return;

    if (lo < node->key) {
        rangeHelper(node->left, lo, hi, result);
    }

    if (node->key >= lo && node->key <= hi) {
        result.append({node->key, node->value});
    }

    if (hi > node->key) {
        rangeHelper(node->right, lo, hi, result);
    }
}
