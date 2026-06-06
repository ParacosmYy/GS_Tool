/**
 * @file VanEmdeBoas.cpp
 * @brief VanEmdeBoas 实现
 *
 * 实现van Emde Boas树：递归簇结构、O(log log U)操作。
 */

#include "utils/tree166/VanEmdeBoas.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- VEBNode ---- */

VanEmdeBoas::VEBNode::VEBNode(int u)
    : universeSize(u), min(-1), max(-1), summary(nullptr)
{
    if (u > 2) {
        int sub = qSqrt(u);
        /* Round up to power of 2 for sub-universe */
        int high = 1;
        while (high < sub) high <<= 1;
        int clusterCount = high;
        int subSize = (u + clusterCount - 1) / clusterCount;

        summary = new VEBNode(clusterCount);
        cluster.resize(clusterCount, nullptr);
        for (int i = 0; i < clusterCount; ++i) {
            cluster[i] = new VEBNode(subSize);
        }
    }
}

VanEmdeBoas::VEBNode::~VEBNode()
{
    delete summary;
    for (auto* c : cluster) delete c;
}

/* ---- VanEmdeBoas ---- */

VanEmdeBoas::VanEmdeBoas(QObject* parent)
    : QObject(parent)
{
}

VanEmdeBoas::~VanEmdeBoas()
{
    delete m_root;
}

void VanEmdeBoas::setUniverseSize(int U)
{
    delete m_root;
    m_universeSize = 1;
    while (m_universeSize < U) m_universeSize <<= 1;
    m_universeSize = qMax(2, m_universeSize);
    m_root = new VEBNode(m_universeSize);
}

int VanEmdeBoas::lowerSqrt(int u) const
{
    int s = 1;
    while (s * s < u) s <<= 1;
    return s;
}

int VanEmdeBoas::high(VEBNode* node, int x) const
{
    if (node->universeSize <= 2) return 0;
    int subSize = lowerSqrt(node->universeSize);
    int clusterCount = (node->universeSize + subSize - 1) / subSize;
    /* Actually use consistent sqrt */
    int s = 1;
    while (s * s < node->universeSize) s <<= 1;
    return x / s;
}

int VanEmdeBoas::low(VEBNode* node, int x) const
{
    if (node->universeSize <= 2) return x;
    int s = 1;
    while (s * s < node->universeSize) s <<= 1;
    return x % s;
}

int VanEmdeBoas::index(VEBNode* node, int h, int l) const
{
    if (node->universeSize <= 2) return l;
    int s = 1;
    while (s * s < node->universeSize) s <<= 1;
    return h * s + l;
}

void VanEmdeBoas::insertRec(VEBNode* node, int x)
{
    if (x < 0 || x >= node->universeSize) return;

    if (node->min == -1) {
        /* Empty node */
        node->min = x;
        node->max = x;
        return;
    }

    if (x < node->min) {
        std::swap(x, node->min);
    }

    if (node->universeSize > 2) {
        int hi = high(node, x);
        int lo = low(node, x);
        if (node->cluster[hi]->min == -1) {
            insertRec(node->summary, hi);
            node->cluster[hi]->min = lo;
            node->cluster[hi]->max = lo;
        } else {
            insertRec(node->cluster[hi], lo);
        }
    }

    if (x > node->max) node->max = x;
}

void VanEmdeBoas::removeRec(VEBNode* node, int x)
{
    if (node->min == -1) return;

    if (node->min == node->max) {
        /* Single element */
        if (x == node->min) {
            node->min = -1;
            node->max = -1;
        }
        return;
    }

    if (node->universeSize <= 2) {
        /* Base case: universe size 2 */
        if (x == 0) {
            node->min = (node->max == 0) ? -1 : 1;
        } else {
            node->max = (node->min == 1) ? -1 : 0;
        }
        if (node->min == -1) node->max = -1;
        else if (node->max == -1) node->max = node->min;
        return;
    }

    if (x == node->min) {
        /* Find new min from clusters */
        int firstCluster = (node->summary) ? node->summary->min : -1;
        if (firstCluster == -1) {
            node->min = node->max;
            return;
        }
        int newMin = index(node, firstCluster, node->cluster[firstCluster]->min);
        node->min = newMin;
        x = newMin;
    }

    int hi = high(node, x);
    int lo = low(node, x);
    if (hi < node->cluster.size() && node->cluster[hi]) {
        removeRec(node->cluster[hi], lo);

        if (node->cluster[hi]->min == -1) {
            removeRec(node->summary, hi);

            /* Update max */
            if (node->max == x) {
                int summaryMax = (node->summary) ? node->summary->max : -1;
                if (summaryMax == -1) {
                    node->max = node->min;
                } else {
                    node->max = index(node, summaryMax, node->cluster[summaryMax]->max);
                }
            }
        } else if (node->max == x) {
            node->max = index(node, hi, node->cluster[hi]->max);
        }
    }
}

