/**
 * @file IntervalTree5.cpp
 * @brief IntervalTree5 实现
 *
 * 实现区间树：增强子树最大端点、全交集查询、优先搜索。
 */

#include "utils/tree195/IntervalTree5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

IntervalTree5::IntervalTree5(QObject *parent) : QObject(parent) {}
IntervalTree5::~IntervalTree5() { clearNode(m_root); }

/* ---- Delete subtree ---- */

void IntervalTree5::clearNode(Node* node)
{
    if (!node) return;
    clearNode(node->left);
    clearNode(node->right);
    delete node;
}

/* ---- Update augmented max-end ---- */

void IntervalTree5::updateMaxEnd(Node* node)
{
    if (!node) return;
    node->maxEnd = 0.0;
    for (const auto& iv : node->leftSorted)
        node->maxEnd = qMax(node->maxEnd, iv.first.second);
    if (node->left)
        node->maxEnd = qMax(node->maxEnd, node->left->maxEnd);
    if (node->right)
        node->maxEnd = qMax(node->maxEnd, node->right->maxEnd);
}

/* ---- Build subtree ---- */

IntervalTree5::Node* IntervalTree5::buildNode(QVector<Interval>& intervals)
{
    if (intervals.isEmpty()) return nullptr;

    // Find center (median of endpoints)
    QVector<double> endpoints;
    for (const auto& iv : intervals) {
        endpoints.append(iv.first.first);
        endpoints.append(iv.first.second);
    }
    std::sort(endpoints.begin(), endpoints.end());
    double center = endpoints[endpoints.size() / 2];

    Node* node = new Node();
    node->center = center;

    QVector<Interval> leftInts, rightInts;

    for (const auto& iv : intervals) {
        double lo = iv.first.first, hi = iv.first.second;
        if (hi < center) {
            leftInts.append(iv);
        } else if (lo > center) {
            rightInts.append(iv);
        } else {
            // Overlaps center
            node->leftSorted.append(iv);
            node->rightSorted.append(iv);
        }
    }

    // Sort by start ascending
    std::sort(node->leftSorted.begin(), node->leftSorted.end(),
              [](const Interval& a, const Interval& b) {
                  return a.first.first < b.first.first;
              });

    // Sort by end descending
    std::sort(node->rightSorted.begin(), node->rightSorted.end(),
              [](const Interval& a, const Interval& b) {
                  return a.first.second > b.first.second;
              });

    node->left = buildNode(leftInts);
    node->right = buildNode(rightInts);
    updateMaxEnd(node);

    return node;
}

/* ---- Build tree ---- */

void IntervalTree5::build(const QVector<Interval>& intervals)
{
    clearNode(m_root);
    m_root = nullptr;
    m_size = intervals.size();

    if (intervals.isEmpty()) return;

    QVector<Interval> copy = intervals;
    m_root = buildNode(copy);

    // Compute height
    int h = 0;
    Node* n = m_root;
    while (n) { h++; n = n->left ? n->left : n->right; }
    m_stats.numIntervals = m_size;
    m_stats.treeHeight = h;
}

/* ---- Insert (incremental) ---- */

void IntervalTree5::insertNode(Node*& node, const Interval& iv)
{
    double lo = iv.first.first, hi = iv.first.second;

    if (!node) {
        node = new Node();
        node->center = (lo + hi) / 2.0;
        node->leftSorted.append(iv);
        node->rightSorted.append(iv);
        node->maxEnd = hi;
        return;
    }

    if (hi < node->center) {
        insertNode(node->left, iv);
    } else if (lo > node->center) {
        insertNode(node->right, iv);
    } else {
        // Insert into sorted arrays
        node->leftSorted.append(iv);
        std::sort(node->leftSorted.begin(), node->leftSorted.end(),
                  [](const Interval& a, const Interval& b) {
                      return a.first.first < b.first.first;
                  });
        node->rightSorted.append(iv);
        std::sort(node->rightSorted.begin(), node->rightSorted.end(),
                  [](const Interval& a, const Interval& b) {
                      return a.first.second > b.first.second;
                  });
    }
    updateMaxEnd(node);
}

