/**
 * @file VanEmdeBoas6.cpp
 * @brief VanEmdeBoas6 实现
 *
 * 实现vEB树：哈希簇存储、自底向上递归find-next、稀疏宇宙优化。
 */

#include "utils/tree223/VanEmdeBoas6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VanEmdeBoas6::VanEmdeBoas6(QObject *parent) : QObject(parent) {}
VanEmdeBoas6::~VanEmdeBoas6() { delete m_root; }

/* ---- Allocate vEB node ---- */

VanEmdeBoas6::VEBNode* VanEmdeBoas6::allocNode(int u)
{
    VEBNode* node = new VEBNode();
    node->universe = u;
    node->sqrtU = qCeil(qSqrt(u));
    node->min = -1;
    node->max = -1;
    return node;
}

/* ---- Initialize ---- */

void VanEmdeBoas6::init(int universeSize)
{
    delete m_root;
    // Round up to next power of 2
    int u = 2;
    while (u < universeSize && u > 0) u *= 2;
    if (u < 2) u = 2;
    m_universe = u;
    m_root = allocNode(u);
    m_stats.universeSize = u;
    m_stats.numElements = 0;
}

/* ---- Recursive insert ---- */

void VanEmdeBoas6::insertRec(VEBNode* node, int key)
{
    if (key < 0 || key >= node->universe) return;

    if (node->min == -1) {
        // Empty node
        node->min = key;
        node->max = key;
        return;
    }

    if (key == node->min) return; // Duplicate

    if (key < node->min) {
        // Swap key with min
        int temp = key;
        key = node->min;
        node->min = temp;
    }

    if (node->universe > 2) {
        int hi = node->high(key);
        int lo = node->low(key);

        // Lazy allocate summary
        if (!node->summary)
            node->summary = allocNode(node->sqrtU);

        // Lazy allocate cluster
        if (!node->cluster.contains(hi))
            node->cluster[hi] = allocNode(node->sqrtU);

        if (node->cluster[hi]->min == -1) {
            // Cluster was empty, insert into summary
            insertRec(node->summary, hi);
            node->cluster[hi]->min = lo;
            node->cluster[hi]->max = lo;
        } else {
            insertRec(node->cluster[hi], lo);
        }
    }

    if (key > node->max) node->max = key;
}

/* ---- Recursive remove ---- */

bool VanEmdeBoas6::removeRec(VEBNode* node, int key)
{
    if (!node || node->min == -1) return false;
    if (key < node->min || key > node->max) return false;

    if (node->min == node->max) {
        if (node->min == key) {
            node->min = -1;
            node->max = -1;
            return true;
        }
        return false;
    }

    int u = node->universe;
    if (u == 2) {
        // Base case: universe size 2
        if (key == 0) {
            if (node->min == 0) node->min = 1;
            if (node->max == 0) node->max = node->min;
        } else {
            if (node->min == 1) node->min = 0;
            if (node->max == 1) node->max = node->min;
        }
        return true;
    }

    if (key == node->min) {
        // Replace min with smallest in first non-empty cluster
        int firstCluster = node->summary ? node->summary->min : -1;
        if (firstCluster == -1) {
            // Only max remains
            node->min = node->max;
            return true;
        }
        int loMin = node->cluster.contains(firstCluster)
            ? node->cluster[firstCluster]->min : 0;
        key = node->index(firstCluster, loMin);
        node->min = key;
    }

    int hi = node->high(key);
    int lo = node->low(key);

    if (node->cluster.contains(hi)) {
        bool removed = removeRec(node->cluster[hi], lo);
        if (removed && node->cluster[hi]->min == -1) {
            // Cluster became empty
            delete node->cluster.take(hi);
            removeRec(node->summary, hi);

            if (key == node->max) {
                int maxCluster = node->summary ? node->summary->max : -1;
                if (maxCluster == -1) {
                    node->max = node->min;
                } else {
                    node->max = node->index(maxCluster,
                        node->cluster.contains(maxCluster)
                            ? node->cluster[maxCluster]->max : 0);
                }
            }
        } else if (key == node->max) {
            // Update max within cluster
            if (node->cluster.contains(hi)) {
                node->max = node->index(hi, node->cluster[hi]->max);
            }
        }
    }
    return true;
}

/* ---- Recursive membership ---- */

bool VanEmdeBoas6::containsRec(const VEBNode* node, int key) const
{
    if (!node || node->min == -1) return false;
    if (key == node->min || key == node->max) return true;
    if (key < node->min || key > node->max) return false;
    if (node->universe <= 2) return false;

    int hi = node->high(key);
    int lo = node->low(key);
    if (!node->cluster.contains(hi)) return false;
    return containsRec(node->cluster[hi], lo);
}

/* ---- Bottom-up find-next ---- */

