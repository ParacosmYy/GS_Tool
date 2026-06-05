/**
 * @file IntervalTree2.cpp
 * @brief 增强区间树实现
 */

#include "IntervalTree2.h"
#include <QElapsedTimer>
#include <algorithm>

IntervalTree2::IntervalTree2(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

IntervalTree2::~IntervalTree2()
{
    destroyTree(m_root);
}

void IntervalTree2::insert(const Interval& interval)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, interval);
    m_size++;

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalQueries);

    emit inserted(interval.id, interval.low, interval.high);
}

QVector<IntervalTree2::Interval> IntervalTree2::queryOverlaps(
    const Interval& query)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> result;
    queryOverlapsHelper(m_root, query, result);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalQueries);

    return result;
}

QVector<IntervalTree2::Interval> IntervalTree2::queryPoint(double point)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> result;
    queryPointHelper(m_root, point, result);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalQueries);

    return result;
}

bool IntervalTree2::hasAnyOverlap(const Interval& query)
{
    Node* node = m_root;
    while (node) {
        if (query.low <= node->interval.high && query.high >= node->interval.low)
            return true;
        if (node->left && node->left->maxEnd >= query.low)
            node = node->left;
        else
            node = node->right;
    }
    return false;
}

QVector<IntervalTree2::Interval> IntervalTree2::allIntervals() const
{
    QVector<Interval> result;
    collectAll(m_root, result);
    return result;
}

int IntervalTree2::size() const { return m_size; }

void IntervalTree2::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

IntervalTree2::Node* IntervalTree2::insertNode(Node* node, const Interval& iv)
{
    if (!node) return new Node(iv);

    if (iv.low < node->interval.low)
        node->left = insertNode(node->left, iv);
    else
        node->right = insertNode(node->right, iv);

    updateMaxEnd(node);

    /* AVL平衡 */
    node->height = 1 + qMax(height(node->left), height(node->right));
    int bf = balanceFactor(node);

    if (bf > 1 && iv.low < node->left->interval.low)
        return rotateRight(node);
    if (bf < -1 && iv.low >= node->right->interval.low)
        return rotateLeft(node);
    if (bf > 1 && iv.low >= node->left->interval.low) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (bf < -1 && iv.low < node->right->interval.low) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

IntervalTree2::Node* IntervalTree2::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    y->height = 1 + qMax(height(y->left), height(y->right));
    x->height = 1 + qMax(height(x->left), height(x->right));
    updateMaxEnd(y);
    updateMaxEnd(x);
    return x;
}

IntervalTree2::Node* IntervalTree2::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    x->height = 1 + qMax(height(x->left), height(x->right));
    y->height = 1 + qMax(height(y->left), height(y->right));
    updateMaxEnd(x);
    updateMaxEnd(y);
    return y;
}

int IntervalTree2::height(Node* n) const { return n ? n->height : 0; }
int IntervalTree2::balanceFactor(Node* n) const
{
    return n ? height(n->left) - height(n->right) : 0;
}

void IntervalTree2::updateMaxEnd(Node* node)
{
    if (!node) return;
    node->maxEnd = node->interval.high;
    if (node->left) node->maxEnd = qMax(node->maxEnd, node->left->maxEnd);
    if (node->right) node->maxEnd = qMax(node->maxEnd, node->right->maxEnd);
}

void IntervalTree2::queryOverlapsHelper(Node* node, const Interval& q,
                                          QVector<Interval>& result)
{
    if (!node) return;

    if (q.low <= node->interval.high && q.high >= node->interval.low)
        result.append(node->interval);

    if (node->left && node->left->maxEnd >= q.low)
        queryOverlapsHelper(node->left, q, result);
    if (node->right && node->interval.low <= q.high)
        queryOverlapsHelper(node->right, q, result);
}

void IntervalTree2::queryPointHelper(Node* node, double point,
                                       QVector<Interval>& result)
{
    if (!node) return;
    if (point >= node->interval.low && point <= node->interval.high)
        result.append(node->interval);
    if (node->left && node->left->maxEnd >= point)
        queryPointHelper(node->left, point, result);
    if (node->right && node->right->maxEnd >= point)
        queryPointHelper(node->right, point, result);
}

void IntervalTree2::collectAll(Node* node, QVector<Interval>& result) const
{
    if (!node) return;
    collectAll(node->left, result);
    result.append(node->interval);
    collectAll(node->right, result);
}

void IntervalTree2::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

IntervalTree2::Stats IntervalTree2::stats() const { return m_stats; }

void IntervalTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
