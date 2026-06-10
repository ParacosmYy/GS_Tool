/**
 * @file VanEmdeBoas9.cpp
 * @brief VanEmdeBoas9 实现
 *
 * 实现van Emde Boas树：聚类底层数组与摘要递归O(lg lg U)前驱后继查询。
 */

#include "utils/tree265/VanEmdeBoas9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VanEmdeBoas9::VanEmdeBoas9(int universeSize, QObject *parent)
    : QObject(parent)
{
    m_universe = nextPower2(qMax(2, universeSize));
    m_root = createNode(m_universe);
}

VanEmdeBoas9::~VanEmdeBoas9()
{
    deleteNode(m_root);
}

/* ---- Utility ---- */

int VanEmdeBoas9::nextPower2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return qMax(2, p);
}

/* ---- Create node ---- */

VanEmdeBoas9::Node* VanEmdeBoas9::createNode(int u) const
{
    Node* node = new Node();
    node->universe = u;
    node->min = -1;
    node->max = -1;

    if (u > 2) {
        int upperSqrt = 1 << ((u + 1) / 2);
        int lowerSqrt = 1 << (u / 2);
        int numC = upperSqrt;
        node->clusters.resize(numC, nullptr);
        // Summary and clusters created lazily
    }
    return node;
}

/* ---- Delete node ---- */

void VanEmdeBoas9::deleteNode(Node* node)
{
    if (!node) return;
    if (node->summary) deleteNode(node->summary);
    for (auto* cluster : node->clusters)
        if (cluster) deleteNode(cluster);
    delete node;
}

/* ---- Compute height ---- */

int VanEmdeBoas9::computeHeight(int u) const
{
    int h = 0;
    while (u > 2) { u = (u + 1) / 2; h++; }
    return h;
}

/* ---- Insert recursively ---- */

void VanEmdeBoas9::insertRec(Node*& node, int x, int u)
{
    if (!node) node = createNode(u);

    if (node->min == -1) {
        // Empty node: store directly
        node->min = x;
        node->max = x;
        return;
    }

    if (x == node->min) return;  // Duplicate

    if (x < node->min) {
        // Swap: min always holds smallest
        int temp = x;
        x = node->min;
        node->min = temp;
    }

    if (u > 2) {
        int highBits = (u + 1) / 2;
        int upperSqrt = 1 << highBits;
        int lowerSqrt = 1 << (u / 2);
        int h = x >> (u / 2);
        int l = x & (lowerSqrt - 1);

        // Lazy-create cluster
        if (h >= node->clusters.size()) return;
        if (!node->clusters[h]) {
            node->clusters[h] = createNode(u / 2);
        }

        if (node->clusters[h]->min == -1) {
            // Lazy-create summary
            if (!node->summary) node->summary = createNode(highBits > 1 ? 1 << highBits : 2);
            insertRec(node->summary, h, upperSqrt);
        }
        insertRec(node->clusters[h], l, u / 2);
    }

    if (x > node->max) node->max = x;
}

/* ---- Remove recursively ---- */

void VanEmdeBoas9::removeRec(Node*& node, int x, int u)
{
    if (!node || node->min == -1) return;

    if (node->min == node->max) {
        if (node->min == x) {
            node->min = -1;
            node->max = -1;
            m_size--;
        }
        return;
    }

    if (u == 2) {
        // Base case: universe size 2
        if (x == 0) {
            node->min = 1;
        } else {
            node->min = 0;
        }
        node->max = node->min;
        m_size--;
        return;
    }

    int lowerSqrt = 1 << (u / 2);
    int h, l;

    if (x == node->min) {
        // Find the new min from summary
        if (node->summary && node->summary->min != -1) {
            h = node->summary->min;
            if (h < node->clusters.size() && node->clusters[h]) {
                l = node->clusters[h]->min;
                node->min = (h << (u / 2)) | l;
                x = node->min;
            }
        } else {
            // Only max remains
            node->min = node->max;
            node->max = node->min;
            m_size--;
            return;
        }
    }

    h = x >> (u / 2);
    l = x & (lowerSqrt - 1);

    if (h < node->clusters.size() && node->clusters[h]) {
        removeRec(node->clusters[h], l, u / 2);
    }

    // Update summary and max
    bool clusterEmpty = !node->clusters[h] || node->clusters[h]->min == -1;
    if (clusterEmpty && node->summary) {
        removeRec(node->summary, h, 1 << ((u + 1) / 2));
    }

    if (node->min != -1 && x == node->max) {
        if (!node->summary || node->summary->max == -1) {
            node->max = node->min;
        } else {
            int maxCluster = node->summary->max;
            if (maxCluster < node->clusters.size() && node->clusters[maxCluster]) {
                node->max = (maxCluster << (u / 2)) | node->clusters[maxCluster]->max;
            }
        }
    }
}

/* ---- Contains ---- */

