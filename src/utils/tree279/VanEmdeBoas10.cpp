/**
 * @file VanEmdeBoas10.cpp
 * @brief VanEmdeBoas10 实现
 *
 * 实现van Emde Boas树：哈希底层簇与x-fast trie增强的O(log log U)前驱查询空间高效数据结构。
 */

#include "utils/tree279/VanEmdeBoas10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VanEmdeBoas10::VanEmdeBoas10(QObject *parent)
    : QObject(parent)
{
    m_rootIdx = allocNode(m_universe);
}

VanEmdeBoas10::~VanEmdeBoas10() = default;

/* ---- Configuration ---- */

void VanEmdeBoas10::setUniverse(int u)
{
    // Round up to next power of 2
    int pu = 1;
    while (pu < u) pu *= 2;
    m_universe = qMax(2, pu);
    m_maxBits = countBits(m_universe);
    m_nodes.clear();
    m_xFast.clear();
    m_rootIdx = allocNode(m_universe);
    m_numElements = 0;
}

/* ---- Count bits ---- */

int VanEmdeBoas10::countBits(int u) const
{
    int bits = 0;
    while (u > 1) { u /= 2; bits++; }
    return qMax(1, bits);
}

/* ---- Allocate node ---- */

int VanEmdeBoas10::allocNode(int u)
{
    int idx = m_nodes.size();
    m_nodes.append(VEBNode{u, -1, -1, -1, {}});
    return idx;
}

/* ---- High/low/index helpers ---- */

int VanEmdeBoas10::high(int x, int u) const
{
    int sqrtU = qMax(1, static_cast<int>(qSqrt(static_cast<double>(u))));
    return x / sqrtU;
}

int VanEmdeBoas10::low(int x, int u) const
{
    int sqrtU = qMax(1, static_cast<int>(qSqrt(static_cast<double>(u))));
    return x % sqrtU;
}

int VanEmdeBoas10::index(int h, int l, int u) const
{
    int sqrtU = qMax(1, static_cast<int>(qSqrt(static_cast<double>(u))));
    return h * sqrtU + l;
}

/* ---- Get or create cluster ---- */

int VanEmdeBoas10::getOrCreateCluster(int nodeIdx, int clusterKey)
{
    VEBNode& node = m_nodes[nodeIdx];
    if (!node.cluster.contains(clusterKey)) {
        int sqrtU = qMax(2, static_cast<int>(qSqrt(static_cast<double>(node.universe))));
        int subIdx = allocNode(sqrtU);
        node.cluster[clusterKey] = subIdx;

        // Ensure summary exists
        if (node.summaryIdx < 0) {
            int summaryU = qMax(2, static_cast<int>(qSqrt(static_cast<double>(node.universe))));
            node.summaryIdx = allocNode(summaryU);
        }
    }
    return node.cluster[clusterKey];
}

/* ---- X-fast trie update ---- */

void VanEmdeBoas10::updateXFast(int x, bool add)
{
    for (int level = 0; level <= m_maxBits; ++level) {
        int prefix = x >> (m_maxBits - level);
        auto key = qMakePair(level, prefix);
        if (add)
            m_xFast.insert(key, x);
        else
            m_xFast.remove(key);
    }
}

/* ---- Recursive insert ---- */

void VanEmdeBoas10::insertRec(int nodeIdx, int x)
{
    VEBNode& node = m_nodes[nodeIdx];
    if (node.universe <= 2) {
        // Base case: direct element storage
        if (node.minVal == -1) {
            node.minVal = x;
            node.maxVal = x;
        } else {
            node.minVal = qMin(node.minVal, x);
            node.maxVal = qMax(node.maxVal, x);
        }
        return;
    }

    if (node.minVal == -1) {
        // Empty node: store as min (which is also max)
        node.minVal = x;
        node.maxVal = x;
        return;
    }

    if (x < node.minVal) {
        std::swap(x, node.minVal);
    }

    int h = high(x, node.universe);
    int l = low(x, node.universe);

    int clusterIdx = getOrCreateCluster(nodeIdx, h);
    VEBNode& cluster = m_nodes[clusterIdx];

    if (cluster.minVal == -1) {
        // Need to insert into summary
        if (node.summaryIdx >= 0)
            insertRec(node.summaryIdx, h);
        else {
            int summaryU = qMax(2, static_cast<int>(qSqrt(static_cast<double>(node.universe))));
            node.summaryIdx = allocNode(summaryU);
            insertRec(node.summaryIdx, h);
        }
    }

    insertRec(clusterIdx, l);
    node.maxVal = qMax(node.maxVal, x);
}

/* ---- Recursive remove ---- */

