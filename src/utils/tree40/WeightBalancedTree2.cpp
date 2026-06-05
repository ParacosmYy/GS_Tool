/**
 * @file WeightBalancedTree2.cpp
 * @brief BB[alpha]平衡树实现 - 基于权重的自适应重建
 *
 * 每个节点维护子树的总权重(weight = 1 + left.weight + right.weight)，
 * 当任一子树权重占总权重比例超过alpha时触发重建。
 * 使用中序遍历收集节点后递归构建完美平衡子树。
 */

#include "utils/tree40/WeightBalancedTree2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空树
 * @param alpha 平衡参数(0.25~0.35，超出范围会被限制)
 * @param parent 父QObject
 *
 * alpha控制平衡严格程度:
 * - 接近0.25: 几乎完全平衡(类似AVL)
 * - 接近0.5: 宽松平衡(类似红黑树)
 */
WeightBalancedTree2::WeightBalancedTree2(double alpha, QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_alpha(qBound(0.25, alpha, 0.45))
{
}

/**
 * @brief 释放节点
 * @param n 要销毁的子树根节点
 */
void WeightBalancedTree2::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/**
 * @brief 更新节点的totalWeight字段
 * @param n 目标节点
 */
static void updateWeight(WeightBalancedTree2::Node* n)
{
    if (!n) return;
    n->totalWeight = n->weight;
    if (n->left) n->totalWeight += n->left->totalWeight;
    if (n->right) n->totalWeight += n->right->totalWeight;
}

/**
 * @brief 检查并重建不平衡的子树
 *
 * 判断条件: 任一子树权重 / 当前节点总权重 > alpha
 * 如果不平衡，通过中序遍历收集节点后重建完美平衡子树。
 *
 * @param n 子树根节点
 * @return 重建后的子树根节点
 */
WeightBalancedTree2::Node* WeightBalancedTree2::rebalance(Node* n)
{
    if (!n) return nullptr;

    int leftW = n->left ? n->left->totalWeight : 0;
    int rightW = n->right ? n->right->totalWeight : 0;
    int totalW = n->totalWeight;

    /* 检查是否需要重建 */
    bool needRebuild = false;
    if (totalW > 2) {
        if (leftW > m_alpha * totalW || rightW > m_alpha * totalW)
            needRebuild = true;
    }

    if (!needRebuild) return n;

    /* 中序遍历收集节点 */
    QVector<Node*> nodes;
    nodes.reserve(totalW);

    /* 非递归中序遍历 */
    QVector<Node*> stack;
    Node* cur = n;
    while (cur || !stack.isEmpty()) {
        while (cur) {
            stack.push_back(cur);
            cur = cur->left;
        }
        cur = stack.takeLast();
        nodes.push_back(cur);
        cur = cur->right;
    }

    /* 递归构建完美平衡子树 */
    std::function<Node*(int, int)> buildBalanced = [&](int lo, int hi) -> Node* {
        if (lo > hi) return static_cast<Node*>(nullptr);

        int mid = (lo + hi) / 2;
        Node* root = nodes[mid];
        root->left = buildBalanced(lo, mid - 1);
        root->right = buildBalanced(mid + 1, hi);

        if (root->left) root->left->parent = root;
        if (root->right) root->right->parent = root;

        updateWeight(root);
        return root;
    };

    Node* newRoot = buildBalanced(0, nodes.size() - 1);
    if (newRoot) newRoot->parent = n->parent;
    return newRoot;
}

/**
 * @brief 插入键值对
 *
 * 标准BST插入后沿路径向上更新权重并检查平衡。
 *
 * @param key 键
 * @param value 值
 */
void WeightBalancedTree2::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{key, value, 1, 1, nullptr, nullptr, nullptr};
        m_size = 1;
        m_stats.totalInsertions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
        emit inserted(key);
        return;
    }

    /* 查找插入位置并记录路径 */
    QVector<Node*> path;
    Node* cur = m_root;
    while (cur) {
        path.push_back(cur);
        if (key < cur->key) {
            if (cur->left) cur = cur->left;
            else {
                cur->left = new Node{key, value, 1, 1, nullptr, nullptr, cur};
                break;
            }
        } else if (key > cur->key) {
            if (cur->right) cur = cur->right;
            else {
                cur->right = new Node{key, value, 1, 1, nullptr, nullptr, cur};
                break;
            }
        } else {
            /* 键已存在，更新值 */
            cur->value = value;
            m_stats.totalInsertions++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
            return;
        }
    }

    m_size++;

    /* 沿路径向上更新权重并检查平衡 */
    for (int i = path.size() - 1; i >= 0; --i) {
        updateWeight(path[i]);
    }

    /* 从根开始重建(简化: 全局重建路径上不平衡的节点) */
    Node* parent = nullptr;
    Node** link = &m_root;
    for (int i = 0; i < path.size(); ++i) {
        int leftW = path[i]->left ? path[i]->left->totalWeight : 0;
        int rightW = path[i]->right ? path[i]->right->totalWeight : 0;
        int totalW = path[i]->totalWeight;

        if (totalW > 2 && (leftW > m_alpha * totalW || rightW > m_alpha * totalW)) {
            Node* rebuilt = rebalance(path[i]);
            *link = rebuilt;
        }

        parent = *link;
        if (key < parent->key) {
            link = &parent->left;
        } else {
            link = &parent->right;
        }
    }

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);

    emit inserted(key);
}

