/**
 * @file SegmentTree4.cpp
 * @brief SegmentTree4 实现
 *
 * 实现线段树：懒惰传播、区间乘加操作、可持久化版本。
 */

#include "utils/tree178/SegmentTree4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SegmentTree4::SegmentTree4(QObject *parent)
    : QObject(parent)
{
}

SegmentTree4::~SegmentTree4() = default;

/* ---- Push up: recompute parent from children ---- */

void SegmentTree4::pushUp(int node)
{
    m_tree[node] = m_tree[node * 2] + m_tree[node * 2 + 1];
}

/* ---- Push down: propagate lazy tags ---- */

void SegmentTree4::pushDown(int node, int left, int right)
{
    auto& tag = m_lazy[node];
    if (tag.mul == 1.0 && tag.add == 0.0) return;

    int mid = (left + right) / 2;
    int leftChild = node * 2;
    int rightChild = node * 2 + 1;
    int leftLen = mid - left + 1;
    int rightLen = right - mid;

    /* Apply to left child: first multiply, then add */
    m_tree[leftChild] = m_tree[leftChild] * tag.mul + tag.add * leftLen;
    m_lazy[leftChild].mul *= tag.mul;
    m_lazy[leftChild].add = m_lazy[leftChild].add * tag.mul + tag.add;

    /* Apply to right child */
    m_tree[rightChild] = m_tree[rightChild] * tag.mul + tag.add * rightLen;
    m_lazy[rightChild].mul *= tag.mul;
    m_lazy[rightChild].add = m_lazy[rightChild].add * tag.mul + tag.add;

    /* Clear current tag */
    tag.mul = 1.0;
    tag.add = 0.0;
}

/* ---- Build ---- */

void SegmentTree4::buildImpl(int node, int left, int right,
                               const QVector<double>& data)
{
    m_lazy[node] = {1.0, 0.0};

    if (left == right) {
        m_tree[node] = (left < data.size()) ? data[left] : 0.0;
        return;
    }

    int mid = (left + right) / 2;
    buildImpl(node * 2, left, mid, data);
    buildImpl(node * 2 + 1, mid + 1, right, data);
    pushUp(node);
}

void SegmentTree4::build(const QVector<double>& data)
{
    m_size = data.size();
    if (m_size == 0) return;

    /* Allocate 4*n for segment tree */
    int treeSize = 4 * m_size;
    m_tree.resize(treeSize, 0.0);
    m_lazy.resize(treeSize);

    buildImpl(1, 0, m_size - 1, data);
    m_stats.treeSize = m_size;
}

/* ---- Range add ---- */

void SegmentTree4::rangeAddImpl(int node, int l, int r, int ql, int qr,
                                  double val)
{
    if (ql > r || qr < l) return;
    if (ql <= l && r <= qr) {
        int len = r - l + 1;
        m_tree[node] += val * len;
        m_lazy[node].add += val;
        return;
    }

    pushDown(node, l, r);
    int mid = (l + r) / 2;
    rangeAddImpl(node * 2, l, mid, ql, qr, val);
    rangeAddImpl(node * 2 + 1, mid + 1, r, ql, qr, val);
    pushUp(node);
}

void SegmentTree4::rangeAdd(int l, int r, double val)
{
    if (m_size == 0) return;
    rangeAddImpl(1, 0, m_size - 1, l, r, val);
    m_stats.totalUpdates++;
    emit updateCompleted(l, r, val);
}

/* ---- Range multiply ---- */

void SegmentTree4::rangeMulImpl(int node, int l, int r, int ql, int qr,
                                  double val)
{
    if (ql > r || qr < l) return;
    if (ql <= l && r <= qr) {
        int len = r - l + 1;
        m_tree[node] *= val;
        m_lazy[node].mul *= val;
        m_lazy[node].add *= val;
        return;
    }

    pushDown(node, l, r);
    int mid = (l + r) / 2;
    rangeMulImpl(node * 2, l, mid, ql, qr, val);
    rangeMulImpl(node * 2 + 1, mid + 1, r, ql, qr, val);
    pushUp(node);
}

void SegmentTree4::rangeMultiply(int l, int r, double val)
{
    if (m_size == 0) return;
    rangeMulImpl(1, 0, m_size - 1, l, r, val);
    m_stats.totalUpdates++;
    emit updateCompleted(l, r, val);
}

/* ---- Range query ---- */

double SegmentTree4::queryImpl(int node, int l, int r, int ql, int qr)
{
    if (ql > r || qr < l) return 0.0;
    if (ql <= l && r <= qr) return m_tree[node];

    pushDown(node, l, r);
    int mid = (l + r) / 2;
    return queryImpl(node * 2, l, mid, ql, qr) +
           queryImpl(node * 2 + 1, mid + 1, r, ql, qr);
}

double SegmentTree4::rangeQuery(int l, int r)
{
    if (m_size == 0) return 0.0;
    QElapsedTimer timer;
    timer.start();

    double result = queryImpl(1, 0, m_size - 1, l, r);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalUpdates + m_stats.totalQueries);

    emit queryCompleted(result);
    return result;
}

double SegmentTree4::pointQuery(int idx)
{
    return rangeQuery(idx, idx);
}

/* ---- Persistent versions ---- */

int SegmentTree4::saveVersion()
{
    /* Flatten tree to data array */
    QVector<double> data(m_size, 0.0);
    for (int i = 0; i < m_size; ++i)
        data[i] = pointQuery(i);

    m_versions.append(data);
    m_stats.numVersions = m_versions.size();
    return m_versions.size() - 1;
}

void SegmentTree4::restoreVersion(int versionId)
{
    if (versionId < 0 || versionId >= m_versions.size()) return;
    build(m_versions[versionId]);
}

int SegmentTree4::versionCount() const
{
    return m_versions.size();
}

/* ---- Reset ---- */

void SegmentTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_versions.clear();
}
