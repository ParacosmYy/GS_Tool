/**
 * @file BinaryIntervalTree.cpp
 * @brief 二叉区间树实现 — 线段树区间查询
 */

#include "utils/bintree/BinaryIntervalTree.h"

/** @brief 构造函数 @param parent 父对象 */
BinaryIntervalTree::BinaryIntervalTree(QObject* parent)
    : QObject(parent)
    , m_n(0)
{
}

/** @brief 构建区间树 @param values 初始数据 */
void BinaryIntervalTree::build(const QVector<double>& values)
{
    m_data = values;
    m_n = m_data.size();

    if (m_n == 0) return;

    /* 线段树大小: 4n足够 */
    int treeSize = 4 * m_n;
    m_sumTree.resize(treeSize);
    m_minTree.resize(treeSize);
    m_maxTree.resize(treeSize);

    m_sumTree.fill(0.0);
    m_minTree.fill(0.0);
    m_maxTree.fill(0.0);

    /* 构建三棵线段树 */
    buildSum(1, 0, m_n - 1);
    buildMin(1, 0, m_n - 1);
    buildMax(1, 0, m_n - 1, m_maxTree);
}

/** @brief 单点更新 @param index 索引 @param value 新值 */
void BinaryIntervalTree::update(int index, double value)
{
    if (index < 0 || index >= m_n) return;

    m_data[index] = value;
    updateSum(1, 0, m_n - 1, index, value);
    updateMin(1, 0, m_n - 1, index, value);
    updateMaxInTree(m_maxTree, 1, 0, m_n - 1, index, value);

    ++m_stats.totalUpdates;
    emit treeUpdated(index, value);
}

/** @brief 区间求和 @param left 左边界 @param right 右边界 @return 区间和 */
double BinaryIntervalTree::rangeSum(int left, int right)
{
    if (left < 0 || right >= m_n || left > right) return 0.0;
    ++m_stats.totalQueries;
    return querySum(1, 0, m_n - 1, left, right);
}

/** @brief 区间最小值 @param left 左边界 @param right 右边界 @return 最小值 */
double BinaryIntervalTree::rangeMin(int left, int right)
{
    if (left < 0 || right >= m_n || left > right) return 0.0;
    ++m_stats.totalQueries;
    return queryMin(1, 0, m_n - 1, left, right);
}

/** @brief 区间最大值 @param left 左边界 @param right 右边界 @return 最大值 */
double BinaryIntervalTree::rangeMax(int left, int right)
{
    if (left < 0 || right >= m_n || left > right) return 0.0;
    ++m_stats.totalQueries;
    return queryMax(m_maxTree, 1, 0, m_n - 1, left, right);
}

/** @brief 数据规模 @return 元素数量 */
int BinaryIntervalTree::size() const
{
    return m_n;
}

/** @brief 重置统计 */
void BinaryIntervalTree::resetStatistics()
{
    m_stats = Stats{};
}

/** @brief 构建求和树递归 */
void BinaryIntervalTree::buildSum(int node, int left, int right)
{
    if (left == right) {
        m_sumTree[node] = m_data[left];
        return;
    }
    int mid = (left + right) / 2;
    buildSum(node * 2, left, mid);
    buildSum(node * 2 + 1, mid + 1, right);
    m_sumTree[node] = m_sumTree[node * 2] + m_sumTree[node * 2 + 1];
}

/** @brief 构建最小树递归 */
void BinaryIntervalTree::buildMin(int node, int left, int right)
{
    if (left == right) {
        m_minTree[node] = m_data[left];
        return;
    }
    int mid = (left + right) / 2;
    buildMin(node * 2, left, mid);
    buildMin(node * 2 + 1, mid + 1, right);
    m_minTree[node] = qMin(m_minTree[node * 2], m_minTree[node * 2 + 1]);
}

/** @brief 构建最大树递归 */
void BinaryIntervalTree::buildMax(int node, int left, int right,
                                  QVector<double>& maxTree)
{
    if (left == right) {
        maxTree[node] = m_data[left];
        return;
    }
    int mid = (left + right) / 2;
    buildMax(node * 2, left, mid, maxTree);
    buildMax(node * 2 + 1, mid + 1, right, maxTree);
    maxTree[node] = qMax(maxTree[node * 2], maxTree[node * 2 + 1]);
}

/** @brief 更新求和树 */
void BinaryIntervalTree::updateSum(int node, int left, int right,
                                   int idx, double val)
{
    if (left == right) {
        m_sumTree[node] = val;
        return;
    }
    int mid = (left + right) / 2;
    if (idx <= mid) {
        updateSum(node * 2, left, mid, idx, val);
    } else {
        updateSum(node * 2 + 1, mid + 1, right, idx, val);
    }
    m_sumTree[node] = m_sumTree[node * 2] + m_sumTree[node * 2 + 1];
}

/** @brief 更新最小树 */
void BinaryIntervalTree::updateMin(int node, int left, int right,
                                   int idx, double val)
{
    if (left == right) {
        m_minTree[node] = val;
        return;
    }
    int mid = (left + right) / 2;
    if (idx <= mid) {
        updateMin(node * 2, left, mid, idx, val);
    } else {
        updateMin(node * 2 + 1, mid + 1, right, idx, val);
    }
    m_minTree[node] = qMin(m_minTree[node * 2], m_minTree[node * 2 + 1]);
}

/** @brief 更新最大树 */
void BinaryIntervalTree::updateMaxInTree(QVector<double>& tree,
    int node, int left, int right, int idx, double val)
{
    if (left == right) {
        tree[node] = val;
        return;
    }
    int mid = (left + right) / 2;
    if (idx <= mid) {
        updateMaxInTree(tree, node * 2, left, mid, idx, val);
    } else {
        updateMaxInTree(tree, node * 2 + 1, mid + 1, right, idx, val);
    }
    tree[node] = qMax(tree[node * 2], tree[node * 2 + 1]);
}

/** @brief 查询区间和 */
double BinaryIntervalTree::querySum(int node, int left, int right,
                                    int ql, int qr) const
{
    if (ql > right || qr < left) return 0.0;
    if (ql <= left && right <= qr) return m_sumTree[node];
    int mid = (left + right) / 2;
    return querySum(node * 2, left, mid, ql, qr)
         + querySum(node * 2 + 1, mid + 1, right, ql, qr);
}

/** @brief 查询区间最小 */
double BinaryIntervalTree::queryMin(int node, int left, int right,
                                    int ql, int qr) const
{
    if (ql > right || qr < left) return 1e308;
    if (ql <= left && right <= qr) return m_minTree[node];
    int mid = (left + right) / 2;
    return qMin(queryMin(node * 2, left, mid, ql, qr),
                queryMin(node * 2 + 1, mid + 1, right, ql, qr));
}

/** @brief 查询区间最大 */
double BinaryIntervalTree::queryMax(const QVector<double>& tree,
    int node, int left, int right, int ql, int qr) const
{
    if (ql > right || qr < left) return -1e308;
    if (ql <= left && right <= qr) return tree[node];
    int mid = (left + right) / 2;
    return qMax(queryMax(tree, node * 2, left, mid, ql, qr),
                queryMax(tree, node * 2 + 1, mid + 1, right, ql, qr));
}
