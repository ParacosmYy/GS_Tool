/**
 * @file VanEmdeBoas5.cpp
 * @brief VanEmdeBoas5 实现
 *
 * 实现van Emde Boas树：聚类递归布局、O(log log U)操作、溢出链表密集键支持。
 */

#include "utils/tree209/VanEmdeBoas5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VanEmdeBoas5::VanEmdeBoas5(int universeBits, QObject *parent)
    : QObject(parent), m_universeBits(qMax(1, universeBits))
{
    m_universe = 1 << m_universeBits;
    m_sqrtU = 1 << (m_universeBits / 2 + (m_universeBits % 2));
    m_root = allocCluster(m_sqrtU);
    m_stats.universeSize = m_universe;
}

VanEmdeBoas5::~VanEmdeBoas5() = default;

/* ---- Allocate cluster ---- */

int VanEmdeBoas5::allocCluster(int subSize)
{
    Cluster c;
    c.min = -1;
    c.max = -1;
    c.summary = -1;
    c.subClusters.resize(subSize, -1);
    int idx = m_clusters.size();
    m_clusters.append(c);
    return idx;
}

/* ---- High/low bits ---- */

int VanEmdeBoas5::high(int key) const { return key / m_sqrtU; }
int VanEmdeBoas5::low(int key) const { return key % m_sqrtU; }
int VanEmdeBoas5::index(int h, int l) const { return h * m_sqrtU + l; }

/* ---- Recursive insert ---- */

void VanEmdeBoas5::insertImpl(int ci, int key)
{
    auto& c = m_clusters[ci];
    int lo = low(key), hi = high(key);

    if (c.min < 0) {
        // Empty cluster
        c.min = c.max = key;
        return;
    }

    if (key == c.min) return; // Duplicate

    if (key < c.min) std::swap(key, c.min);

    int subSize = qMax(1, m_sqrtU / 2);

    if (hi < c.subClusters.size()) {
        if (c.subClusters[hi] < 0) {
            // Need to create sub-cluster
            c.subClusters[hi] = allocCluster(subSize);
            // Update summary
            if (c.summary < 0) c.summary = allocCluster(subSize);
            insertImpl(c.summary, hi);
        }
        insertImpl(c.subClusters[hi], lo);
    } else {
        // Key falls outside sub-cluster range: use overflow
        addToOverflow(ci, key);
    }

    if (key > c.max) c.max = key;
}

/* ---- Recursive remove ---- */

void VanEmdeBoas5::removeImpl(int ci, int key)
{
    auto& c = m_clusters[ci];
    if (c.min < 0) return;

    int hi = high(key), lo = low(key);

    if (key == c.min) {
        if (c.min == c.max) {
            c.min = c.max = -1;
            return;
        }

        // Find new min from summary
        int newMinHi = -1;
        if (c.summary >= 0 && m_clusters[c.summary].min >= 0)
            newMinHi = m_clusters[c.summary].min;

        if (newMinHi >= 0 && newMinHi < c.subClusters.size()
            && c.subClusters[newMinHi] >= 0) {
            int newMinLo = m_clusters[c.subClusters[newMinHi]].min;
            c.min = index(newMinHi, newMinLo);
            removeImpl(c.subClusters[newMinHi], newMinLo);
        } else if (!c.overflowKeys.isEmpty()) {
            // Get from overflow
            c.min = c.overflowKeys.first();
            c.overflowKeys.removeFirst();
        } else {
            c.min = c.max;
        }
    } else if (hi < c.subClusters.size() && c.subClusters[hi] >= 0) {
        removeImpl(c.subClusters[hi], lo);
        // If sub-cluster is now empty, update summary
        if (m_clusters[c.subClusters[hi]].min < 0) {
            if (c.summary >= 0) removeImpl(c.summary, hi);
        }
    } else {
        removeFromOverflow(ci, key);
    }

    // Update max
    if (c.min < 0) {
        c.max = -1;
    } else {
        int newMaxHi = -1;
        if (c.summary >= 0) newMaxHi = m_clusters[c.summary].max;
        if (newMaxHi >= 0 && newMaxHi < c.subClusters.size()
            && c.subClusters[newMaxHi] >= 0) {
            c.max = index(newMaxHi, m_clusters[c.subClusters[newMaxHi]].max);
        } else if (!c.overflowKeys.isEmpty()) {
            c.max = *std::max_element(c.overflowKeys.begin(), c.overflowKeys.end());
        } else {
            c.max = c.min;
        }
    }
}

/* ---- Recursive contains ---- */

bool VanEmdeBoas5::containsImpl(int ci, int key) const
{
    if (ci < 0 || ci >= m_clusters.size()) return false;
    const auto& c = m_clusters[ci];

    if (key == c.min || key == c.max) return true;
    if (c.min < 0 || key < c.min || key > c.max) return false;

    int hi = high(key);
    if (hi < c.subClusters.size() && c.subClusters[hi] >= 0)
        return containsImpl(c.subClusters[hi], low(key));

    return checkOverflow(ci, key);
}

