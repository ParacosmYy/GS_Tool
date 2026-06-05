/**
 * @file Treap4.cpp
 * @brief Treap4实现 — 持久化Treap+可分裂合并
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree44/Treap4.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <random>
#include <limits>

/**
 * @brief 构造函数，初始化空Treap
 * @param parent 父对象
 */
Treap4::Treap4(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("Treap4"));
}

/**
 * @brief 析构函数，释放所有节点
 */
Treap4::~Treap4()
{
    destroyTree(m_root);
}

/**
 * @brief 插入键值对
 *
 * 标准Treap插入：按BST规则找到位置，
 * 然后通过旋转维护堆性质。
 *
 * @param key 键
 * @param value 值
 */
void Treap4::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);

    m_stats.totalInsertions++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalInsertions + m_stats.totalDeletions +
                m_stats.totalSplits + m_stats.totalMerges);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键
 * @return 是否成功删除
 */
bool Treap4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = removeNode(m_root, key);

    m_stats.totalDeletions++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalInsertions + m_stats.totalDeletions +
                m_stats.totalSplits + m_stats.totalMerges);

    return m_size < oldSize;
}

/**
 * @brief 查找指定键的值
 * @param key 目标键
 * @return 对应值（未找到返回-1）
 */
int Treap4::find(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur->value;
    }
    return -1;
}

/**
 * @brief 按键值分裂Treap为两棵子树
 *
 * 所有 <= key 的节点进入左子树，> key 的进入右子树。
 * 返回两个新的Treap对象，调用者负责管理生命周期。
 *
 * @param key 分裂键
 * @return QPair<左Treap, 右Treap>
 */
QPair<Treap4*, Treap4*> Treap4::split(double key)
{
    QElapsedTimer timer;
    timer.start();

    auto [leftRoot, rightRoot] = splitNode(m_root, key);

    Treap4* leftTree = new Treap4(parent());
    leftTree->m_root = leftRoot;
    leftTree->m_size = 0;
    /* 计算大小 */
    std::function<void(Node*)> countSize = [&](Node* n) {
        if (!n) return;
        leftTree->m_size++;
        countSize(n->left);
        countSize(n->right);
    };
    countSize(leftRoot);

    Treap4* rightTree = new Treap4(parent());
    rightTree->m_root = rightRoot;
    rightTree->m_size = m_size - leftTree->m_size;

    m_root = nullptr;
    m_size = 0;

    m_stats.totalSplits++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalInsertions + m_stats.totalDeletions +
                m_stats.totalSplits + m_stats.totalMerges);

    emit splitCompleted(leftTree->m_size, rightTree->m_size);
    return {leftTree, rightTree};
}

/**
 * @brief 合并两棵Treap
 *
 * 要求left中所有键 <= right中所有键。
 * 通过优先级决定根节点，递归合并。
 *
 * @param left 左Treap（所有键较小）
 * @param right 右Treap（所有键较大）
 * @return 合并后的Treap
 */
Treap4* Treap4::merge(Treap4* left, Treap4* right)
{
    if (!left && !right) return nullptr;

    Treap4* result = new Treap4(left ? left->parent() : (right ? right->parent() : nullptr));
    result->m_root = result->mergeNode(left ? left->m_root : nullptr,
                                         right ? right->m_root : nullptr);
    result->m_size = (left ? left->m_size : 0) + (right ? right->m_size : 0);

    result->m_stats.totalMerges++;

    emit result->mergeCompleted(result->m_size);
    return result;
}

/**
 * @brief 清空树
 */
void Treap4::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 查找第k小的键
 * @param k 排名（0-indexed）
 * @return 第k小的键
 */
double Treap4::kth(int k) const
{
    if (k < 0 || k >= m_size) return std::numeric_limits<double>::quiet_NaN();

    Node* cur = m_root;
    while (cur) {
        int leftSize = cur->left ? cur->left->subtreeSize : 0;
        if (k < leftSize) {
            cur = cur->left;
        } else if (k == leftSize) {
            return cur->key;
        } else {
            k -= leftSize + 1;
            cur = cur->right;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

/**
 * @brief 计算指定键的排名（0-indexed）
 * @param key 目标键
 * @return 排名
 */
int Treap4::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += 1 + (cur->left ? cur->left->subtreeSize : 0);
            cur = cur->right;
        } else {
            r += cur->left ? cur->left->subtreeSize : 0;
            return r;
        }
    }
    return r;
}