int VanEmdeBoas::successorRec(VEBNode* node, int x) const
{
    if (!node || node->min == -1) return -1;

    if (x < node->min) return node->min;

    if (node->universeSize <= 2) {
        if (x < 1 && node->max == 1) return 1;
        return -1;
    }

    int hi = high(node, x);
    int lo = low(node, x);

    if (hi < node->cluster.size() && node->cluster[hi]
        && node->cluster[hi]->max != -1 && lo <= node->cluster[hi]->max) {
        /* Successor in same cluster */
        return index(node, hi, successorRec(node->cluster[hi], lo));
    }

    /* Successor in next cluster */
    int nextCluster = successorRec(node->summary, hi);
    if (nextCluster == -1) return -1;

    return index(node, nextCluster, node->cluster[nextCluster]->min);
}

int VanEmdeBoas::predecessorRec(VEBNode* node, int x) const
{
    if (!node || node->min == -1) return -1;

    if (x > node->max) return node->max;

    if (node->universeSize <= 2) {
        if (x > 0 && node->min == 0) return 0;
        return -1;
    }

    int hi = high(node, x);
    int lo = low(node, x);

    if (hi < node->cluster.size() && node->cluster[hi]
        && node->cluster[hi]->min != -1 && lo >= node->cluster[hi]->min) {
        return index(node, hi, predecessorRec(node->cluster[hi], lo));
    }

    int prevCluster = predecessorRec(node->summary, hi);
    if (prevCluster == -1) {
        if (node->min < x) return node->min;
        return -1;
    }

    return index(node, prevCluster, node->cluster[prevCluster]->max);
}

bool VanEmdeBoas::containsRec(VEBNode* node, int x) const
{
    if (!node || node->min == -1) return false;
    if (x == node->min || x == node->max) return true;
    if (node->universeSize <= 2) return false;

    int hi = high(node, x);
    int lo = low(node, x);
    if (hi >= node->cluster.size()) return false;
    return containsRec(node->cluster[hi], lo);
}

bool VanEmdeBoas::insert(int x)
{
    if (!m_root || x < 0 || x >= m_universeSize) return false;
    if (contains(x)) return false;

    QElapsedTimer timer;
    timer.start();

    insertRec(m_root, x);
    m_stats.currentSize++;
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(x, m_stats.currentSize);
    return true;
}

bool VanEmdeBoas::remove(int x)
{
    if (!m_root || !contains(x)) return false;

    QElapsedTimer timer;
    timer.start();

    removeRec(m_root, x);
    m_stats.currentSize--;
    m_stats.totalDeletes++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit deleteCompleted(x);
    return true;
}

bool VanEmdeBoas::contains(int x) const
{
    return containsRec(m_root, x);
}

int VanEmdeBoas::successor(int x) const
{
    return successorRec(m_root, x);
}

int VanEmdeBoas::predecessor(int x) const
{
    return predecessorRec(m_root, x);
}

int VanEmdeBoas::minimum() const
{
    return m_root ? m_root->min : -1;
}

int VanEmdeBoas::maximum() const
{
    return m_root ? m_root->max : -1;
}

bool VanEmdeBoas::isEmpty() const
{
    return !m_root || m_root->min == -1;
}

void VanEmdeBoas::clear()
{
    delete m_root;
    m_root = new VEBNode(m_universeSize);
    m_stats.currentSize = 0;
}

void VanEmdeBoas::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