/* ---- Recursive successor ---- */

int VanEmdeBoas5::successorImpl(int ci, int key) const
{
    if (ci < 0) return -1;
    const auto& c = m_clusters[ci];
    if (c.min < 0) return -1;

    // If key < min, min is the successor
    if (key < c.min) return c.min;
    if (key >= c.max) return -1;

    int hi = high(key), lo = low(key);

    // Check within same sub-cluster
    if (hi < c.subClusters.size() && c.subClusters[hi] >= 0) {
        const auto& sub = m_clusters[c.subClusters[hi]];
        if (sub.max >= 0 && lo < sub.max) {
            int subSucc = successorImpl(c.subClusters[hi], lo);
            if (subSucc >= 0) return index(hi, subSucc);
        }
    }

    // Check summary for next non-empty cluster
    if (c.summary >= 0) {
        int nextHi = successorImpl(c.summary, hi);
        if (nextHi >= 0 && nextHi < c.subClusters.size()
            && c.subClusters[nextHi] >= 0) {
            return index(nextHi, m_clusters[c.subClusters[nextHi]].min);
        }
    }

    return c.max;
}

/* ---- Recursive predecessor ---- */

int VanEmdeBoas5::predecessorImpl(int ci, int key) const
{
    if (ci < 0) return -1;
    const auto& c = m_clusters[ci];
    if (c.min < 0) return -1;

    if (key > c.max) return c.max;
    if (key <= c.min) return -1;

    int hi = high(key), lo = low(key);

    if (hi < c.subClusters.size() && c.subClusters[hi] >= 0) {
        const auto& sub = m_clusters[c.subClusters[hi]];
        if (sub.min >= 0 && lo > sub.min) {
            int subPred = predecessorImpl(c.subClusters[hi], lo);
            if (subPred >= 0) return index(hi, subPred);
        }
    }

    // Previous cluster via summary
    if (c.summary >= 0) {
        int prevHi = predecessorImpl(c.summary, hi);
        if (prevHi >= 0 && prevHi < c.subClusters.size()
            && c.subClusters[prevHi] >= 0) {
            return index(prevHi, m_clusters[c.subClusters[prevHi]].max);
        }
    }

    return c.min;
}

/* ---- Overflow helpers ---- */

bool VanEmdeBoas5::checkOverflow(int ci, int key) const
{
    return m_clusters[ci].overflowKeys.contains(key);
}

void VanEmdeBoas5::addToOverflow(int ci, int key)
{
    if (!m_clusters[ci].overflowKeys.contains(key)) {
        m_clusters[ci].overflowKeys.append(key);
        m_stats.numOverflow++;
    }
}

void VanEmdeBoas5::removeFromOverflow(int ci, int key)
{
    m_clusters[ci].overflowKeys.removeAll(key);
}

/* ---- Public interface ---- */

void VanEmdeBoas5::insert(int key)
{
    QElapsedTimer timer;
    timer.start();
    if (key < 0 || key >= m_universe) return;
    insertImpl(m_root, key);
    m_stats.numElements++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
    emit operationCompleted("insert", key, timer.elapsed());
}

void VanEmdeBoas5::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    if (key < 0 || key >= m_universe) return;
    if (!containsImpl(m_root, key)) return;
    removeImpl(m_root, key);
    m_stats.numElements--;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
    emit operationCompleted("remove", key, timer.elapsed());
}

bool VanEmdeBoas5::contains(int key) const
{
    if (key < 0 || key >= m_universe) return false;
    return containsImpl(m_root, key);
}

int VanEmdeBoas5::successor(int key) const
{
    return successorImpl(m_root, key);
}

int VanEmdeBoas5::predecessor(int key) const
{
    return predecessorImpl(m_root, key);
}

int VanEmdeBoas5::minimum() const
{
    return m_clusters[m_root].min;
}

int VanEmdeBoas5::maximum() const
{
    return m_clusters[m_root].max;
}

QVector<int> VanEmdeBoas5::rangeQuery(int lo, int hi) const
{
    QVector<int> result;
    int cur = successorImpl(m_root, lo - 1);
    while (cur >= 0 && cur <= hi) {
        result.append(cur);
        cur = successorImpl(m_root, cur);
    }
    return result;
}

void VanEmdeBoas5::clear()
{
    m_clusters.clear();
    m_root = allocCluster(m_sqrtU);
    m_stats.numElements = 0;
    m_stats.numOverflow = 0;
}

void VanEmdeBoas5::resetStatistics()
{
    m_stats = Stats{};
    m_stats.universeSize = m_universe;
    m_timeSum = 0.0;
    clear();
}
