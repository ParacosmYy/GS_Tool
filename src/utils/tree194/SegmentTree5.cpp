/**
 * @file SegmentTree5.cpp
 * @brief SegmentTree5 实现
 *
 * 实现线段树：归并排序树构建、范围名次查询、第k小值查询、小波树混合优化。
 */

#include "utils/tree194/SegmentTree5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SegmentTree5::SegmentTree5(QObject *parent) : QObject(parent) {}
SegmentTree5::~SegmentTree5() = default;

/* ---- Build merge-sort tree recursively ---- */

void SegmentTree5::buildTree(int node, int start, int end)
{
    if (start == end) {
        m_tree[node].resize(1);
        m_tree[node][0] = m_data[start];
        return;
    }

    int mid = (start + end) / 2;
    int left = 2 * node;
    int right = 2 * node + 1;

    buildTree(left, start, mid);
    buildTree(right, mid + 1, end);

    // Merge children's sorted arrays
    const auto& lv = m_tree[left];
    const auto& rv = m_tree[right];
    int ls = lv.size(), rs = rv.size();
    m_tree[node].resize(ls + rs);

    int i = 0, j = 0, k = 0;
    while (i < ls && j < rs) {
        if (lv[i] <= rv[j])
            m_tree[node][k++] = lv[i++];
        else
            m_tree[node][k++] = rv[j++];
    }
    while (i < ls) m_tree[node][k++] = lv[i++];
    while (j < rs) m_tree[node][k++] = rv[j++];
}

/* ---- Binary search rank in a node ---- */

int SegmentTree5::rankInNode(int node, double value) const
{
    const auto& arr = m_tree[node];
    // Count elements strictly less than value
    int lo = 0, hi = arr.size();
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (arr[mid] < value)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

/* ---- Collect tree nodes covering [ql, qr] ---- */

void SegmentTree5::collectNodes(int node, int start, int end,
                                  int ql, int qr, QVector<int>& nodes) const
{
    if (ql > end || qr < start) return;
    if (ql <= start && end <= qr) {
        nodes.append(node);
        return;
    }
    int mid = (start + end) / 2;
    collectNodes(2 * node, start, mid, ql, qr, nodes);
    collectNodes(2 * node + 1, mid + 1, end, ql, qr, nodes);
}

/* ---- Build tree from data ---- */

void SegmentTree5::build(const QVector<double>& data)
{
    m_data = data;
    m_size = data.size();
    if (m_size == 0) return;

    // Allocate tree: 4*N nodes
    m_treeSize = 4 * m_size;
    m_tree.resize(m_treeSize);

    buildTree(1, 0, m_size - 1);

    // Global sorted copy for k-th query bounds
    m_sorted = m_data;
    std::sort(m_sorted.begin(), m_sorted.end());

    // Compute height
    m_stats.dataSize = m_size;
    m_stats.treeHeight = static_cast<int>(qCeil(qLog2(m_size))) + 1;
}

/* ---- Rank query ---- */

int SegmentTree5::rankQuery(int l, int r, double value) const
{
    QElapsedTimer timer;
    timer.start();

    if (l < 0 || r >= m_size || l > r) return 0;

    QVector<int> nodes;
    const_cast<SegmentTree5*>(this)->collectNodes(1, 0, m_size - 1, l, r, nodes);

    int count = 0;
    for (int n : nodes)
        count += rankInNode(n, value);

    const_cast<SegmentTree5*>(this)->m_stats.totalQueries++;
    const_cast<SegmentTree5*>(this)->m_stats.rankQueries++;
    m_timeSum += timer.elapsed();
    const_cast<SegmentTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalQueries;

    const_cast<SegmentTree5*>(this)->queryCompleted("rank", timer.elapsed());
    return count;
}

/* ---- K-th smallest in range ---- */

double SegmentTree5::kthSmallest(int l, int r, int k) const
{
    QElapsedTimer timer;
    timer.start();

    if (l < 0 || r >= m_size || l > r || k < 1 || k > (r - l + 1))
        return 0.0;

    // Binary search on value using rank query
    double lo = m_sorted[0];
    double hi = m_sorted[m_sorted.size() - 1];

    // Wavelet-tree-style binary search on value domain
    for (int iter = 0; iter < 64; ++iter) {
        double mid = (lo + hi) / 2.0;
        int rank = rankQuery(l, r, mid);

        if (rank < k)
            lo = mid;
        else
            hi = mid;

        if (hi - lo < 1e-12) break;
    }

    const_cast<SegmentTree5*>(this)->m_stats.totalQueries++;
    const_cast<SegmentTree5*>(this)->m_stats.kthQueries++;
    m_timeSum += timer.elapsed();
    const_cast<SegmentTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalQueries;

    const_cast<SegmentTree5*>(this)->queryCompleted("kth", timer.elapsed());
    return hi;
}

/* ---- Range count in [loVal, hiVal] ---- */

int SegmentTree5::rangeCount(int l, int r, double loVal, double hiVal) const
{
    QElapsedTimer timer;
    timer.start();

    if (l < 0 || r >= m_size || l > r) return 0;

    // Count < hiVal+eps minus count < loVal
    int upper = rankQuery(l, r, hiVal + 1e-9);
    int lower = rankQuery(l, r, loVal);
    int count = upper - lower;

    const_cast<SegmentTree5*>(this)->m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    const_cast<SegmentTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalQueries;

    const_cast<SegmentTree5*>(this)->queryCompleted("rangeCount", timer.elapsed());
    return count;
}

/* ---- Get data ---- */

QVector<double> SegmentTree5::data() const
{
    return m_data;
}

/* ---- Reset ---- */

void SegmentTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
