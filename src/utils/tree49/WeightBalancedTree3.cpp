/**
 * @file WeightBalancedTree3.cpp
 * @brief 权重平衡树3 — BB[alpha]+加权旋转 实现
 *
 * 实现 BB[alpha] 权重平衡二叉搜索树。
 * 每个节点维护子树权重，插入/删除后检查平衡条件：
 *   alpha <= w(left)/w(node) <= 1 - alpha
 * 不平衡时通过单/双旋转恢复平衡。
 */

#include "utils/tree49/WeightBalancedTree3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param alpha 平衡参数，范围 (0, 0.5)，默认 0.288 (接近最优值)
 * @param parent 父QObject
 */
WeightBalancedTree3::WeightBalancedTree3(double alpha, QObject* parent)
    : QObject(parent)
    , m_alpha(qBound(0.1, alpha, 0.499))
{
}

/**
 * @brief 析构函数，释放所有节点
 */
WeightBalancedTree3::~WeightBalancedTree3()
{
    destroyTree(m_root);
}

/**
 * @brief 插入键值对
 *
 * 递归插入后沿路径检查 BB[alpha] 平衡条件，
 * 不满足时选择单旋转或双旋转重建平衡。
 *
 * @param key 键
 * @param value 值
 */
void WeightBalancedTree3::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);
    m_size++;

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalInsertions++;
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalInsertions + m_stats.totalDeletions);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键
 * @return 是否删除成功
 */
bool WeightBalancedTree3::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = removeNode(m_root, key);

    if (m_size < oldSize) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.totalDeletions++;
        m_stats.avgProcessingTimeMs = m_timeSum
            / (m_stats.totalInsertions + m_stats.totalDeletions);
        return true;
    }
    return false;
}

/**
 * @brief 查找键对应的值
 * @param key 查找的键
 * @return 对应值，未找到返回 0
 */
int WeightBalancedTree3::find(double key) const
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
 * @brief 查询键的排名（小于该键的元素数量）
 * @param key 查询的键
 * @return 排名，从 0 开始
 */
int WeightBalancedTree3::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur) {
        if (key <= cur->key) {
            cur = cur->left;
        } else {
            r += (cur->left ? cur->left->subtreeWeight : 0) + 1;
            cur = cur->right;
        }
    }
    return r;
}

/**
 * @brief 查找第 k 小的键
 * @param k 排名（从 0 开始）
 * @return 第 k 小的键
 */
double WeightBalancedTree3::kth(int k) const
{
    Node* cur = m_root;
    while (cur) {
        int leftW = cur->left ? cur->left->subtreeWeight : 0;
        if (k < leftW) {
            cur = cur->left;
        } else if (k == leftW) {
            return cur->key;
        } else {
            k -= leftW + 1;
            cur = cur->right;
        }
    }
    return 0.0; /* 未找到 */
}

/**
 * @brief 清空整棵树
 */
void WeightBalancedTree3::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 计算树的高度
 * @return 树的高度（空树为 0）
 */
int WeightBalancedTree3::height() const
{
    if (!m_root) return 0;

    int h = 0;
    QVector<Node*> level = {m_root};
    while (!level.isEmpty()) {
        h++;
        QVector<Node*> next;
        for (Node* n : level) {
            if (n->left) next.append(n->left);
            if (n->right) next.append(n->right);
        }
        level = next;
    }
    return h;
}

/**
 * @brief 中序遍历
 * @return 按键排序的键值对列表
 */
QVector<QPair<double, int>> WeightBalancedTree3::inOrderTraversal() const
{
    QVector<QPair<double, int>> result;
    result.reserve(m_size);

    /* 非递归中序遍历 */
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
 * @brief 重置统计信息
 */
void WeightBalancedTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 递归插入节点
 *
 * 插入后更新子树权重并检查平衡。
 * 不平衡时根据失衡方向选择旋转方式。
 *
 * @param node 当前子树根节点
 * @param key 插入键
 * @param value 插入值
 * @return 新的子树根节点
 */
WeightBalancedTree3::Node* WeightBalancedTree3::insertNode(Node* node, double key, int value)
{
    if (!node) {
        Node* n = new Node{key, value, 1, 1, nullptr, nullptr};
        return n;
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, value);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, value);
    } else {
        /* 键已存在，更新值 */
        node->value = value;
        m_size--; /* 抵消外层的 m_size++ */
        return node;
    }

    /* 更新子树权重 */
    node->subtreeWeight = 1
        + (node->left ? node->left->subtreeWeight : 0)
        + (node->right ? node->right->subtreeWeight : 0);

    return balance(node);
}

/**
 * @brief 递归删除节点
 * @param node 当前子树根节点
 * @param key 要删除的键
 * @return 新的子树根节点
 */
