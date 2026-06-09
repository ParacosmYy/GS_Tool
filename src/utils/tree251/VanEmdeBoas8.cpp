/**
 * @file VanEmdeBoas8.cpp
 * @brief VanEmdeBoas8 实现
 *
 * 实现van Emde Boas树：哈希簇存储与破坏性合并稀疏宇宙内存优化。
 */

#include "utils/tree251/VanEmdeBoas8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VanEmdeBoas8::VanEmdeBoas8(int universeBits, QObject *parent)
    : QObject(parent), m_universeBits(qBound(1, universeBits, 24))
{
    m_universeSize = 1 << m_universeBits;
    m_root = allocNode(m_universeBits);
}

VanEmdeBoas8::~VanEmdeBoas8() { deleteNode(m_root); }

/* ---- Allocate vEB node ---- */

VanEmdeBoas8::VEBNode* VanEmdeBoas8::allocNode(int universeBits)
{
    auto* node = new VEBNode();
    node->universe = universeBits;
    node->min = -1;
    node->max = -1;
    node->summary = nullptr;
    // Clusters are hash-based, no pre-allocation needed
    return node;
}

/* ---- Recursively delete node ---- */

void VanEmdeBoas8::deleteNode(VEBNode* node)
{
    if (!node) return;
    deleteNode(node->summary);
    for (auto it = node->clusters.begin(); it != node->clusters.end(); ++it)
        deleteNode(it.value());
    node->clusters.clear();
    delete node;
}

/* ---- Recursive insert ---- */

void VanEmdeBoas8::insertRec(VEBNode* node, int key)
{
    if (node->min == -1) {
        // Empty node: just set min and max
        node->min = key;
        node->max = key;
        return;
    }

    if (key < node->min) std::swap(key, node->min);
    if (key > node->max) node->max = key;

    if (node->universe <= 1) return; // Base case

    int hi = node->high(key);
    int lo = node->low(key);

    // Lazy-create cluster and summary
    if (!node->clusters.contains(hi)) {
        int subBits = node->universe / 2;
        node->clusters[hi] = allocNode(subBits);
        if (!node->summary) {
            int sumBits = node->universe - node->universe / 2;
            node->summary = allocNode(sumBits);
        }
        insertRec(node->summary, hi);
    }

    // If cluster was empty, key becomes its min
    VEBNode* cluster = node->clusters[hi];
    if (cluster->min == -1) {
        cluster->min = lo;
        cluster->max = lo;
    } else {
        insertRec(cluster, lo);
    }
}

/* ---- Recursive remove ---- */

void VanEmdeBoas8::removeRec(VEBNode* node, int key)
{
    if (!node || node->min == -1) return;

    if (node->min == node->max) {
        if (node->min == key) {
            node->min = -1;
            node->max = -1;
        }
        return;
    }

    if (node->universe <= 1) {
        if (key == node->min) node->min = node->max;
        else if (key == node->max) node->max = node->min;
        return;
    }

    if (key == node->min) {
        // Replace min with first element in lowest non-empty cluster
        if (!node->summary || node->summary->min == -1) {
            node->min = node->max;
            return;
        }
        int firstCluster = node->summary->min;
        key = node->index(firstCluster, node->clusters[firstCluster]->min);
        node->min = key;
    }

    int hi = node->high(key);
    int lo = node->low(key);

    if (node->clusters.contains(hi)) {
        VEBNode* cluster = node->clusters[hi];
        removeRec(cluster, lo);

        // If cluster is now empty, remove from summary and reclaim memory
        if (cluster->min == -1) {
            // Destructive merge: absorb cluster memory
            removeRec(node->summary, hi);
            deleteNode(cluster);
            node->clusters.remove(hi);
            m_stats.numMerges++;
        }

        // Update max
        if (key == node->max) {
            if (!node->summary || node->summary->max == -1) {
                node->max = node->min;
            } else {
                int lastCluster = node->summary->max;
                node->max = node->index(lastCluster,
                    node->clusters[lastCluster]->max);
            }
        }
    }
}

/* ---- Recursive contains ---- */

bool VanEmdeBoas8::containsRec(const VEBNode* node, int key) const
{
    if (!node) return false;
    if (key == node->min || key == node->max) return true;
    if (node->universe <= 1) return false;

    int hi = node->high(key);
    if (!node->clusters.contains(hi)) return false;
    return containsRec(node->clusters[hi], node->low(key));
}