bool VanEmdeBoas9::containsRec(Node* node, int x, int u) const
{
    if (!node) return false;
    if (x == node->min || x == node->max) return true;
    if (u == 2) return false;

    int h = x >> (u / 2);
    int l = x & ((1 << (u / 2)) - 1);
    if (h >= node->clusters.size() || !node->clusters[h]) return false;
    return containsRec(node->clusters[h], l, u / 2);
}

/* ---- Successor ---- */

int VanEmdeBoas9::successorRec(Node* node, int x, int u) const
{
    if (!node) return -1;
    if (u == 2) {
        if (x == 0 && node->max == 1) return 1;
        return -1;
    }

    if (node->min != -1 && x < node->min) return node->min;

    int lowerSqrt = 1 << (u / 2);
    int h = x >> (u / 2);
    int l = x & (lowerSqrt - 1);

    if (h < node->clusters.size() && node->clusters[h] &&
        node->clusters[h]->max != -1 && l < node->clusters[h]->max) {
        // Successor in same cluster
        int s = successorRec(node->clusters[h], l, u / 2);
        return (h << (u / 2)) | s;
    }

    // Find next non-empty cluster
    if (node->summary) {
        int sc = successorRec(node->summary, h, 1 << ((u + 1) / 2));
        if (sc != -1 && sc < node->clusters.size() && node->clusters[sc]) {
            int offset = node->clusters[sc]->min;
            return (sc << (u / 2)) | offset;
        }
    }
    return -1;
}

/* ---- Predecessor ---- */

int VanEmdeBoas9::predecessorRec(Node* node, int x, int u) const
{
    if (!node) return -1;
    if (u == 2) {
        if (x == 1 && node->min == 0) return 0;
        return -1;
    }

    if (node->max != -1 && x > node->max) return node->max;

    int lowerSqrt = 1 << (u / 2);
    int h = x >> (u / 2);
    int l = x & (lowerSqrt - 1);

    if (h < node->clusters.size() && node->clusters[h] &&
        node->clusters[h]->min != -1 && l > node->clusters[h]->min) {
        int p = predecessorRec(node->clusters[h], l, u / 2);
        return (h << (u / 2)) | p;
    }

    if (node->summary) {
        int pc = predecessorRec(node->summary, h, 1 << ((u + 1) / 2));
        if (pc != -1 && pc < node->clusters.size() && node->clusters[pc]) {
            int offset = node->clusters[pc]->max;
            return (pc << (u / 2)) | offset;
        }
    }

    // Could be the min itself
    if (node->min != -1 && x > node->min) return node->min;
    return -1;
}

/* ---- Collect elements ---- */

void VanEmdeBoas9::collectElements(Node* node, int base, int u,
                                     QVector<int>& result) const
{
    if (!node || node->min == -1) return;
    result.append(base | node->min);
    if (node->min != node->max) {
        if (u > 2) {
            int lowerSqrt = 1 << (u / 2);
            for (int i = 0; i < node->clusters.size(); ++i) {
                if (node->clusters[i]) {
                    collectElements(node->clusters[i], base | (i << (u / 2)), u / 2, result);
                }
            }
        } else {
            result.append(base | node->max);
        }
    }
}

/* ---- Public interface ---- */

void VanEmdeBoas9::insert(int x)
{
    QElapsedTimer timer;
    timer.start();
    if (x < 0 || x >= m_universe) return;
    if (!contains(x)) {
        insertRec(m_root, x, m_universe);
        m_size++;
    }
    double elapsed = timer.elapsed();
    m_stats.numElements = m_size;
    m_stats.universeSize = m_universe;
    m_stats.treeHeight = computeHeight(m_universe);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_universe, elapsed);
}

void VanEmdeBoas9::remove(int x)
{
    QElapsedTimer timer;
    timer.start();
    if (x < 0 || x >= m_universe || !contains(x)) return;
    removeRec(m_root, x, m_universe);
    double elapsed = timer.elapsed();
    m_stats.numElements = m_size;
    m_stats.universeSize = m_universe;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_universe, elapsed);
}

bool VanEmdeBoas9::contains(int x) const
{
    if (x < 0 || x >= m_universe) return false;
    return containsRec(m_root, x, m_universe);
}

int VanEmdeBoas9::successor(int x) const
{
    return successorRec(m_root, x, m_universe);
}

int VanEmdeBoas9::predecessor(int x) const
{
    return predecessorRec(m_root, x, m_universe);
}

int VanEmdeBoas9::minimum() const { return m_root ? m_root->min : -1; }
int VanEmdeBoas9::maximum() const { return m_root ? m_root->max : -1; }
int VanEmdeBoas9::size() const { return m_size; }

QVector<int> VanEmdeBoas9::elements() const
{
    QVector<int> result;
    collectElements(m_root, 0, m_universe, result);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

/* ---- Reset ---- */

void VanEmdeBoas9::resetStatistics()
{
    deleteNode(m_root);
    m_root = createNode(m_universe);
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
