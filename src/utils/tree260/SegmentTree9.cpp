/**
 * @file SegmentTree9.cpp
 * @brief SegmentTree9 实现
 *
 * 实现线段树：Beats操作与懒惰传播区间赋值最大最小和查询。
 */

#include "utils/tree260/SegmentTree9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SegmentTree9::SegmentTree9(QObject *parent)
    : QObject(parent) {}
SegmentTree9::~SegmentTree9() = default;

/* ---- Pull up from children ---- */

void SegmentTree9::pullUp(int idx)
{
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    Node& node = m_tree[idx];
    const Node& lc = m_tree[left];
    const Node& rc = m_tree[right];

    node.sum = lc.sum + rc.sum;

    // Max tracking with second max
    if (lc.maxValue >= rc.maxValue) {
        node.maxValue = lc.maxValue;
        node.maxCount = lc.maxCount;
        node.secondMax = qMax(lc.secondMax, rc.maxValue);
    } else {
        node.maxValue = rc.maxValue;
        node.maxCount = rc.maxCount;
        node.secondMax = qMax(lc.maxValue, rc.secondMax);
    }

    // Min tracking with second min
    if (lc.minValue <= rc.minValue) {
        node.minValue = lc.minValue;
        node.minCount = lc.minCount;
        node.secondMin = qMin(lc.secondMin, rc.minValue);
    } else {
        node.minValue = rc.minValue;
        node.minCount = rc.minCount;
        node.secondMin = qMin(lc.minValue, rc.secondMin);
    }
}

/* ---- Push down lazy assignment ---- */

void SegmentTree9::pushDown(int idx, int lo, int hi)
{
    Node& node = m_tree[idx];
    if (!node.hasAssign) return;

    int mid = (lo + hi) / 2;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    // Apply to left child
    double v = node.lazyAssign;
    int leftSize = mid - lo + 1;
    m_tree[left].maxValue = v;
    m_tree[left].secondMax = -std::numeric_limits<double>::infinity();
    m_tree[left].minValue = v;
    m_tree[left].secondMin = std::numeric_limits<double>::infinity();
    m_tree[left].sum = v * leftSize;
    m_tree[left].maxCount = leftSize;
    m_tree[left].minCount = leftSize;
    m_tree[left].lazyAssign = v;
    m_tree[left].hasAssign = true;

    // Apply to right child
    int rightSize = hi - mid;
    m_tree[right].maxValue = v;
    m_tree[right].secondMax = -std::numeric_limits<double>::infinity();
    m_tree[right].minValue = v;
    m_tree[right].secondMin = std::numeric_limits<double>::infinity();
    m_tree[right].sum = v * rightSize;
    m_tree[right].maxCount = rightSize;
    m_tree[right].minCount = rightSize;
    m_tree[right].lazyAssign = v;
    m_tree[right].hasAssign = true;

    node.hasAssign = false;
    node.lazyAssign = qQNaN();
}

/* ---- Apply chmin beats ---- */

void SegmentTree9::applyChmin(int idx, double value)
{
    Node& node = m_tree[idx];
    if (value >= node.maxValue) return;
    if (value > node.secondMax) {
        // Beats: only max values change
        double diff = node.maxValue - value;
        node.sum -= diff * node.maxCount;
        node.maxValue = value;
        // If max == min, update min too
        if (node.maxValue <= node.minValue) {
            node.minValue = node.maxValue;
            node.minCount = node.maxCount;
        }
    } else {
        // Need to recurse (handled by caller)
    }
}

/* ---- Apply chmax beats ---- */

void SegmentTree9::applyChmax(int idx, double value)
{
    Node& node = m_tree[idx];
    if (value <= node.minValue) return;
    if (value < node.secondMin) {
        // Beats: only min values change
        double diff = value - node.minValue;
        node.sum += diff * node.minCount;
        node.minValue = value;
        if (node.minValue >= node.maxValue) {
            node.maxValue = node.minValue;
            node.maxCount = node.minCount;
        }
    }
}

/* ---- Build ---- */

void SegmentTree9::build(const QVector<double>& values)
{
    m_n = values.size();
    if (m_n == 0) return;
    m_tree.resize(4 * m_n);
    buildHelper(0, 0, m_n - 1, values);
    m_stats.treeSize = m_n;
}

void SegmentTree9::buildHelper(int idx, int lo, int hi, const QVector<double>& values)
{
    if (lo == hi) {
        Node& node = m_tree[idx];
        node.maxValue = values[lo];
        node.minValue = values[lo];
        node.sum = values[lo];
        node.maxCount = 1;
        node.minCount = 1;
        node.secondMax = -std::numeric_limits<double>::infinity();
        node.secondMin = std::numeric_limits<double>::infinity();
        node.hasAssign = false;
        return;
    }
    int mid = (lo + hi) / 2;
    buildHelper(2 * idx + 1, lo, mid, values);
    buildHelper(2 * idx + 2, mid + 1, hi, values);
    pullUp(idx);
}

/* ---- Range assign ---- */