/**
 * @brief 删除键
 * @param key 要删除的键
 * @return true成功删除，false键不存在
 */
bool WeightBalancedTree2::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* 查找节点 */
    Node* cur = m_root;
    Node* parent = nullptr;
    bool isLeft = false;

    while (cur) {
        if (key < cur->key) {
            parent = cur;
            isLeft = true;
            cur = cur->left;
        } else if (key > cur->key) {
            parent = cur;
            isLeft = false;
            cur = cur->right;
        } else {
            break;
        }
    }

    if (!cur) {
        m_stats.totalDeletions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
        return false;
    }

    /* 删除节点 */
    if (!cur->left && !cur->right) {
        /* 叶节点 */
        if (parent) {
            if (isLeft) parent->left = nullptr;
            else parent->right = nullptr;
        } else {
            m_root = nullptr;
        }
        delete cur;
    } else if (!cur->left || !cur->right) {
        /* 单子节点 */
        Node* child = cur->left ? cur->left : cur->right;
        child->parent = parent;
        if (parent) {
            if (isLeft) parent->left = child;
            else parent->right = child;
        } else {
            m_root = child;
        }
        delete cur;
    } else {
        /* 双子节点: 找中序后继 */
        Node* succ = cur->right;
        Node* succParent = cur;
        while (succ->left) {
            succParent = succ;
            succ = succ->left;
        }

        /* 用后继替换当前节点 */
        cur->key = succ->key;
        cur->value = succ->value;

        /* 删除后继 */
        if (succParent == cur) {
            succParent->right = succ->right;
        } else {
            succParent->left = succ->right;
        }
        if (succ->right) succ->right->parent = succParent;
        delete succ;
    }

    m_size--;

    /* 更新权重并重建 */
    if (m_root) {
        /* 简化: 从根开始全局重建 */
        m_root->parent = nullptr;
        QVector<Node*> path;
        /* 更新所有节点的权重 */
        std::function<void(Node*)> updateAll = [&](Node* n) {
            if (!n) return;
            updateAll(n->left);
            updateAll(n->right);
            updateWeight(n);
        };
        updateAll(m_root);

        /* 检查根是否需要重建 */
        m_root = rebalance(m_root);
        if (m_root) m_root->parent = nullptr;
    }

    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);

    emit removed(key);
    return true;
}

/**
 * @brief 查找键对应的值
 * @param key 要查找的键
 * @return 对应的值，未找到返回0
 */
int WeightBalancedTree2::find(double key) const
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else {
            const_cast<WeightBalancedTree2*>(this)->m_stats.totalQueries++;
            const_cast<WeightBalancedTree2*>(this)->m_timeSum += timer.elapsed();
            const_cast<WeightBalancedTree2*>(this)->m_stats.avgProcessingTimeMs =
                m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
            return cur->value;
        }
    }

    const_cast<WeightBalancedTree2*>(this)->m_stats.totalQueries++;
    const_cast<WeightBalancedTree2*>(this)->m_timeSum += timer.elapsed();
    const_cast<WeightBalancedTree2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);
    return 0;
}

/**
 * @brief 选择第k小的键
 * @param k 排名(从0开始)
 * @return 第k小的键值
 */
double WeightBalancedTree2::selectKth(int k) const
{
    if (k < 0 || k >= m_size) return 0.0;

    Node* cur = m_root;
    while (cur) {
        int leftW = cur->left ? cur->left->totalWeight : 0;
        if (k < leftW) {
            cur = cur->left;
        } else if (k == leftW) {
            return cur->key;
        } else {
            k -= leftW + 1;
            cur = cur->right;
        }
    }
    return 0.0;
}

/**
 * @brief 计算键的排名
 * @param key 要查询的键
 * @return 小于key的元素个数
 */
int WeightBalancedTree2::rank(double key) const
{
    Node* cur = m_root;
    int r = 0;

    while (cur) {
        int leftW = cur->left ? cur->left->totalWeight : 0;
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += leftW + 1;
            cur = cur->right;
        } else {
            r += leftW;
            break;
        }
    }
    return r;
}

/**
 * @brief 清空整棵树
 */
void WeightBalancedTree2::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 中序遍历获取所有键值对(有序)
 * @return 按键排序的键值对列表
 */
QVector<QPair<double, int>> WeightBalancedTree2::inOrderTraversal() const
{
    QVector<QPair<double, int>> result;
    result.reserve(m_size);

    /* 非递归中序遍历 */
    QVector<Node*> stack;
    Node* cur = m_root;
    while (cur || !stack.isEmpty()) {
        while (cur) {
            stack.push_back(cur);
            cur = cur->left;
        }
        cur = stack.takeLast();
        result.append({cur->key, cur->value});
        cur = cur->right;
    }

    return result;
}

/**
 * @brief 重置所有统计数据
 */
void WeightBalancedTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
