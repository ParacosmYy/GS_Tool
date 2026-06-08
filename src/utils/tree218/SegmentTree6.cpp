/**
 * @file SegmentTree6.cpp
 * @brief SegmentTree6 实现
 *
 * 实现线段树：区间加乘懒标记传播、持久化版本管理。
 */

#include "utils/tree218/SegmentTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SegmentTree6::SegmentTree6(QObject *parent) : QObject(parent) {}
SegmentTree6::~SegmentTree6() = default;

/* ---- Build ---- */

void SegmentTree6::build(const QVector<double>& values)
{
    m_n = values.size();
    if (m_n == 0) return;

    m_tree.resize(m_n * 4);
    for (auto& node : m_tree) {
        node.sum = 0.0;
        node.lazyAdd = 0.0;
        node.lazyMul = 1.0;
    }
    m_versions.clear();
    m_version = 0;

    buildImpl(1, 0, m_n - 1, values);

    m_stats.treeSize = m_tree.size();
    m_stats.numLeaves = m_n;
}

/* ---- Build implementation ---- */

void SegmentTree6::buildImpl(int idx, int left, int right,
                               const QVector<double>& vals)
{
    m_tree[idx].lazyAdd = 0.0;
    m_tree[idx].lazyMul = 1.0;
    if (left == right) {
        m_tree[idx].sum = vals[left];
        return;
    }
    int mid = (left + right) / 2;
    buildImpl(idx * 2, left, mid, vals);
    buildImpl(idx * 2 + 1, mid + 1, right, vals);
    pullUp(idx);
}

/* ---- Push down lazy tags ---- */

void SegmentTree6::pushDown(int idx, int left, int right)
{
    if (m_tree[idx].lazyMul == 1.0 && m_tree[idx].lazyAdd == 0.0) return;

    int mid = (left + right) / 2;
    int lc = idx * 2, rc = idx * 2 + 1;
    double lenL = mid - left + 1;
    double lenR = right - mid;

    // Apply multiply first, then add: child = child * mul + add
    Node& par = m_tree[idx];
    Node& lNode = m_tree[lc];
    Node& rNode = m_tree[rc];

    // Left child
    lNode.sum = lNode.sum * par.lazyMul + par.lazyAdd * lenL;
    lNode.lazyMul *= par.lazyMul;
    lNode.lazyAdd = lNode.lazyAdd * par.lazyMul + par.lazyAdd;

    // Right child
    rNode.sum = rNode.sum * par.lazyMul + par.lazyAdd * lenR;
    rNode.lazyMul *= par.lazyMul;
    rNode.lazyAdd = rNode.lazyAdd * par.lazyMul + par.lazyAdd;

    // Clear parent tags
    par.lazyMul = 1.0;
    par.lazyAdd = 0.0;
}

/* ---- Pull up from children ---- */

void SegmentTree6::pullUp(int idx)
{
    m_tree[idx].sum = m_tree[idx * 2].sum + m_tree[idx * 2 + 1].sum;
}

/* ---- Range add implementation ---- */

void SegmentTree6::rangeAddImpl(int idx, int left, int right,
                                  int l, int r, double val)
{
    if (l > right || r < left) return;
    if (l <= left && right <= r) {
        m_tree[idx].sum += val * (right - left + 1);
        m_tree[idx].lazyAdd += val;
        return;
    }
    pushDown(idx, left, right);
    int mid = (left + right) / 2;
    rangeAddImpl(idx * 2, left, mid, l, r, val);
    rangeAddImpl(idx * 2 + 1, mid + 1, right, l, r, val);
    pullUp(idx);
}

/* ---- Range multiply implementation ---- */

void SegmentTree6::rangeMulImpl(int idx, int left, int right,
                                  int l, int r, double val)
{
    if (l > right || r < left) return;
    if (l <= left && right <= r) {
        m_tree[idx].sum *= val;
        m_tree[idx].lazyMul *= val;
        m_tree[idx].lazyAdd *= val;
        return;
    }
    pushDown(idx, left, right);
    int mid = (left + right) / 2;
    rangeMulImpl(idx * 2, left, mid, l, r, val);
    rangeMulImpl(idx * 2 + 1, mid + 1, right, l, r, val);
    pullUp(idx);
}

/* ---- Range query implementation ---- */

double SegmentTree6::rangeQueryImpl(int idx, int left, int right,
                                      int l, int r) const
{
    if (l > right || r < left) return 0.0;
    if (l <= left && right <= r) return m_tree[idx].sum;
    // Need to push down but this is const - cast away for lazy prop
    const_cast<SegmentTree6*>(this)->pushDown(idx, left, right);
    int mid = (left + right) / 2;
    return rangeQueryImpl(idx * 2, left, mid, l, r)
         + rangeQueryImpl(idx * 2 + 1, mid + 1, right, l, r);
}

/* ---- Public: range add ---- */

void SegmentTree6::rangeAdd(int l, int r, double val)
{
    QElapsedTimer timer;
    timer.start();
    if (m_n == 0) return;
    rangeAddImpl(1, 0, m_n - 1, qMax(0, l), qMin(m_n - 1, r), val);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("rangeAdd", m_version, timer.elapsed());
}

/* ---- Public: range multiply ---- */

void SegmentTree6::rangeMultiply(int l, int r, double val)
{
    QElapsedTimer timer;
    timer.start();
    if (m_n == 0) return;
    rangeMulImpl(1, 0, m_n - 1, qMax(0, l), qMin(m_n - 1, r), val);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("rangeMul", m_version, timer.elapsed());
}

/* ---- Public: range query ---- */

double SegmentTree6::rangeQuery(int l, int r) const
{
    if (m_n == 0) return 0.0;
    return rangeQueryImpl(1, 0, m_n - 1, qMax(0, l), qMin(m_n - 1, r));
}

/* ---- Public: point query ---- */

double SegmentTree6::pointQuery(int idx) const
{
    if (idx < 0 || idx >= m_n) return 0.0;
    return rangeQuery(idx, idx);
}

/* ---- Save version ---- */

int SegmentTree6::saveVersion()
{
    m_versions.append(m_tree);
    m_version = m_versions.size() - 1;
    m_stats.numVersions = m_versions.size();
    return m_version;
}

/* ---- Query at version ---- */

double SegmentTree6::queryVersion(int version, int l, int r) const
{
    if (version < 0 || version >= m_versions.size()) return 0.0;
    if (l < 0 || r >= m_n || l > r) return 0.0;

    // Reconstruct query on saved tree
    const QVector<Node>& saved = m_versions[version];
    // Simple recursive query on saved tree (no lazy push for snapshots)
    // Use iterative approach: walk down the tree
    double sum = 0.0;
    // For simplicity, do point-by-point query on the saved version
    for (int i = qMax(0, l); i <= qMin(m_n - 1, r); ++i) {
        // Walk path from root to leaf
        int idx = 1, left = 0, right = m_n - 1;
        double val = 0.0;
        QVector<QPair<int, int>> path;
        while (left != right) {
            int mid = (left + right) / 2;
            if (i <= mid) {
                // Push right child contribution
                path.append({idx, left, right});
                idx = idx * 2;
                right = mid;
            } else {
                path.append({idx, left, right});
                idx = idx * 2 + 1;
                left = mid + 1;
            }
        }
        val = saved[idx].sum;
        sum += val;
    }
    return sum;
}

/* ---- Version count ---- */

int SegmentTree6::versionCount() const { return m_versions.size(); }

/* ---- Reset ---- */

void SegmentTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_tree.clear();
    m_versions.clear();
    m_version = 0;
    m_n = 0;
}