void IntervalTree5::insert(const Interval& iv)
{
    insertNode(m_root, iv);
    m_size++;
    m_stats.numIntervals = m_size;
}

/* ---- Query overlaps ---- */

void IntervalTree5::queryNode(Node* node, double low, double high,
                                QVector<Interval>& result) const
{
    if (!node) return;

    // Prune: if max-end < low, no overlap possible in this subtree
    if (node->maxEnd < low) return;

    // Check left subtree
    if (node->left && node->left->maxEnd >= low)
        queryNode(node->left, low, high, result);

    // Check intervals at this node
    // leftSorted: intervals sorted by start ascending
    // If interval start <= high and interval end >= low -> overlap
    for (const auto& iv : node->leftSorted) {
        if (iv.first.first > high) break;  // sorted by start, can stop
        if (iv.first.second >= low && iv.first.first <= high)
            result.append(iv);
    }

    // Check right subtree
    if (node->right)
        queryNode(node->right, low, high, result);
}

QVector<IntervalTree5::Interval> IntervalTree5::queryAll(
    double low, double high) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> result;
    queryNode(m_root, low, high, result);

    const_cast<IntervalTree5*>(this)->m_stats.totalQueries++;
    const_cast<IntervalTree5*>(this)->m_stats.lastResultCount = result.size();
    m_timeSum += timer.elapsed();
    const_cast<IntervalTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalQueries;

    const_cast<IntervalTree5*>(this)->queryCompleted(
        result.size(), timer.elapsed());
    return result;
}

/* ---- Query point ---- */

QVector<IntervalTree5::Interval> IntervalTree5::queryPoint(
    double point) const
{
    return queryAll(point, point);
}

/* ---- Count overlaps ---- */

int IntervalTree5::countNode(Node* node, double low, double high) const
{
    if (!node || node->maxEnd < low) return 0;

    int count = 0;
    count += countNode(node->left, low, high);

    for (const auto& iv : node->leftSorted) {
        if (iv.first.first > high) break;
        if (iv.first.second >= low && iv.first.first <= high)
            count++;
    }

    count += countNode(node->right, low, high);
    return count;
}

int IntervalTree5::countOverlaps(double low, double high) const
{
    return countNode(m_root, low, high);
}

/* ---- Priority search ---- */

void IntervalTree5::priorityNode(Node* node, double low, double high, int k,
                                   QVector<QPair<double, Interval>>& heap) const
{
    if (!node || node->maxEnd < low) return;

    // Compute overlap sizes at this node
    for (const auto& iv : node->leftSorted) {
        if (iv.first.first > high) break;
        double oLo = qMax(iv.first.first, low);
        double oHi = qMin(iv.first.second, high);
        if (oHi >= oLo) {
            double overlap = oHi - oLo;
            if (heap.size() < k) {
                heap.append({overlap, iv});
                std::push_heap(heap.begin(), heap.end(),
                    [](const auto& a, const auto& b) { return a.first > b.first; });
            } else if (overlap > heap[0].first) {
                std::pop_heap(heap.begin(), heap.end(),
                    [](const auto& a, const auto& b) { return a.first > b.first; });
                heap.back() = {overlap, iv};
                std::push_heap(heap.begin(), heap.end(),
                    [](const auto& a, const auto& b) { return a.first > b.first; });
            }
        }
    }

    priorityNode(node->left, low, high, k, heap);
    priorityNode(node->right, low, high, k, heap);
}

QVector<IntervalTree5::Interval> IntervalTree5::prioritySearch(
    double low, double high, int k) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, Interval>> heap;
    heap.reserve(k + 1);
    priorityNode(m_root, low, high, k, heap);

    // Sort by overlap descending
    std::sort(heap.begin(), heap.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    QVector<Interval> result;
    for (const auto& p : heap) result.append(p.second);

    const_cast<IntervalTree5*>(this)->m_stats.totalQueries++;
    const_cast<IntervalTree5*>(this)->m_stats.lastResultCount = result.size();
    m_timeSum += timer.elapsed();
    const_cast<IntervalTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalQueries;

    return result;
}

/* ---- Reset ---- */

void IntervalTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
