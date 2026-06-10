/**
 * @file SegmentTree10.cpp
 * @brief SegmentTree10 实现
 *
 * 实现线段树：持久化版本与路径复制不可变历史范围查询快照。
 */

#include "utils/tree274/SegmentTree10.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SegmentTree10::SegmentTree10(QObject *parent)
    : QObject(parent) {}

SegmentTree10::~SegmentTree10() = default;

/* ---- Build tree recursively ---- */

int SegmentTree10::buildRec(int left, int right, const QVector<double>& data)
{
    int idx = m_nodes.size();
    m_nodes.append(Node{});
    m_nodes[idx].rangeLeft = left;
    m_nodes[idx].rangeRight = right;

    if (left == right) {
        m_nodes[idx].sum = data[left];
        m_nodes[idx].minVal = data[left];
        return idx;
    }

    int mid = left + (right - left) / 2;
    m_nodes[idx].left = buildRec(left, mid, data);
    m_nodes[idx].right = buildRec(mid + 1, right, data);

    m_nodes[idx].sum = m_nodes[m_nodes[idx].left].sum + m_nodes[m_nodes[idx].right].sum;
    m_nodes[idx].minVal = qMin(m_nodes[m_nodes[idx].left].minVal,
                                m_nodes[m_nodes[idx].right].minVal);
    return idx;
}

/* ---- Persistent update: path-copy from old root ---- */

int SegmentTree10::updateRec(int nodeIdx, int index, double value, int left, int right)
{
    int newIdx = m_nodes.size();
    m_nodes.append(m_nodes[nodeIdx]);  // Copy node (path-copying)

    if (left == right) {
        m_nodes[newIdx].sum = value;
        m_nodes[newIdx].minVal = value;
        return newIdx;
    }

    int mid = left + (right - left) / 2;
    if (index <= mid) {
        m_nodes[newIdx].left = updateRec(m_nodes[nodeIdx].left, index, value, left, mid);
    } else {
        m_nodes[newIdx].right = updateRec(m_nodes[nodeIdx].right, index, value, mid + 1, right);
    }

    // Recompute aggregate values
    m_nodes[newIdx].sum = m_nodes[m_nodes[newIdx].left].sum + m_nodes[m_nodes[newIdx].right].sum;
    m_nodes[newIdx].minVal = qMin(m_nodes[m_nodes[newIdx].left].minVal,
                                   m_nodes[m_nodes[newIdx].right].minVal);
    return newIdx;
}

/* ---- Range sum query ---- */

double SegmentTree10::querySum(int nodeIdx, int ql, int qr) const
{
    if (nodeIdx < 0) return 0.0;
    const Node& node = m_nodes[nodeIdx];

    if (ql > node.rangeRight || qr < node.rangeLeft) return 0.0;
    if (ql <= node.rangeLeft && node.rangeRight <= qr) return node.sum;

    return querySum(node.left, ql, qr) + querySum(node.right, ql, qr);
}

/* ---- Range min query ---- */

double SegmentTree10::queryMin(int nodeIdx, int ql, int qr) const
{
    if (nodeIdx < 0) return 1e18;
    const Node& node = m_nodes[nodeIdx];

    if (ql > node.rangeRight || qr < node.rangeLeft) return 1e18;
    if (ql <= node.rangeLeft && node.rangeRight <= qr) return node.minVal;

    return qMin(queryMin(node.left, ql, qr), queryMin(node.right, ql, qr));
}

/* ---- Extract snapshot recursively ---- */

void SegmentTree10::snapshotRec(int nodeIdx, QVector<double>& result) const
{
    if (nodeIdx < 0) return;
    const Node& node = m_nodes[nodeIdx];

    if (node.rangeLeft == node.rangeRight) {
        if (node.rangeLeft < result.size())
            result[node.rangeLeft] = node.sum;
        return;
    }
    snapshotRec(node.left, result);
    snapshotRec(node.right, result);
}

/* ---- Build from data ---- */

void SegmentTree10::build(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_roots.clear();
    m_dataSize = data.size();

    if (m_dataSize == 0) return;

    int root = buildRec(0, m_dataSize - 1, data);
    m_roots.append(root);

    double elapsed = timer.elapsed();
    m_stats.dataSize = m_dataSize;
    m_stats.numVersions = 1;
    m_stats.numNodes = m_nodes.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit versionCreated(0, 1, elapsed);
}

/* ---- Point update (persistent, returns new version) ---- */

int SegmentTree10::update(int version, int index, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_roots.isEmpty() || version < 0 || version >= m_roots.size()) return -1;
    if (index < 0 || index >= m_dataSize) return -1;

    int newRoot = updateRec(m_roots[version], index, value, 0, m_dataSize - 1);
    m_roots.append(newRoot);

    int newVersion = m_roots.size() - 1;

    double elapsed = timer.elapsed();
    m_stats.numVersions = m_roots.size();
    m_stats.numNodes = m_nodes.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit versionCreated(newVersion, m_roots.size(), elapsed);

    return newVersion;
}

/* ---- Range sum query on version ---- */

double SegmentTree10::rangeSum(int version, int left, int right) const
{
    if (version < 0 || version >= m_roots.size()) return 0.0;
    return querySum(m_roots[version], left, right);
}

/* ---- Range min query on version ---- */

double SegmentTree10::rangeMin(int version, int left, int right) const
{
    if (version < 0 || version >= m_roots.size()) return 1e18;
    return queryMin(m_roots[version], left, right);
}

/* ---- Extract snapshot from version ---- */

QVector<double> SegmentTree10::snapshot(int version) const
{
    if (version < 0 || version >= m_roots.size() || m_dataSize == 0) return {};
    QVector<double> result(m_dataSize, 0.0);
    snapshotRec(m_roots[version], result);
    return result;
}

/* ---- Accessors ---- */

int SegmentTree10::latestVersion() const { return m_roots.size() - 1; }
int SegmentTree10::numVersions() const { return m_roots.size(); }

/* ---- Reset ---- */

void SegmentTree10::resetStatistics()
{
    m_nodes.clear();
    m_roots.clear();
    m_dataSize = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