void VanEmdeBoas10::removeRec(int nodeIdx, int x)
{
    VEBNode& node = m_nodes[nodeIdx];
    if (node.minVal == -1) return;

    if (node.universe <= 2) {
        if (node.minVal == x && node.maxVal == x) {
            node.minVal = -1;
            node.maxVal = -1;
        } else if (node.minVal == x) {
            node.minVal = node.maxVal;
        } else if (node.maxVal == x) {
            node.maxVal = node.minVal;
        }
        return;
    }

    if (x == node.minVal) {
        // Find new min from summary
        if (node.summaryIdx >= 0 && m_nodes[node.summaryIdx].minVal != -1) {
            int h = m_nodes[node.summaryIdx].minVal;
            int clusterIdx = node.cluster.value(h, -1);
            if (clusterIdx >= 0) {
                node.minVal = index(h, m_nodes[clusterIdx].minVal, node.universe);
                x = node.minVal;
            } else {
                node.minVal = node.maxVal;
                return;
            }
        } else {
            if (x == node.maxVal) {
                node.minVal = -1;
                node.maxVal = -1;
            } else {
                node.minVal = node.maxVal;
            }
            return;
        }
    }

    int h = high(x, node.universe);
    int l = low(x, node.universe);
    int clusterIdx = node.cluster.value(h, -1);

    if (clusterIdx >= 0) {
        removeRec(clusterIdx, l);

        // If cluster became empty, remove from summary
        if (m_nodes[clusterIdx].minVal == -1) {
            if (node.summaryIdx >= 0)
                removeRec(node.summaryIdx, h);
            node.cluster.remove(h);
        }
    }

    // Update max
    if (x == node.maxVal) {
        if (node.summaryIdx >= 0 && m_nodes[node.summaryIdx].maxVal != -1) {
            int hMax = m_nodes[node.summaryIdx].maxVal;
            int cIdx = node.cluster.value(hMax, -1);
            if (cIdx >= 0)
                node.maxVal = index(hMax, m_nodes[cIdx].maxVal, node.universe);
            else
                node.maxVal = node.minVal;
        } else {
            node.maxVal = node.minVal;
        }
    }
}

/* ---- Recursive successor ---- */

int VanEmdeBoas10::successorRec(int nodeIdx, int x) const
{
    const VEBNode& node = m_nodes[nodeIdx];
    if (node.minVal == -1) return -1;

    if (node.universe <= 2) {
        if (x < node.minVal) return node.minVal;
        if (x < node.maxVal) return node.maxVal;
        return -1;
    }

    if (x < node.minVal) return node.minVal;

    int h = high(x, node.universe);
    int l = low(x, node.universe);
    int clusterIdx = node.cluster.value(h, -1);

    if (clusterIdx >= 0 && l <= m_nodes[clusterIdx].maxVal) {
        int s = successorRec(clusterIdx, l);
        if (s != -1) return index(h, s, node.universe);
    }

    // Search in summary for next non-empty cluster
    if (node.summaryIdx >= 0) {
        int nextCluster = successorRec(node.summaryIdx, h);
        if (nextCluster != -1) {
            int cIdx = node.cluster.value(nextCluster, -1);
            if (cIdx >= 0)
                return index(nextCluster, m_nodes[cIdx].minVal, node.universe);
        }
    }

    return -1;
}

/* ---- Recursive predecessor ---- */

int VanEmdeBoas10::predecessorRec(int nodeIdx, int x) const
{
    const VEBNode& node = m_nodes[nodeIdx];
    if (node.minVal == -1) return -1;

    if (node.universe <= 2) {
        if (x > node.maxVal) return node.maxVal;
        if (x > node.minVal) return node.minVal;
        return -1;
    }

    if (x > node.maxVal) return node.maxVal;

    int h = high(x, node.universe);
    int l = low(x, node.universe);
    int clusterIdx = node.cluster.value(h, -1);

    if (clusterIdx >= 0 && l >= m_nodes[clusterIdx].minVal) {
        int p = predecessorRec(clusterIdx, l);
        if (p != -1) return index(h, p, node.universe);
    }

    if (node.summaryIdx >= 0) {
        int prevCluster = predecessorRec(node.summaryIdx, h);
        if (prevCluster != -1) {
            int cIdx = node.cluster.value(prevCluster, -1);
            if (cIdx >= 0)
                return index(prevCluster, m_nodes[cIdx].maxVal, node.universe);
        }
    }

    return node.minVal;
}

/* ---- Public interface ---- */

void VanEmdeBoas10::insert(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (x < 0 || x >= m_universe) return;
    if (contains(x)) return;

    insertRec(m_rootIdx, x);
    updateXFast(x, true);
    m_numElements++;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_stats.universeSize = m_universe;
    m_stats.numElements = m_numElements;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("insert"), x, m_numElements, elapsed);
}

void VanEmdeBoas10::remove(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (!contains(x)) return;

    removeRec(m_rootIdx, x);
    updateXFast(x, false);
    m_numElements--;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_stats.numElements = m_numElements;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), x, m_numElements, elapsed);
}

bool VanEmdeBoas10::contains(int x) const
{
    if (x < 0 || x >= m_universe) return false;
    // Use x-fast trie for O(1) lookup
    auto key = qMakePair(m_maxBits, x);
    return m_xFast.contains(key);
}

int VanEmdeBoas10::successor(int x) const
{
    if (x < 0) return minimum();
    return successorRec(m_rootIdx, x);
}

int VanEmdeBoas10::predecessor(int x) const
{
    if (x >= m_universe) return maximum();
    return predecessorRec(m_rootIdx, x);
}

int VanEmdeBoas10::minimum() const
{
    return (m_rootIdx < m_nodes.size()) ? m_nodes[m_rootIdx].minVal : -1;
}

int VanEmdeBoas10::maximum() const
{
    return (m_rootIdx < m_nodes.size()) ? m_nodes[m_rootIdx].maxVal : -1;
}

int VanEmdeBoas10::size() const { return m_numElements; }

QVector<int> VanEmdeBoas10::elements() const
{
    QVector<int> result;
    int cur = minimum();
    while (cur != -1) {
        result.append(cur);
        int next = successor(cur);
        if (next <= cur) break;  // Safety check
        cur = next;
    }
    return result;
}

/* ---- Reset ---- */

void VanEmdeBoas10::resetStatistics()
{
    m_nodes.clear();
    m_xFast.clear();
    m_numElements = 0;
    m_rootIdx = allocNode(m_universe);
    m_stats = Stats{};
    m_timeSum = 0.0;
}
