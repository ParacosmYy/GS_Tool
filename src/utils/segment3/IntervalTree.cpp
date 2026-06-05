/**
 * @file IntervalTree.cpp
 * @brief 区间树实现 — AVL增强平衡BST
 */

#include "utils/segment3/IntervalTree.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
IntervalTree::IntervalTree(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_count(0)
    , m_timeSum(0.0)
{
}

/** @brief 插入区间
 *  @param start 起点 @param end 终点 @param data 关联数据 */
void IntervalTree::insert(double start, double end, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    Interval interval;
    interval.start = start;
    interval.end = end;
    interval.data = data;

    m_root = insertNode(m_root, interval);
    ++m_count;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalInserts;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalInserts + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit insertCompleted(start, end);
}

/** @brief 点查询 @param point 查询点 @return 匹配的区间列表 */
QVector<IntervalTree::Interval> IntervalTree::query(double point)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> results;
    queryPoint(m_root, point, results);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalQueries;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalInserts + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit queryCompleted(results.size());
    return results;
}

/** @brief 区间查询 @param start 起点 @param end 终点 @return 匹配的区间列表 */
QVector<IntervalTree::Interval> IntervalTree::queryRange(double start, double end)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> results;
    queryRangeNode(m_root, start, end, results);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalQueries;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalInserts + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit queryCompleted(results.size());
    return results;
}

/** @brief 清空所有区间 */
void IntervalTree::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_count = 0;
}

/** @brief 获取区间数量 */
int IntervalTree::count() const
{
    return m_count;
}

/** @brief 重置统计 */
void IntervalTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 递归插入 */
IntervalTree::Node* IntervalTree::insertNode(Node* node, const Interval& interval)
{
    if (!node) {
        Node* newNode = new Node;
        newNode->interval = interval;
        newNode->maxEnd = interval.end;
        newNode->left = nullptr;
        newNode->right = nullptr;
        newNode->height = 1;
        return newNode;
    }

    double key = interval.start;
    double nodeKey = node->interval.start;

    if (key < nodeKey) {
        node->left = insertNode(node->left, interval);
    } else {
        node->right = insertNode(node->right, interval);
    }

    updateNode(node);

    /* AVL平衡 */
    int bf = balanceFactor(node);

    if (bf > 1 && key < node->left->interval.start) {
        return rotateRight(node);
    }
    if (bf < -1 && key >= node->right->interval.start) {
        return rotateLeft(node);
    }
    if (bf > 1 && key >= node->left->interval.start) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (bf < -1 && key < node->right->interval.start) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

/** @brief 递归点查询 */
void IntervalTree::queryPoint(Node* node, double point,
                               QVector<Interval>& results) const
{
    if (!node) return;

    /* 如果点在当前区间内，加入结果 */
    if (point >= node->interval.start && point <= node->interval.end) {
        results.append(node->interval);
    }

    /* 左子树可能有包含该点的区间 */
    if (node->left && point <= node->left->maxEnd) {
        queryPoint(node->left, point, results);
    }

    /* 右子树可能有包含该点的区间 */
    if (node->right && point <= node->right->maxEnd) {
        queryPoint(node->right, point, results);
    }
}

/** @brief 递归区间查询 */
void IntervalTree::queryRangeNode(Node* node, double start, double end,
                                   QVector<Interval>& results) const
{
    if (!node) return;

    /* 检查当前区间是否与查询区间重叠 */
    if (start <= node->interval.end && end >= node->interval.start) {
        results.append(node->interval);
    }

    /* 左子树: 如果左子树maxEnd >= start则可能有重叠 */
    if (node->left && node->left->maxEnd >= start) {
        queryRangeNode(node->left, start, end, results);
    }

    /* 右子树 */
    if (node->right && node->right->maxEnd >= start) {
        queryRangeNode(node->right, start, end, results);
    }
}

/** @brief AVL右旋 */
IntervalTree::Node* IntervalTree::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    updateNode(y);
    updateNode(x);
    return x;
}

/** @brief AVL左旋 */
IntervalTree::Node* IntervalTree::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    updateNode(x);
    updateNode(y);
    return y;
}

/** @brief 获取节点高度 */
int IntervalTree::nodeHeight(Node* node) const
{
    return node ? node->height : 0;
}

/** @brief 计算平衡因子 */
int IntervalTree::balanceFactor(Node* node) const
{
    return node ? nodeHeight(node->left) - nodeHeight(node->right) : 0;
}

/** @brief 更新节点 */
void IntervalTree::updateNode(Node* node)
{
    if (!node) return;
    node->height = 1 + qMax(nodeHeight(node->left), nodeHeight(node->right));
    node->maxEnd = node->interval.end;
    if (node->left) node->maxEnd = qMax(node->maxEnd, node->left->maxEnd);
    if (node->right) node->maxEnd = qMax(node->maxEnd, node->right->maxEnd);
}

/** @brief 递归删除 */
void IntervalTree::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}