WeightBalancedTree3::Node* WeightBalancedTree3::removeNode(Node* node, double key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        /* 找到要删除的节点 */
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            m_size--;
            return child;
        }
        /* 两个孩子：用中序后继替换 */
        Node* succ = node->right;
        while (succ->left) succ = succ->left;
        node->key = succ->key;
        node->value = succ->value;
        node->right = removeNode(node->right, succ->key);
    }

    /* 更新子树权重 */
    node->subtreeWeight = 1
        + (node->left ? node->left->subtreeWeight : 0)
        + (node->right ? node->right->subtreeWeight : 0);

    return balance(node);
}

/**
 * @brief 检查并恢复 BB[alpha] 平衡
 *
 * 计算左右子树的权重比例，若超出 [alpha, 1-alpha] 范围则旋转。
 * 左重时右旋，右重时左旋。判断是否需要双旋转。
 *
 * @param node 当前节点
 * @return 平衡后的子树根
 */
WeightBalancedTree3::Node* WeightBalancedTree3::balance(Node* node)
{
    if (!node) return nullptr;

    int leftW = node->left ? node->left->subtreeWeight : 0;
    int rightW = node->right ? node->right->subtreeWeight : 0;
    int total = leftW + rightW + 1;

    if (total <= 2) return node; /* 小子树无需平衡 */

    double leftRatio = static_cast<double>(leftW) / total;
    double rightRatio = static_cast<double>(rightW) / total;

    if (leftRatio > 1.0 - m_alpha) {
        /* 左子树过重 */
        int leftLeftW = node->left->left ? node->left->left->subtreeWeight : 0;
        double innerRatio = static_cast<double>(leftLeftW) / qMax(1, leftW);

        if (innerRatio < m_alpha) {
            /* 双旋转：先左旋左子树，再右旋 */
            node->left = rotateLeft(node->left);
            m_stats.totalRotations++;
        }
        node = rotateRight(node);
        m_stats.totalRotations++;

        emit rebalanceCompleted(m_stats.totalRotations, m_size);
    } else if (rightRatio > 1.0 - m_alpha) {
        /* 右子树过重 */
        int rightRightW = node->right->right ? node->right->right->subtreeWeight : 0;
        double innerRatio = static_cast<double>(rightRightW) / qMax(1, rightW);

        if (innerRatio < m_alpha) {
            /* 双旋转：先右旋右子树，再左旋 */
            node->right = rotateRight(node->right);
            m_stats.totalRotations++;
        }
        node = rotateLeft(node);
        m_stats.totalRotations++;

        emit rebalanceCompleted(m_stats.totalRotations, m_size);
    }

    return node;
}

/**
 * @brief 左旋转
 * @param x 旋转根节点
 * @return 旋转后的新根节点
 */
WeightBalancedTree3::Node* WeightBalancedTree3::rotateLeft(Node* x)
{
    if (!x || !x->right) return x;
    Node* y = x->right;
    x->right = y->left;
    y->left = x;

    /* 更新权重 */
    x->subtreeWeight = 1
        + (x->left ? x->left->subtreeWeight : 0)
        + (x->right ? x->right->subtreeWeight : 0);
    y->subtreeWeight = 1
        + (y->left ? y->left->subtreeWeight : 0)
        + (y->right ? y->right->subtreeWeight : 0);

    return y;
}

/**
 * @brief 右旋转
 * @param y 旋转根节点
 * @return 旋转后的新根节点
 */
WeightBalancedTree3::Node* WeightBalancedTree3::rotateRight(Node* y)
{
    if (!y || !y->left) return y;
    Node* x = y->left;
    y->left = x->right;
    x->right = y;

    /* 更新权重 */
    y->subtreeWeight = 1
        + (y->left ? y->left->subtreeWeight : 0)
        + (y->right ? y->right->subtreeWeight : 0);
    x->subtreeWeight = 1
        + (x->left ? x->left->subtreeWeight : 0)
        + (x->right ? x->right->subtreeWeight : 0);

    return x;
}

/**
 * @brief 从有序序列构建平衡树（用于批量重建）
 * @param items 有序键值对
 * @param lo 起始索引
 * @param hi 结束索引（不含）
 * @return 子树根节点
 */
WeightBalancedTree3::Node* WeightBalancedTree3::buildBalanced(
    const QVector<QPair<double, int>>& items, int lo, int hi)
{
    if (lo >= hi) return nullptr;
    int mid = (lo + hi) / 2;

    Node* node = new Node{items[mid].first, items[mid].second, 1, 1, nullptr, nullptr};
    node->left = buildBalanced(items, lo, mid);
    node->right = buildBalanced(items, mid + 1, hi);
    node->subtreeWeight = 1
        + (node->left ? node->left->subtreeWeight : 0)
        + (node->right ? node->right->subtreeWeight : 0);
    return node;
}

/**
 * @brief 递归销毁子树
 * @param n 子树根节点
 */
void WeightBalancedTree3::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}