void SegmentTree9::rangeAssign(int lo, int hi, double value)
{
    if (m_n == 0) return;
    QElapsedTimer timer;
    timer.start();
    assignHelper(0, 0, m_n - 1, lo, hi, value);
    double elapsed = timer.elapsed();
    m_stats.numUpdates++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit updateCompleted(lo, hi, elapsed);
}

void SegmentTree9::assignHelper(int idx, int lo, int hi, int qLo, int qHi, double value)
{
    if (qLo > hi || qHi < lo) return;
    if (qLo <= lo && hi <= qHi) {
        int size = hi - lo + 1;
        m_tree[idx].maxValue = value;
        m_tree[idx].secondMax = -std::numeric_limits<double>::infinity();
        m_tree[idx].minValue = value;
        m_tree[idx].secondMin = std::numeric_limits<double>::infinity();
        m_tree[idx].sum = value * size;
        m_tree[idx].maxCount = size;
        m_tree[idx].minCount = size;
        m_tree[idx].lazyAssign = value;
        m_tree[idx].hasAssign = true;
        return;
    }
    pushDown(idx, lo, hi);
    int mid = (lo + hi) / 2;
    assignHelper(2 * idx + 1, lo, mid, qLo, qHi, value);
    assignHelper(2 * idx + 2, mid + 1, hi, qLo, qHi, value);
    pullUp(idx);
}

/* ---- Range chmin (Beats) ---- */

void SegmentTree9::rangeChmin(int lo, int hi, double value)
{
    if (m_n == 0) return;
    QElapsedTimer timer;
    timer.start();
    chminHelper(0, 0, m_n - 1, lo, hi, value);
    double elapsed = timer.elapsed();
    m_stats.numBeatsPushes++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

void SegmentTree9::chminHelper(int idx, int lo, int hi, int qLo, int qHi, double value)
{
    if (qLo > hi || qHi < lo) return;
    if (value >= m_tree[idx].maxValue) return;
    if (qLo <= lo && hi <= qHi && value > m_tree[idx].secondMax) {
        applyChmin(idx, value);
        return;
    }
    pushDown(idx, lo, hi);
    int mid = (lo + hi) / 2;
    chminHelper(2 * idx + 1, lo, mid, qLo, qHi, value);
    chminHelper(2 * idx + 2, mid + 1, hi, qLo, qHi, value);
    pullUp(idx);
}

/* ---- Range chmax (Beats) ---- */

void SegmentTree9::rangeChmax(int lo, int hi, double value)
{
    if (m_n == 0) return;
    QElapsedTimer timer;
    timer.start();
    chmaxHelper(0, 0, m_n - 1, lo, hi, value);
    double elapsed = timer.elapsed();
    m_stats.numBeatsPushes++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

void SegmentTree9::chmaxHelper(int idx, int lo, int hi, int qLo, int qHi, double value)
{
    if (qLo > hi || qHi < lo) return;
    if (value <= m_tree[idx].minValue) return;
    if (qLo <= lo && hi <= qHi && value < m_tree[idx].secondMin) {
        applyChmax(idx, value);
        return;
    }
    pushDown(idx, lo, hi);
    int mid = (lo + hi) / 2;
    chmaxHelper(2 * idx + 1, lo, mid, qLo, qHi, value);
    chmaxHelper(2 * idx + 2, mid + 1, hi, qLo, qHi, value);
    pullUp(idx);
}

/* ---- Range query ---- */

SegmentTree9::RangeResult SegmentTree9::rangeQuery(int lo, int hi) const
{
    RangeResult result;
    if (m_n == 0) return result;
    QElapsedTimer timer;
    timer.start();
    result = queryHelper(0, 0, m_n - 1, lo, hi);
    double elapsed = timer.elapsed();
    const_cast<SegmentTree9*>(this)->m_stats.numQueries++;
    const_cast<SegmentTree9*>(this)->m_stats.totalOps++;
    const_cast<SegmentTree9*>(this)->m_timeSum += elapsed;
    const_cast<SegmentTree9*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<SegmentTree9*>(this)->queryCompleted(
        lo, hi, result.maximum, result.minimum, result.sum, elapsed);
    return result;
}

SegmentTree9::RangeResult SegmentTree9::queryHelper(int idx, int lo, int hi, int qLo, int qHi) const
{
    if (qLo > hi || qHi < lo) return {0.0, 0.0, 0.0};
    if (qLo <= lo && hi <= qHi) {
        return {m_tree[idx].maxValue, m_tree[idx].minValue, m_tree[idx].sum};
    }
    const_cast<SegmentTree9*>(this)->pushDown(idx, lo, hi);
    int mid = (lo + hi) / 2;
    RangeResult left = queryHelper(2 * idx + 1, lo, mid, qLo, qHi);
    RangeResult right = queryHelper(2 * idx + 2, mid + 1, hi, qLo, qHi);
    return {qMax(left.maximum, right.maximum),
            qMin(left.minimum, right.minimum),
            left.sum + right.sum};
}

/* ---- Point query ---- */

double SegmentTree9::pointQuery(int index) const
{
    auto r = rangeQuery(index, index);
    return r.sum;
}

/* ---- Size ---- */

int SegmentTree9::size() const { return m_n; }

/* ---- Reset ---- */

void SegmentTree9::resetStatistics()
{
    m_tree.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
