/**
 * @file VanEmdeBoas11.cpp
 * @brief VanEmdeBoas11 实现
 *
 * 实现Van Emde Boas树：哈希簇摘要与递归y-fast前驱检索实现O(lg lg U)前驱后继查询。
 */

#include "utils/tree293/VanEmdeBoas11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VanEmdeBoas11::VanEmdeBoas11(QObject *parent)
    : QObject(parent)
{
    // Initialize root node
    m_root = allocNode(m_universe);
}

VanEmdeBoas11::~VanEmdeBoas11() = default;

/* ---- Configuration ---- */

void VanEmdeBoas11::setUniverse(int U)
{
    // Round up to power of 2
    int p = 1;
    while (p < U) p *= 2;
    m_universe = qBound(2, p, (1 << 24));
    m_nodes.clear();
    m_nextFree = 0;
    m_root = allocNode(m_universe);
    m_size = 0;
    m_stats.universeSize = m_universe;
}

/* ---- Node allocation ---- */

int VanEmdeBoas11::allocNode(int universe)
{
    int idx = m_nextFree++;
    m_nodes.append(VEBNode{universe, NIL, NIL, NIL, {}});
    return idx;
}

/* ---- High/Low decomposition ---- */

int VanEmdeBoas11::high(int x, int u) const
{
    int sqrtU = qCeil(qSqrt(static_cast<double>(u)));
    return x / qMax(sqrtU, 1);
}

int VanEmdeBoas11::low(int x, int u) const
{
    int sqrtU = qCeil(qSqrt(static_cast<double>(u)));
    return x % qMax(sqrtU, 1);
}

int VanEmdeBoas11::index(int h, int l, int u) const
{
    int sqrtU = qCeil(qSqrt(static_cast<double>(u)));
    return h * sqrtU + l;
}

/* ---- Recursive insert ---- */

void VanEmdeBoas11::insertRec(int nodeIdx, int x)
{
    auto& node = m_nodes[nodeIdx];
    int u = node.universe;

    if (node.min == NIL) {
        // Empty node: set min = max = x
        node.min = x;
        node.max = x;
        return;
    }

    if (x < node.min) {
        std::swap(x, node.min);
    }

    if (u > 2) {
        int hi = high(x, u);
        int lo = low(x, u);

        // Ensure summary exists
        if (node.summaryIdx == NIL) {
            int sqrtU = qCeil(qSqrt(static_cast<double>(u)));
            node.summaryIdx = allocNode(sqrtU);
        }

        // Ensure cluster exists (hash-based)
        if (!node.clusterToIdx.contains(hi)) {
            int sqrtU = qCeil(qSqrt(static_cast<double>(u)));
            int clusterNode = allocNode(sqrtU);
            node.clusterToIdx[hi] = clusterNode;

            // Insert hi into summary
            insertRec(node.summaryIdx, hi);
        }

        // Insert into cluster
        int clusterIdx = node.clusterToIdx[hi];
        if (m_nodes[clusterIdx].min == NIL) {
            insertRec(clusterIdx, lo);
            // Also update summary if cluster was empty
        } else {
            insertRec(clusterIdx, lo);
        }
    }

    if (x > node.max)
        node.max = x;
}

/* ---- Recursive remove ---- */

void VanEmdeBoas11::removeRec(int nodeIdx, int x)
{
    auto& node = m_nodes[nodeIdx];
    int u = node.universe;

    if (node.min == node.max) {
        // Only one element
        if (node.min == x) {
            node.min = NIL;
            node.max = NIL;
        }
        return;
    }

    if (u == 2) {
        // Base case: universe of size 2
        if (x == 0) node.min = 1;
        else node.min = 0;
        node.max = node.min;
        return;
    }

    if (x == node.min) {
        // Find new minimum from summary
        if (node.summaryIdx != NIL) {
            int firstCluster = m_nodes[node.summaryIdx].min;
            if (firstCluster != NIL && node.clusterToIdx.contains(firstCluster)) {
                int clusterIdx = node.clusterToIdx[firstCluster];
                x = index(firstCluster, m_nodes[clusterIdx].min, u);
                node.min = x;
            }
        }
    }

    int hi = high(x, u);
    int lo = low(x, u);

    if (node.clusterToIdx.contains(hi)) {
        int clusterIdx = node.clusterToIdx[hi];
        removeRec(clusterIdx, lo);

        // If cluster became empty, remove from summary
        if (m_nodes[clusterIdx].min == NIL) {
            if (node.summaryIdx != NIL)
                removeRec(node.summaryIdx, hi);
            node.clusterToIdx.remove(hi);
        }
    }

    // Update max
    if (node.summaryIdx == NIL || m_nodes[node.summaryIdx].min == NIL) {
        // No clusters left
        if (x == node.max && x != node.min)
            node.max = node.min;
        else if (node.min == NIL)
            node.max = NIL;
    } else {
        // Max is in the highest non-empty cluster
        int summaryMax = m_nodes[node.summaryIdx].max;
        if (summaryMax != NIL && node.clusterToIdx.contains(summaryMax)) {
            int clusterIdx = node.clusterToIdx[summaryMax];
            node.max = index(summaryMax, m_nodes[clusterIdx].max, u);
        }
    }
}

/* ---- Recursive contains ---- */

