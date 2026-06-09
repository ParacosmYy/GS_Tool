/**
 * @file SegmentTree8.cpp
 * @brief SegmentTree8 实现
 *
 * 实现线段树：惰性传播与持久化版本的时间范围查询与回滚。
 */

#include "utils/tree246/SegmentTree8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SegmentTree8::SegmentTree8(QObject *parent) : QObject(parent) {}
SegmentTree8::~SegmentTree8() = default;

/* ---- Allocate node ---- */

int SegmentTree8::allocNode()
{
    int idx = m_nodes.size();
    m_nodes.append(Node{});
    return idx;
}

/* ---- Clone node (copy-on-write) ---- */

int SegmentTree8::cloneNode(int nodeIdx)
{
    if (nodeIdx < 0) return -1;
    int idx = allocNode();
    m_nodes[idx] = m_nodes[nodeIdx];  // Deep copy
    return idx;
}

/* ---- Build recursively ---- */

int SegmentTree8::buildRec(int l, int r, const QVector<double>& data)
{
    int nodeIdx = allocNode();
    auto& node = m_nodes[nodeIdx];

    if (l == r) {
        node.sum = (l < data.size()) ? data[l] : 0.0;
        node.minVal = node.sum;
        node.maxVal = node.sum;
        return nodeIdx;
    }

    int mid = (l + r) / 2;
    node.left = buildRec(l, mid, data);
    node.right = buildRec(mid + 1, r, data);

    const auto& lc = m_nodes[node.left];
    const auto& rc = m_nodes[node.right];
    node.sum = lc.sum + rc.sum;
    node.minVal = qMin(lc.minVal, rc.minVal);
    node.maxVal = qMax(lc.maxVal, rc.maxVal);

    return nodeIdx;
}

/* ---- Push lazy down ---- */

void SegmentTree8::pushDown(int nodeIdx, int l, int r)
{
    if (nodeIdx < 0) return;
    auto& node = m_nodes[nodeIdx];
    if (qAbs(node.lazy) < 1e-15) return;
    if (node.left < 0 && node.right < 0) return;

    int mid = (l + r) / 2;

    // Push to left child (clone for persistence)
    if (node.left >= 0) {
        int clonedLeft = cloneNode(node.left);
        m_nodes[node.left] = m_nodes[clonedLeft];  // Keep original
        node.left = clonedLeft;
        auto& lc = m_nodes[node.left];
        lc.sum += node.lazy * (mid - l + 1);
        lc.minVal += node.lazy;
        lc.maxVal += node.lazy;
        lc.lazy += node.lazy;
    }

    // Push to right child (clone for persistence)
    if (node.right >= 0) {
        int clonedRight = cloneNode(node.right);
        m_nodes[node.right] = m_nodes[clonedRight];
        node.right = clonedRight;
        auto& rc = m_nodes[node.right];
        rc.sum += node.lazy * (r - mid);
        rc.minVal += node.lazy;
        rc.maxVal += node.lazy;
        rc.lazy += node.lazy;
    }

    node.lazy = 0.0;
}

/* ---- Build ---- */

