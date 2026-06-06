/**
 * @file IntervalTree4.cpp
 * @brief IntervalTree4 实现
 *
 * 实现区间树：中心点分割构建、穿刺查询、重叠检测、区间聚合。
 */

#include "utils/tree179/IntervalTree4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

IntervalTree4::IntervalTree4(QObject *parent) : QObject(parent) {}
IntervalTree4::~IntervalTree4() { deleteTree(m_root); }

/* ---- Delete tree recursively ---- */

void IntervalTree4::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Build: center-point split ---- */

IntervalTree4::Node* IntervalTree4::buildImpl(const QVector<Interval>& intervals,
                                                 int depth)
{
    if (intervals.isEmpty()) return nullptr;

    /* Find center point (median of all endpoints) */
    QVector<double> endpoints;
    for (const auto& iv : intervals) {
        endpoints.append(iv.low);
        endpoints.append(iv.high);
    }
    std::sort(endpoints.begin(), endpoints.end());
    double center = endpoints[endpoints.size() / 2];

    /* Split intervals: left, center, right */
    QVector<Interval> leftIv, centerIv, rightIv;
    for (const auto& iv : intervals) {
        if (iv.high < center) {
            leftIv.append(iv);
        } else if (iv.low > center) {
            rightIv.append(iv);
        } else {
            centerIv.append(iv);
        }
    }

    auto* node = new Node();
    node->center = center;
    node->byLowAsc = centerIv;
    node->byHighDesc = centerIv;

    /* Sort by low ascending for stabbing from left */
    std::sort(node->byLowAsc.begin(), node->byLowAsc.end(),
              [](const Interval& a, const Interval& b) { return a.low < b.low; });

    /* Sort by high descending for stabbing from right */
    std::sort(node->byHighDesc.begin(), node->byHighDesc.end(),
              [](const Interval& a, const Interval& b) { return a.high > b.high; });

    if (depth + 1 > m_height) m_height = depth + 1;
    node->left = buildImpl(leftIv, depth + 1);
    node->right = buildImpl(rightIv, depth + 1);

    return node;
}

void IntervalTree4::build(const QVector<Interval>& intervals)
{
    deleteTree(m_root);
    m_root = nullptr;
    m_height = 0;

    if (intervals.isEmpty()) return;
    m_root = buildImpl(intervals, 0);

    m_stats.numIntervals = intervals.size();
    m_stats.treeHeight = m_height;
}

/* ---- Stabbing query: find all intervals containing point x ---- */

void IntervalTree4::stabbingImpl(Node* node, double x,
                                    QVector<Interval>& result) const
{
    if (!node) return;

    if (x < node->center) {
        /* Check center intervals (sorted by low asc): include if low <= x */
        for (const auto& iv : node->byLowAsc) {
            if (iv.low <= x) result.append(iv);
            else break; /* Sorted by low ascending, can stop early */
        }
        stabbingImpl(node->left, x, result);
    } else {
        /* Check center intervals (sorted by high desc): include if high >= x */
        for (const auto& iv : node->byHighDesc) {
            if (iv.high >= x) result.append(iv);
            else break; /* Sorted by high descending, can stop early */
        }
        stabbingImpl(node->right, x, result);
    }
}

QVector<IntervalTree4::Interval> IntervalTree4::stabbingQuery(double x) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> result;
    stabbingImpl(m_root, x, result);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueries;

    emit queryCompleted(result.size());
    return result;
}

/* ---- Overlap query: find all intervals overlapping [low, high] ---- */

void IntervalTree4::overlapImpl(Node* node, double low, double high,
                                   QVector<Interval>& result) const
{
    if (!node) return;

    /* Check center intervals for overlap with [low, high] */
    for (const auto& iv : node->byLowAsc) {
        /* Overlap: NOT (iv.high < low || iv.low > high) */
        if (iv.high >= low && iv.low <= high)
            result.append(iv);
    }

    if (high < node->center) {
        overlapImpl(node->left, low, high, result);
    } else if (low > node->center) {
        overlapImpl(node->right, low, high, result);
    } else {
        /* Query spans center: search both subtrees */
        overlapImpl(node->left, low, high, result);
        overlapImpl(node->right, low, high, result);
    }
}

QVector<IntervalTree4::Interval> IntervalTree4::overlapQuery(double low,
                                                                double high) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Interval> result;
    overlapImpl(m_root, low, high, result);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueries;

    emit queryCompleted(result.size());
    return result;
}

/* ---- Aggregate: merge overlapping intervals ---- */

QVector<IntervalTree4::Interval> IntervalTree4::aggregate(
    const QVector<Interval>& intervals) const
{
    if (intervals.isEmpty()) return {};

    /* Sort by low endpoint */
    auto sorted = intervals;
    std::sort(sorted.begin(), sorted.end(),
              [](const Interval& a, const Interval& b) { return a.low < b.low; });

    QVector<Interval> merged;
    Interval current = sorted[0];

    for (int i = 1; i < sorted.size(); ++i) {
        if (sorted[i].low <= current.high) {
            /* Overlap: extend current */
            current.high = qMax(current.high, sorted[i].high);
        } else {
            merged.append(current);
            current = sorted[i];
        }
    }
    merged.append(current);
    return merged;
}

/* ---- Find all overlapping pairs ---- */

QVector<QPair<IntervalTree4::Interval, IntervalTree4::Interval>>
IntervalTree4::findAllOverlaps() const
{
    QVector<QPair<Interval, Interval>> overlaps;

    /* Collect all intervals via stabbing at each center */
    QVector<Interval> allIv;
    stabbingImpl(m_root, m_root ? m_root->center : 0.0, allIv);

    /* Also collect from subtrees by full scan */
    QVector<Interval> collected;
    auto collectAll = [&](auto& self, Node* node) -> void {
        if (!node) return;
        for (const auto& iv : node->byLowAsc)
            collected.append(iv);
        self(self, node->left);
        self(self, node->right);
    };
    collectAll(collectAll, m_root);

    /* Brute force pairwise overlap check */
    for (int i = 0; i < collected.size(); ++i)
        for (int j = i + 1; j < collected.size(); ++j) {
            const auto& a = collected[i];
            const auto& b = collected[j];
            if (a.high >= b.low && b.high >= a.low)
                overlaps.append({a, b});
        }

    return overlaps;
}

/* ---- Utility ---- */

bool IntervalTree4::isEmpty() const { return m_root == nullptr; }

void IntervalTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
