/**
 * @file ImplicitTreap.cpp
 * @brief 隐式Treap实现
 */

#include "utils/treap2/ImplicitTreap.h"

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ImplicitTreap::ImplicitTreap(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
{
}

/** @brief 析构函数 — 递归释放所有节点 */
ImplicitTreap::~ImplicitTreap()
{
    deleteTree(m_root);
}

/**
 * @brief 尾部追加元素
 * @param value 元素值
 */
void ImplicitTreap::pushBack(double value)
{
    m_timer.start();

    Node* newNode = new Node(value);
    updateSize(newNode);

    m_root = merge(m_root, newNode);
    ++m_size;

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalOperations);

    emit structureChanged();
}

/**
 * @brief 在指定位置插入元素
 * @param pos 位置索引(0-based)
 * @param value 元素值
 */
void ImplicitTreap::insert(int pos, double value)
{
    pos = qBound(0, pos, m_size);
    m_timer.start();

    Node* newNode = new Node(value);
    updateSize(newNode);

    Node* left = nullptr;
    Node* right = nullptr;
    split(m_root, pos, left, right);

    m_root = merge(merge(left, newNode), right);
    ++m_size;

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalOperations);

    emit structureChanged();
}

/**
 * @brief 删除指定位置元素
 * @param pos 位置索引(0-based)
 */
void ImplicitTreap::remove(int pos)
{
    if (pos < 0 || pos >= m_size) return;
    m_timer.start();

    Node* left = nullptr;
    Node* mid = nullptr;
    Node* right = nullptr;

    split(m_root, pos, left, right);
    split(right, 1, mid, right);

    delete mid;
    m_root = merge(left, right);
    --m_size;

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalOperations);

    emit structureChanged();
}

/**
 * @brief 访问指定位置元素
 * @param pos 位置索引(0-based)
 * @return 元素值
 */
double ImplicitTreap::at(int pos) const
{
    if (pos < 0 || pos >= m_size) return 0.0;

    m_timer.start();

    Node* node = findByPos(m_root, pos);
    double val = node ? node->value : 0.0;

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalOperations);

    return val;
}

/**
 * @brief 反转区间[l, r]
 * @param l 左端点(含)
 * @param r 右端点(含)
 */
void ImplicitTreap::reverse(int l, int r)
{
    if (l < 0 || r >= m_size || l >= r) return;
    m_timer.start();

    Node* left = nullptr;
    Node* mid = nullptr;
    Node* right = nullptr;

    /* 分成 [0..l-1] | [l..r] | [r+1..end] */
    split(m_root, l, left, mid);
    split(mid, r - l + 1, mid, right);

    /* 在中间段打反转标记 */
    if (mid) {
        mid->rev = !mid->rev;
    }

    /* 合并回来 */
    m_root = merge(merge(left, mid), right);

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalOperations);

    emit structureChanged();
}

/** @brief 重置统计 */
void ImplicitTreap::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief 更新子树大小
 * @param node 待更新的节点
 */
void ImplicitTreap::updateSize(Node* node)
{
    if (!node) return;
    node->size = 1;
    if (node->left) node->size += node->left->size;
    if (node->right) node->size += node->right->size;
}

/**
 * @brief 下推懒标记
 * 如果有反转标记，交换左右子树并向下传播
 * @param node 待下推的节点
 */
void ImplicitTreap::pushDown(Node* node)
{
    if (!node || !node->rev) return;

    node->rev = false;
    std::swap(node->left, node->right);

    if (node->left) node->left->rev = !node->left->rev;
    if (node->right) node->right->rev = !node->right->rev;
}

/**
 * @brief 按位置分裂Treap
 * 前 k 个元素放入 left，其余放入 right
 * @param node 当前根节点
 * @param k 分裂位置
 * @param left [out] 左半部分
 * @param right [out] 右半部分
 */
void ImplicitTreap::split(Node* node, int k,
                          Node*& left, Node*& right) const
{
    if (!node) {
        left = right = nullptr;
        return;
    }

    pushDown(node);

    int leftSize = node->left ? node->left->size : 0;

    if (leftSize >= k) {
        /* 分裂点在左子树 */
        split(node->left, k, left, node->left);
        right = node;
    } else {
        /* 分裂点在右子树 */
        split(node->right, k - leftSize - 1, node->right, right);
        left = node;
    }

    updateSize(node);
}

/**
 * @brief 合并两棵子树
 * 按优先级合并，左树的所有位置 < 右树
 * @param left 左子树
 * @param right 右子树
 * @return 合并后的根
 */
ImplicitTreap::Node* ImplicitTreap::merge(Node* left, Node* right) const
{
    if (!left) return right;
    if (!right) return left;

    pushDown(left);
    pushDown(right);

    if (left->priority > right->priority) {
        left->right = merge(left->right, right);
        updateSize(left);
        return left;
    } else {
        right->left = merge(left, right->left);
        updateSize(right);
        return right;
    }
}

/**
 * @brief 递归删除子树
 * @param node 待删除的子树根
 */
void ImplicitTreap::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/**
 * @brief 按位置查找节点
 * @param node 当前节点
 * @param pos 目标位置
 * @return 目标节点指针
 */
ImplicitTreap::Node* ImplicitTreap::findByPos(Node* node, int pos) const
{
    if (!node) return nullptr;

    pushDown(node);

    int leftSize = node->left ? node->left->size : 0;

    if (pos < leftSize) {
        return findByPos(node->left, pos);
    } else if (pos == leftSize) {
        return node;
    } else {
        return findByPos(node->right, pos - leftSize - 1);
    }
}