void SegmentTree8::build(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_versions.clear();
    m_size = data.size();
    if (m_size == 0) { m_root = -1; return; }

    m_root = buildRec(0, m_size - 1, data);

    // Create initial version
    m_currentVersion = 0;
    m_versions.append({m_root, m_nodes.size()});

    m_stats.treeSize = m_size;
    m_stats.numVersions = 1;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Point update (persistent) ---- */

int SegmentTree8::updateRec(int nodeIdx, int l, int r, int idx, double value)
{
    int newNode = cloneNode(nodeIdx);
    if (l == r) {
        m_nodes[newNode].sum = value;
        m_nodes[newNode].minVal = value;
        m_nodes[newNode].maxVal = value;
        return newNode;
    }

    int mid = (l + r) / 2;
    if (idx <= mid)
        m_nodes[newNode].left = updateRec(m_nodes[nodeIdx].left, l, mid, idx, value);
    else
        m_nodes[newNode].right = updateRec(m_nodes[nodeIdx].right, mid + 1, r, idx, value);

    const auto& lc = m_nodes[m_nodes[newNode].left];
    const auto& rc = m_nodes[m_nodes[newNode].right];
    m_nodes[newNode].sum = lc.sum + rc.sum;
    m_nodes[newNode].minVal = qMin(lc.minVal, rc.minVal);
    m_nodes[newNode].maxVal = qMax(lc.maxVal, rc.maxVal);

    return newNode;
}

void SegmentTree8::update(int index, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (index < 0 || index >= m_size || m_root < 0) return;

    m_root = updateRec(m_root, 0, m_size - 1, index, value);
    m_stats.numUpdates++;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Range update with lazy propagation ---- */

int SegmentTree8::rangeUpdateRec(int nodeIdx, int l, int r, int ql, int qr, double delta)
{
    int newNode = cloneNode(nodeIdx);
    auto& node = m_nodes[newNode];

    if (ql <= l && r <= qr) {
        node.sum += delta * (r - l + 1);
        node.minVal += delta;
        node.maxVal += delta;
        node.lazy += delta;
        return newNode;
    }

    pushDown(newNode, l, r);
    int mid = (l + r) / 2;

    if (ql <= mid)
        node.left = rangeUpdateRec(m_nodes[nodeIdx].left, l, mid, ql, qr, delta);
    if (qr > mid)
        node.right = rangeUpdateRec(m_nodes[nodeIdx].right, mid + 1, r, ql, qr, delta);

    const auto& lc = m_nodes[node.left];
    const auto& rc = m_nodes[node.right];
    node.sum = lc.sum + rc.sum;
    node.minVal = qMin(lc.minVal, rc.minVal);
    node.maxVal = qMax(lc.maxVal, rc.maxVal);

    return newNode;
}

void SegmentTree8::rangeUpdate(int l, int r, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (l > r || l < 0 || r >= m_size || m_root < 0) return;

    m_root = rangeUpdateRec(m_root, 0, m_size - 1, l, r, delta);
    m_stats.numUpdates++;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Range query ---- */

SegmentTree8::RangeResult SegmentTree8::queryRec(int nodeIdx, int l, int r, int ql, int qr) const
{
    if (nodeIdx < 0) return {0.0, std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()};

    const auto& node = m_nodes[nodeIdx];

    if (ql <= l && r <= qr)
        return {node.sum, node.minVal, node.maxVal};

    // Push lazy (read-only: don't modify, just account for it)
    double lazyOffset = node.lazy;
    int mid = (l + r) / 2;

    RangeResult result = {0.0, std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()};

    if (ql <= mid && node.left >= 0) {
        auto left = queryRec(node.left, l, mid, ql, qr);
        result.sum += left.sum;
        result.minimum = qMin(result.minimum, left.minimum);
        result.maximum = qMax(result.maximum, left.maximum);
    }
    if (qr > mid && node.right >= 0) {
        auto right = queryRec(node.right, mid + 1, r, ql, qr);
        result.sum += right.sum;
        result.minimum = qMin(result.minimum, right.minimum);
        result.maximum = qMax(result.maximum, right.maximum);
    }

    // Apply pending lazy to the queried range portion
    int overlapLen = qMin(qr, r) - qMax(ql, l) + 1;
    if (overlapLen > 0) {
        result.sum += lazyOffset * overlapLen;
        result.minimum += lazyOffset;
        result.maximum += lazyOffset;
    }

    return result;
}

double SegmentTree8::query(int index) const
{
    auto r = rangeQuery(index, index);
    return r.sum;
}

SegmentTree8::RangeResult SegmentTree8::rangeQuery(int l, int r) const
{
    if (l > r || m_root < 0) return {0.0, 0.0, 0.0};
    l = qMax(0, l);
    r = qMin(m_size - 1, r);
    m_stats.numQueries++;
    return queryRec(m_root, 0, m_size - 1, l, r);
}

/* ---- Version management ---- */

int SegmentTree8::commitVersion()
{
    QElapsedTimer timer;
    timer.start();

    m_versions.append({m_root, m_nodes.size()});
    m_currentVersion = m_versions.size() - 1;
    m_stats.numVersions = m_versions.size();

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit versionCreated(m_currentVersion, m_stats.numVersions, timer.elapsed());
    return m_currentVersion;
}

bool SegmentTree8::rollback(int version)
{
    if (version < 0 || version >= m_versions.size()) return false;
    m_root = m_versions[version].rootIdx;
    m_currentVersion = version;
    return true;
}

SegmentTree8::RangeResult SegmentTree8::queryVersion(int version, int l, int r) const
{
    if (version < 0 || version >= m_versions.size()) return {0.0, 0.0, 0.0};
    int savedRoot = m_root;
    // Temporarily use version's root (const_cast for querying)
    const_cast<int&>(m_root) = m_versions[version].rootIdx;
    auto result = rangeQuery(l, r);
    const_cast<int&>(m_root) = savedRoot;
    return result;
}

/* ---- Accessors ---- */

int SegmentTree8::currentVersion() const { return m_currentVersion; }
int SegmentTree8::versionCount() const { return m_versions.size(); }

/* ---- Reset ---- */

void SegmentTree8::resetStatistics()
{
    m_nodes.clear(); m_versions.clear();
    m_root = -1; m_size = 0; m_currentVersion = -1;
    m_stats = Stats{}; m_timeSum = 0.0;
}