/* ---- Recursive successor ---- */

int VanEmdeBoas8::successorRec(const VEBNode* node, int key) const
{
    if (!node || node->min == -1) return -1;
    if (key < node->min) return node->min;
    if (node->universe <= 1) return (key < node->max) ? node->max : -1;

    int hi = node->high(key);
    int lo = node->low(key);

    // Check if successor is in the same cluster
    if (node->clusters.contains(hi) &&
        node->clusters[hi]->max != -1 && lo < node->clusters[hi]->max) {
        int offset = successorRec(node->clusters[hi], lo);
        return node->index(hi, offset);
    }

    // Successor is in the next non-empty cluster
    if (!node->summary) return -1;
    int succCluster = successorRec(node->summary, hi);
    if (succCluster == -1) return -1;
    return node->index(succCluster, node->clusters[succCluster]->min);
}

/* ---- Recursive predecessor ---- */

int VanEmdeBoas8::predecessorRec(const VEBNode* node, int key) const
{
    if (!node || node->min == -1) return -1;
    if (key > node->max) return node->max;
    if (node->universe <= 1) return (key > node->min) ? node->min : -1;

    int hi = node->high(key);
    int lo = node->low(key);

    if (node->clusters.contains(hi) &&
        node->clusters[hi]->min != -1 && lo > node->clusters[hi]->min) {
        int offset = predecessorRec(node->clusters[hi], lo);
        return node->index(hi, offset);
    }

    if (!node->summary) return node->min;
    int predCluster = predecessorRec(node->summary, hi);
    if (predCluster == -1) {
        return (node->min < key) ? node->min : -1;
    }
    return node->index(predCluster, node->clusters[predCluster]->max);
}

/* ---- Destructive merge ---- */

void VanEmdeBoas8::destructiveMerge(VEBNode* target, VEBNode* source)
{
    if (!source || source->min == -1) return;
    // Collect all elements from source, insert into target, then delete source
    QVector<int> elems;
    collectElements(source, elems);
    for (int key : elems) insertRec(target, key);
    deleteNode(source);
    m_stats.numMerges++;
    emit mergeCompleted(source->universe, target->universe, elems.size());
}

/* ---- Collect elements recursively ---- */

void VanEmdeBoas8::collectElements(const VEBNode* node,
                                      QVector<int>& result) const
{
    if (!node || node->min == -1) return;
    if (node->universe <= 1 || node->min == node->max) {
        result.append(node->min);
        return;
    }
    result.append(node->min);
    for (auto it = node->clusters.begin(); it != node->clusters.end(); ++it) {
        collectElements(it.value(), result);
    }
}

/* ---- Public API ---- */

void VanEmdeBoas8::insert(int key)
{
    QElapsedTimer timer;
    timer.start();
    if (key < 0 || key >= m_universeSize) return;
    if (!contains(key)) {
        insertRec(m_root, key);
        m_stats.numElements++;
    }
    m_stats.numInsertions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

void VanEmdeBoas8::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    if (key < 0 || key >= m_universeSize) return;
    if (contains(key)) {
        removeRec(m_root, key);
        m_stats.numElements--;
    }
    m_stats.numDeletions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

bool VanEmdeBoas8::contains(int key) const
{
    if (key < 0 || key >= m_universeSize) return false;
    bool result = containsRec(m_root, key);
    const_cast<VanEmdeBoas8*>(this)->m_stats.numSearches++;
    return result;
}

int VanEmdeBoas8::successor(int key) const
{
    if (key < 0 || key >= m_universeSize) return -1;
    return successorRec(m_root, key);
}

int VanEmdeBoas8::predecessor(int key) const
{
    if (key < 0 || key >= m_universeSize) return -1;
    return predecessorRec(m_root, key);
}

int VanEmdeBoas8::minimum() const { return m_root ? m_root->min : -1; }
int VanEmdeBoas8::maximum() const { return m_root ? m_root->max : -1; }

QVector<int> VanEmdeBoas8::elements() const
{
    QVector<int> result;
    collectElements(m_root, result);
    std::sort(result.begin(), result.end());
    return result;
}

void VanEmdeBoas8::clear()
{
    deleteNode(m_root);
    m_root = allocNode(m_universeBits);
    m_stats.numElements = 0;
    m_stats.numClusters = 0;
}

/* ---- Reset ---- */

void VanEmdeBoas8::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_stats.universeSize = m_universeSize;
    m_timeSum = 0.0;
}
