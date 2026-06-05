/**
 * @file IntervalTree2.cpp
 * @brief 增强区间树实现 — AVL平衡 + 增广子树最大值 + 重叠查询
 */

#include "utils/tree17/IntervalTree2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ========== 构造/析构 ========== */

IntervalTree2::IntervalTree2(QObject* parent)
    : QObject(parent), m_root(nullptr), m_size(0), m_nextId(1),
      m_timeSum(0.0) {}

IntervalTree2::~IntervalTree2()
{
    deleteSubtree(m_root);
}

void IntervalTree2::deleteSubtree(Node* node)
{
    if (!node) return;
    deleteSubtree(node->left);
    deleteSubtree(node->right);
    delete node;
}

/* ========== 插入 ========== */

void IntervalTree2::insert(double low, double high, int id)
{
    QElapsedTimer timer;
    timer.start();

    if (low > high) std::swap(low, high);

    Interval iv;
    iv.low  = low;
    iv.high = high;
    iv.id   = (id < 0) ? m_nextId++ : id;

    m_root = insertNode(m_root, iv);
    ++m_size;

    /* 统计更新 */
    ++m_stats.totalInsertions;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInsertions + m_stats.totalDeletions
                     + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit intervalInserted(iv.id, iv.low, iv.high);
}

IntervalTree2::Node* IntervalTree2::insertNode(Node* node, const Interval& iv)
{
    if (!node) return new Node(iv);

    if (iv.low < node->interval.low ||
        (iv.low == node->interval.low && iv.high < node->interval.high)) {
        node->left = insertNode(node->left, iv);
    } else {
        node->right = insertNode(node->right, iv);
    }

    updateAugment(node);

    /* AVL平衡 */
    int balance = getBalance(node);

    /* LL */
    if (balance > 1 && iv.low < node->left->interval.low)
        return rotateRight(node);

    /* RR */
    if (balance < -1 && iv.low >= node->right->interval.low)
        return rotateLeft(node);

    /* LR */
    if (balance > 1 && iv.low >= node->left->interval.low) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    /* RL */
    if (balance < -1 && iv.low < node->right->interval.low) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

/* ========== 删除 ========== */

bool IntervalTree2::remove(int id)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeNode(m_root, id, removed);

    if (removed) {
        --m_size;
        ++m_stats.totalDeletions;
        m_timeSum += timer.elapsed();
        quint64 totalOps = m_stats.totalInsertions + m_stats.totalDeletions
                         + m_stats.totalQueries;
        m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;
        emit intervalRemoved(id);
    }

    return removed;
}

IntervalTree2::Node* IntervalTree2::removeNode(Node* node, int id, bool& removed)
{
    if (!node) return nullptr;

    if (node->interval.id == id) {
        removed = true;

        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }

        /* 找右子树最小节点(中序后继) */
        Node* successor = node->right;
        while (successor->left) successor = successor->left;

        node->interval = successor->interval;
        node->right = removeNode(node->right, successor->interval.id, removed);
        /* restored removed flag (already true) */
        removed = true;
    } else {
        /* 需要搜索整棵树(因为id不决定位置) */
        node->left  = removeNode(node->left, id, removed);
        if (!removed) node->right = removeNode(node->right, id, removed);
    }

    if (!node) return nullptr;

    updateAugment(node);

    /* AVL平衡 */
    int balance = getBalance(node);

    if (balance > 1 && getBalance(node->left) >= 0)
        return rotateRight(node);
    if (balance > 1 && getBalance(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && getBalance(node->right) <= 0)
        return rotateLeft(node);
    if (balance < -1 && getBalance(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

/* ========== 重叠查询 ========== */

QVector<IntervalTree2::Interval> IntervalTree2::queryOverlaps(
    double low, double high) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> result;
    if (m_root) queryOverlapsHelper(m_root, low, high, result);

    /* 统计更新 */
    ++m_stats.totalQueries;
    m_stats.totalOverlapsFound += result.size();
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInsertions + m_stats.totalDeletions
                     + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return result;
}

void IntervalTree2::queryOverlapsHelper(Node* node, double low, double high,
                                          QVector<Interval>& result) const
{
    if (!node) return;

    /* 剪枝: 如果查询区间完全在子树最小low的左边, 不可能有重叠 */
    if (high < node->minLow) return;
    /* 剪枝: 如果查询区间完全在子树最大high的右边, 不可能有重叠 */
    if (low > node->maxHigh) return;

    /* 检查当前节点 */
    if (overlaps(node->interval, Interval{low, high, 0}))
        result.append(node->interval);

    /* 递归搜索左右子树 */
    queryOverlapsHelper(node->left, low, high, result);
    queryOverlapsHelper(node->right, low, high, result);
}

/* ========== 点查询(Stabbing) ========== */

QVector<IntervalTree2::Interval> IntervalTree2::queryPoint(double point) const
{
    return queryOverlaps(point, point);
}

/* ========== 静态重叠检测 ========== */

bool IntervalTree2::overlaps(const Interval& a, const Interval& b)
{
    return !(a.high < b.low || b.high < a.low);
}

/* ========== 所有区间(中序遍历) ========== */

QVector<IntervalTree2::Interval> IntervalTree2::allIntervals() const
{
    QVector<Interval> result;
    result.reserve(m_size);
    inorderTraversal(m_root, result);
    return result;
}

void IntervalTree2::inorderTraversal(Node* node, QVector<Interval>& result) const
{
    if (!node) return;
    inorderTraversal(node->left, result);
    result.append(node->interval);
    inorderTraversal(node->right, result);
}

/* ========== 清空 ========== */

void IntervalTree2::clear()
{
    deleteSubtree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/* ========== AVL旋转 ========== */

IntervalTree2::Node* IntervalTree2::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;

    updateAugment(y);
    updateAugment(x);
    return x;
}

IntervalTree2::Node* IntervalTree2::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;

    updateAugment(x);
    updateAugment(y);
    return y;
}

/* ========== 增广信息更新 ========== */

void IntervalTree2::updateAugment(Node* node)
{
    if (!node) return;

    node->height = 1 + qMax(getHeight(node->left), getHeight(node->right));

    node->maxHigh = node->interval.high;
    node->minLow  = node->interval.low;

    if (node->left) {
        node->maxHigh = qMax(node->maxHigh, node->left->maxHigh);
        node->minLow  = qMin(node->minLow,  node->left->minLow);
    }
    if (node->right) {
        node->maxHigh = qMax(node->maxHigh, node->right->maxHigh);
        node->minLow  = qMin(node->minLow,  node->right->minLow);
    }
}

/* ========== 平衡因子 ========== */

int IntervalTree2::getBalance(Node* node) const
{
    return getHeight(node->left) - getHeight(node->right);
}

int IntervalTree2::getHeight(Node* node) const
{
    return node ? node->height : 0;
}

/* ========== 重置统计 ========== */

void IntervalTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
