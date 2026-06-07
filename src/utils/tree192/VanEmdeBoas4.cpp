/**
 * @file VanEmdeBoas4.cpp
 * @brief VanEmdeBoas4 实现
 *
 * 实现van Emde Boas树：簇层次、O(log log U)操作、前驱后继查询。
 */

#include "utils/tree192/VanEmdeBoas4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VanEmdeBoas4::VanEmdeBoas4(int universeSize, QObject *parent)
    : QObject(parent), m_universe(universeSize), m_count(0)
{
    // Round universe to power of 2
    int u = 2;
    while (u < m_universe) u *= 2;
    m_universe = u;
    m_root = new Node(m_universe);

    m_stats.universeSize = m_universe;
}

VanEmdeBoas4::~VanEmdeBoas4()
{
    deleteNode(m_root);
}

void VanEmdeBoas4::deleteNode(Node* node)
{
    if (!node) return;
    if (node->u > 2) {
        deleteNode(node->summary);
        for (auto* c : node->clusters)
            deleteNode(c);
    }
    delete node;
}

/* ---- Helper: split x into (high, low) ---- */

int VanEmdeBoas4::high(int x) const
{
    int ls = lowerSqrt(m_universe);
    // Use the root's universe for consistent splitting
    // Simplified: compute per-node in recursive calls
    Q_UNUSED(x)
    return 0;
}

int VanEmdeBoas4::low(int x) const
{
    Q_UNUSED(x)
    return 0;
}

int VanEmdeBoas4::index(int h, int l) const
{
    Q_UNUSED(h) Q_UNUSED(l)
    return 0;
}

/* ---- Recursive helpers with per-node splitting ---- */

// Helper functions using node-local splitting
static int nodeHigh(int x, int u)
{
    int ls = VanEmdeBoas4::lowerSqrt(u);
    return x / ls;
}

static int nodeLow(int x, int u)
{
    int ls = VanEmdeBoas4::lowerSqrt(u);
    return x % ls;
}

static int nodeIndex(int h, int l, int u)
{
    int ls = VanEmdeBoas4::lowerSqrt(u);
    return h * ls + l;
}

/* ---- Recursive insert ---- */

void VanEmdeBoas4::insertRec(Node* node, int x)
{
    if (node->min == -1) {
        // Empty node: set min and max
        node->min = x;
        node->max = x;
        return;
    }

    if (x < node->min) {
        std::swap(x, node->min);
    }

    if (node->u > 2) {
        int h = nodeHigh(x, node->u);
        int l = nodeLow(x, node->u);

        if (node->clusters[h] == nullptr) {
            int cu = upperSqrt(node->u);
            node->clusters[h] = new Node(lowerSqrt(node->u));
        }

        if (node->clusters[h]->min == -1) {
            // Need to insert into summary
            if (node->summary == nullptr) {
                node->summary = new Node(upperSqrt(node->u));
            }
            insertRec(node->summary, h);
            node->clusters[h]->min = l;
            node->clusters[h]->max = l;
        } else {
            insertRec(node->clusters[h], l);
        }
    }

    if (x > node->max) {
        node->max = x;
    }
}

/* ---- Recursive remove ---- */

void VanEmdeBoas4::removeRec(Node* node, int x)
{
    if (node->min == node->max) {
        // Only one element
        if (node->min == x) {
            node->min = -1;
            node->max = -1;
        }
        return;
    }

    if (node->u == 2) {
        // Base case: universe of size 2
        if (x == 0) {
            node->min = 1;
        } else {
            node->min = 0;
        }
        node->max = node->min;
        return;
    }

    if (x == node->min) {
        // Find new min from summary
        if (node->summary && node->summary->min != -1) {
            int firstCluster = node->summary->min;
            x = nodeIndex(firstCluster, node->clusters[firstCluster]->min,
                          node->u);
            node->min = x;
        } else {
            node->min = node->max;
            return;
        }
    }

    int h = nodeHigh(x, node->u);
    int l = nodeLow(x, node->u);

    if (node->clusters[h]) {
        removeRec(node->clusters[h], l);

        if (node->clusters[h]->min == -1) {
            // Cluster became empty, remove from summary
            if (node->summary) {
                removeRec(node->summary, h);
            }

            // Update max
            if (x == node->max) {
                if (node->summary && node->summary->max != -1) {
                    int summaryMax = node->summary->max;
                    node->max = nodeIndex(summaryMax,
                                          node->clusters[summaryMax]->max,
                                          node->u);
                } else {
                    node->max = node->min;
                }
            }
        } else if (x == node->max) {
            node->max = nodeIndex(h, node->clusters[h]->max, node->u);
        }
    }
}