/**
 * @brief 中序遍历获取所有键值对
 * @return 按键升序排列的键值对列表
 */
QVector<QPair<double,int>> Treap4::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    std::function<void(Node*)> traverse = [&](Node* node) {
        if (!node) return;
        traverse(node->left);
        result.append({node->key, node->value});
        traverse(node->right);
    };
    traverse(m_root);
    return result;
}

/**
 * @brief 插入节点（递归）
 *
 * 按BST规则插入，如果新节点优先级高于父节点则旋转提升。
 */
Treap4::Node* Treap4::insertNode(Node* root, double key, int value)
{
    if (!root) {
        m_size++;
        static std::mt19937 rng(42);
        return new Node{key, value, (int)(rng() & 0x7FFFFFFF), 1, nullptr, nullptr};
    }

    if (key < root->key) {
        root->left = insertNode(root->left, key, value);
        if (root->left && root->left->priority > root->priority) {
            root = rotateRight(root);
        }
    } else if (key > root->key) {
        root->right = insertNode(root->right, key, value);
        if (root->right && root->right->priority > root->priority) {
            root = rotateLeft(root);
        }
    } else {
        root->value = value;
    }

    updateSize(root);
    return root;
}

/**
 * @brief 删除节点（递归）
 *
 * 将目标节点旋转到叶位置后删除。
 */
Treap4::Node* Treap4::removeNode(Node* root, double key)
{
    if (!root) return nullptr;

    if (key < root->key) {
        root->left = removeNode(root->left, key);
    } else if (key > root->key) {
        root->right = removeNode(root->right, key);
    } else {
        m_size--;
        if (!root->left && !root->right) {
            delete root;
            return nullptr;
        }
        if (!root->left) {
            Node* r = root->right;
            delete root;
            return r;
        }
        if (!root->right) {
            Node* l = root->left;
            delete root;
            return l;
        }

        /* 旋转优先级较高的子节点上来 */
        if (root->left->priority > root->right->priority) {
            root = rotateRight(root);
            root->right = removeNode(root->right, key);
        } else {
            root = rotateLeft(root);
            root->left = removeNode(root->left, key);
        }
    }

    updateSize(root);
    return root;
}

/**
 * @brief 分裂操作（递归）
 *
 * 按key将树分为两部分：左树 <= key，右树 > key。
 */
QPair<Treap4::Node*, Treap4::Node*> Treap4::splitNode(Node* root, double key)
{
    if (!root) return {nullptr, nullptr};

    if (key < root->key) {
        auto [ll, lr] = splitNode(root->left, key);
        root->left = lr;
        updateSize(root);
        return {ll, root};
    } else {
        auto [rl, rr] = splitNode(root->right, key);
        root->right = rl;
        updateSize(root);
        return {root, rr};
    }
}

/**
 * @brief 合并操作（递归）
 *
 * 要求left的最大键 <= right的最小键。
 * 通过优先级决定根节点。
 */
Treap4::Node* Treap4::mergeNode(Node* left, Node* right)
{
    if (!left) return right;
    if (!right) return left;

    if (left->priority > right->priority) {
        left->right = mergeNode(left->right, right);
        updateSize(left);
        return left;
    } else {
        right->left = mergeNode(left, right->left);
        updateSize(right);
        return right;
    }
}

/**
 * @brief 更新子树大小
 */
void Treap4::updateSize(Node* n)
{
    if (!n) return;
    n->subtreeSize = 1;
    if (n->left) n->subtreeSize += n->left->subtreeSize;
    if (n->right) n->subtreeSize += n->right->subtreeSize;
}

/**
 * @brief 左旋
 */
Treap4::Node* Treap4::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    updateSize(x);
    updateSize(y);
    return y;
}

/**
 * @brief 右旋
 */
Treap4::Node* Treap4::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    updateSize(y);
    updateSize(x);
    return x;
}

/**
 * @brief 递归销毁树
 */
void Treap4::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/**
 * @brief 重置所有统计数据
 */
void Treap4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