int VanEmdeBoas6::findNextRec(const VEBNode* node, int key) const
{
    if (!node || node->min == -1) return -1;
    if (key < node->min) return node->min;
    if (key >= node->max) return -1;

    if (node->universe <= 2) {
        // Base case: if key < max and universe=2, successor is max
        return (key < node->max) ? node->max : -1;
    }

    int hi = node->high(key);
    int lo = node->low(key);

    // Bottom-up: check if successor is in same cluster
    if (node->cluster.contains(hi) && node->cluster[hi]->max != -1
        && lo < node->cluster[hi]->max) {
        int succLo = findNextRec(node->cluster[hi], lo);
        return (succLo != -1) ? node->index(hi, succLo) : -1;
    }

    // Bottom-up: recurse into summary to find next non-empty cluster
    int nextCluster = node->summary ? findNextRec(node->summary, hi) : -1;
    if (nextCluster == -1) return node->max;

    int minInCluster = node->cluster.contains(nextCluster)
        ? node->cluster[nextCluster]->min : 0;
    return node->index(nextCluster, minInCluster);
}

/* ---- Bottom-up find-prev ---- */

int VanEmdeBoas6::findPrevRec(const VEBNode* node, int key) const
{
    if (!node || node->min == -1) return -1;
    if (key > node->max) return node->max;
    if (key <= node->min) return -1;

    if (node->universe <= 2) {
        return (key > node->min) ? node->min : -1;
    }

    int hi = node->high(key);
    int lo = node->low(key);

    if (node->cluster.contains(hi) && node->cluster[hi]->min != -1
        && lo > node->cluster[hi]->min) {
        int predLo = findPrevRec(node->cluster[hi], lo);
        return (predLo != -1) ? node->index(hi, predLo) : -1;
    }

    int prevCluster = node->summary ? findPrevRec(node->summary, hi) : -1;
    if (prevCluster == -1) return node->min;

    int maxInCluster = node->cluster.contains(prevCluster)
        ? node->cluster[prevCluster]->max : 0;
    return node->index(prevCluster, maxInCluster);
}

/* ---- Insert ---- */

void VanEmdeBoas6::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) init(m_universe > 0 ? m_universe : 16);
    if (key < 0 || key >= m_universe) return;
    if (!contains(key)) {
        insertRec(m_root, key);
        m_stats.numElements++;
    }
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.numClusters = countClusters(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Remove ---- */

bool VanEmdeBoas6::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root || !contains(key)) return false;
    bool removed = removeRec(m_root, key);
    if (removed) m_stats.numElements--;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.numClusters = countClusters(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", key, timer.elapsed());
    return removed;
}

/* ---- Contains ---- */

bool VanEmdeBoas6::contains(int key) const
{
    return containsRec(m_root, key);
}

/* ---- Find-next ---- */

int VanEmdeBoas6::findNext(int key) const
{
    return findNextRec(m_root, key);
}

/* ---- Find-prev ---- */

int VanEmdeBoas6::findPrev(int key) const
{
    return findPrevRec(m_root, key);
}

/* ---- Min / Max ---- */

int VanEmdeBoas6::minimum() const { return m_root ? m_root->min : -1; }
int VanEmdeBoas6::maximum() const { return m_root ? m_root->max : -1; }

/* ---- Collect elements ---- */

void VanEmdeBoas6::collectElements(const VEBNode* node, int base,
                                      QVector<int>& result) const
{
    if (!node || node->min == -1) return;
    // Collect min
    result.append(base + node->min);
    // Recurse into clusters
    for (auto it = node->cluster.constBegin(); it != node->cluster.constEnd(); ++it) {
        int clusterBase = base + it.key() * node->sqrtU;
        collectElements(it.value(), clusterBase, result);
    }
}

QVector<int> VanEmdeBoas6::elements() const
{
    QVector<int> result;
    collectElements(m_root, 0, result);
    std::sort(result.begin(), result.end());
    return result;
}

/* ---- Compute height ---- */

int VanEmdeBoas6::computeHeight(const VEBNode* node) const
{
    if (!node) return 0;
    int h = 1;
    int maxSub = node->summary ? computeHeight(node->summary) : 0;
    for (auto it = node->cluster.constBegin(); it != node->cluster.constEnd(); ++it)
        maxSub = qMax(maxSub, computeHeight(it.value()));
    return h + maxSub;
}

/* ---- Count clusters ---- */

int VanEmdeBoas6::countClusters(const VEBNode* node) const
{
    if (!node) return 0;
    int count = node->cluster.size();
    for (auto it = node->cluster.constBegin(); it != node->cluster.constEnd(); ++it)
        count += countClusters(it.value());
    return count;
}

/* ---- Reset ---- */

void VanEmdeBoas6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    delete m_root;
    m_root = nullptr;
    m_universe = 0;
}