/* ---- Recursive contains ---- */

bool VanEmdeBoas4::containsRec(Node* node, int x) const
{
    if (!node) return false;
    if (x == node->min || x == node->max) return true;
    if (node->u == 2) return false;

    int h = nodeHigh(x, node->u);
    int l = nodeLow(x, node->u);

    if (h < node->clusters.size() && node->clusters[h])
        return containsRec(node->clusters[h], l);
    return false;
}

/* ---- Recursive predecessor ---- */

int VanEmdeBoas4::predRec(Node* node, int x) const
{
    if (!node) return -1;

    if (node->u == 2) {
        if (x == 1 && node->min == 0) return 0;
        return -1;
    }

    if (node->max != -1 && x > node->max) return node->max;
    if (node->min != -1 && x <= node->min) return -1;

    int h = nodeHigh(x, node->u);
    int l = nodeLow(x, node->u);

    // Check if predecessor is in the same cluster
    if (h < node->clusters.size() && node->clusters[h] &&
        node->clusters[h]->min != -1 && l > node->clusters[h]->min) {
        int predLow = predRec(node->clusters[h], l);
        return nodeIndex(h, predLow, node->u);
    }

    // Predecessor is in a previous cluster
    int predCluster = -1;
    if (node->summary) {
        predCluster = predRec(node->summary, h);
    }

    if (predCluster == -1) {
        // No previous cluster; predecessor is node->min
        if (node->min != -1 && x > node->min) return node->min;
        return -1;
    }

    return nodeIndex(predCluster, node->clusters[predCluster]->max, node->u);
}

/* ---- Recursive successor ---- */

int VanEmdeBoas4::succRec(Node* node, int x) const
{
    if (!node) return -1;

    if (node->u == 2) {
        if (x == 0 && node->max == 1) return 1;
        return -1;
    }

    if (node->min != -1 && x < node->min) return node->min;
    if (node->max != -1 && x >= node->max) return -1;

    int h = nodeHigh(x, node->u);
    int l = nodeLow(x, node->u);

    // Check if successor is in the same cluster
    if (h < node->clusters.size() && node->clusters[h] &&
        node->clusters[h]->max != -1 && l < node->clusters[h]->max) {
        int succLow = succRec(node->clusters[h], l);
        return nodeIndex(h, succLow, node->u);
    }

    // Successor is in next cluster
    int succCluster = -1;
    if (node->summary) {
        succCluster = succRec(node->summary, h);
    }

    if (succCluster == -1) return -1;

    return nodeIndex(succCluster, node->clusters[succCluster]->min, node->u);
}

/* ---- Public API ---- */

void VanEmdeBoas4::insert(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (x < 0 || x >= m_universe) return;
    if (!containsRec(m_root, x)) {
        insertRec(m_root, x);
        m_count++;
        m_stats.insertCount++;
    }

    m_stats.totalOperations++;
    m_stats.numElements = m_count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", x, timer.elapsed());
}

void VanEmdeBoas4::remove(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (x < 0 || x >= m_universe) return;
    if (containsRec(m_root, x)) {
        removeRec(m_root, x);
        m_count--;
        m_stats.deleteCount++;
    }

    m_stats.totalOperations++;
    m_stats.numElements = m_count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", x, timer.elapsed());
}

bool VanEmdeBoas4::contains(int x) const
{
    if (x < 0 || x >= m_universe) return false;
    return containsRec(m_root, x);
}

int VanEmdeBoas4::predecessor(int x) const
{
    if (x < 0 || x >= m_universe) return -1;
    return predRec(m_root, x);
}

int VanEmdeBoas4::successor(int x) const
{
    if (x < 0 || x >= m_universe) return -1;
    return succRec(m_root, x);
}

int VanEmdeBoas4::minimum() const { return m_root ? m_root->min : -1; }
int VanEmdeBoas4::maximum() const { return m_root ? m_root->max : -1; }

/* ---- Reset ---- */

void VanEmdeBoas4::resetStatistics()
{
    m_stats = Stats{};
    m_stats.universeSize = m_universe;
    m_timeSum = 0.0;
    deleteNode(m_root);
    m_root = new Node(m_universe);
    m_count = 0;
}