bool VanEmdeBoas11::containsRec(int nodeIdx, int x) const
{
    if (nodeIdx == NIL || nodeIdx >= m_nodes.size()) return false;
    const auto& node = m_nodes[nodeIdx];

    if (x == node.min || x == node.max) return true;
    if (node.universe <= 2) return false;

    int hi = high(x, node.universe);
    if (node.clusterToIdx.contains(hi))
        return containsRec(node.clusterToIdx[hi], low(x, node.universe));
    return false;
}

/* ---- Recursive min/max ---- */

int VanEmdeBoas11::minRec(int nodeIdx) const
{
    if (nodeIdx == NIL || nodeIdx >= m_nodes.size()) return NIL;
    return m_nodes[nodeIdx].min;
}

int VanEmdeBoas11::maxRec(int nodeIdx) const
{
    if (nodeIdx == NIL || nodeIdx >= m_nodes.size()) return NIL;
    return m_nodes[nodeIdx].max;
}

/* ---- Recursive successor ---- */

int VanEmdeBoas11::successorRec(int nodeIdx, int x) const
{
    if (nodeIdx == NIL || nodeIdx >= m_nodes.size()) return NIL;
    const auto& node = m_nodes[nodeIdx];
    int u = node.universe;

    if (u == 2) {
        if (x == 0 && node.max == 1) return 1;
        return NIL;
    }

    if (node.min != NIL && x < node.min)
        return node.min;

    int hi = high(x, u);
    int lo = low(x, u);

    if (node.clusterToIdx.contains(hi)) {
        int clusterIdx = node.clusterToIdx[hi];
        int clusterMax = m_nodes[clusterIdx].max;
        if (clusterMax != NIL && lo < clusterMax) {
            // Successor is in same cluster
            int offset = successorRec(clusterIdx, lo);
            return (offset != NIL) ? index(hi, offset, u) : NIL;
        }
    }

    // Successor is in next non-empty cluster
    if (node.summaryIdx != NIL) {
        int succCluster = successorRec(node.summaryIdx, hi);
        if (succCluster != NIL && node.clusterToIdx.contains(succCluster)) {
            int clusterIdx = node.clusterToIdx[succCluster];
            int offset = m_nodes[clusterIdx].min;
            return (offset != NIL) ? index(succCluster, offset, u) : NIL;
        }
    }
    return NIL;
}

/* ---- Recursive predecessor ---- */

int VanEmdeBoas11::predecessorRec(int nodeIdx, int x) const
{
    if (nodeIdx == NIL || nodeIdx >= m_nodes.size()) return NIL;
    const auto& node = m_nodes[nodeIdx];
    int u = node.universe;

    if (u == 2) {
        if (x == 1 && node.min == 0) return 0;
        return NIL;
    }

    if (node.max != NIL && x > node.max)
        return node.max;

    int hi = high(x, u);
    int lo = low(x, u);

    if (node.clusterToIdx.contains(hi)) {
        int clusterIdx = node.clusterToIdx[hi];
        int clusterMin = m_nodes[clusterIdx].min;
        if (clusterMin != NIL && lo > clusterMin) {
            int offset = predecessorRec(clusterIdx, lo);
            return (offset != NIL) ? index(hi, offset, u) : NIL;
        }
    }

    // Predecessor is in previous non-empty cluster
    if (node.summaryIdx != NIL) {
        int predCluster = predecessorRec(node.summaryIdx, hi);
        if (predCluster != NIL && node.clusterToIdx.contains(predCluster)) {
            int clusterIdx = node.clusterToIdx[predCluster];
            int offset = m_nodes[clusterIdx].max;
            return (offset != NIL) ? index(predCluster, offset, u) : NIL;
        }
    }

    // Could be the overall min
    if (node.min != NIL && x > node.min)
        return node.min;
    return NIL;
}

/* ---- Compute height estimate ---- */

int VanEmdeBoas11::computeHeight() const
{
    if (m_universe <= 1) return 0;
    int h = 0;
    int u = m_universe;
    while (u > 2) {
        u = qCeil(qSqrt(static_cast<double>(u)));
        h++;
    }
    return h;
}

/* ---- Public interface ---- */

void VanEmdeBoas11::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (key < 0 || key >= m_universe) return;
    if (!containsRec(m_root, key)) {
        insertRec(m_root, key);
        m_size++;
    }

    double elapsed = timer.elapsed();
    m_stats.universeSize = m_universe;
    m_stats.numElements = m_size;
    m_stats.treeHeight = computeHeight();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("insert"), key, elapsed);
}

void VanEmdeBoas11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (containsRec(m_root, key)) {
        removeRec(m_root, key);
        m_size--;
    }

    double elapsed = timer.elapsed();
    m_stats.numElements = m_size;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("remove"), key, elapsed);
}

bool VanEmdeBoas11::contains(int key) const
{
    return containsRec(m_root, key);
}

int VanEmdeBoas11::predecessor(int key) const
{
    return predecessorRec(m_root, key);
}

int VanEmdeBoas11::successor(int key) const
{
    return successorRec(m_root, key);
}

int VanEmdeBoas11::minimum() const { return minRec(m_root); }
int VanEmdeBoas11::maximum() const { return maxRec(m_root); }

/* ---- Reset ---- */

void VanEmdeBoas11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_nextFree = 0;
    m_root = allocNode(m_universe);
    m_size = 0;
}
