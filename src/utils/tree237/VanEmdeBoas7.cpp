/**
 * @file VanEmdeBoas7.cpp
 * @brief VanEmdeBoas7 实现
 *
 * 实现van Emde Boas树：簇递归结构与自底向上最小值查找。
 */

#include "utils/tree237/VanEmdeBoas7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VanEmdeBoas7::VanEmdeBoas7(QObject *parent) : QObject(parent) {}
VanEmdeBoas7::~VanEmdeBoas7() { destroyNode(m_root); }

/* ---- sqrt helpers ---- */

int VanEmdeBoas7::sqrtCeil(int u)
{
    int s = static_cast<int>(qCeil(qSqrt(static_cast<double>(u))));
    return qMax(2, s);
}

int VanEmdeBoas7::sqrtFloor(int u)
{
    int s = static_cast<int>(qFloor(qSqrt(static_cast<double>(u))));
    return qMax(2, s);
}

/* ---- Create vEB node ---- */

VanEmdeBoas7::VEBNode* VanEmdeBoas7::createNode(int u) const
{
    if (u < 2) return nullptr;

    VEBNode* node = new VEBNode();
    node->universe = u;
    node->min = -1;
    node->max = -1;
    node->minValue = 0.0;

    if (u == 2) {
        // Base case: no clusters, no summary
        node->sqrtU = 2;
        node->values.resize(2, 0.0);
        return node;
    }

    int highBits = sqrtCeil(u);
    int lowBits = sqrtFloor(u);
    // Ensure lowBits * highBits >= u
    while (lowBits * highBits < u) lowBits++;
    node->sqrtU = highBits;

    int numClusters = highBits;
    node->clusters.resize(numClusters, nullptr);
    node->values.resize(lowBits * numClusters, 0.0);

    // Summary tracks which clusters are non-empty
    node->summary = createNode(highBits);

    return node;
}

/* ---- Destroy vEB node ---- */

void VanEmdeBoas7::destroyNode(VEBNode* node)
{
    if (!node) return;
    destroyNode(node->summary);
    for (auto* child : node->clusters)
        destroyNode(child);
    delete node;
}

/* ---- Index decomposition ---- */

int VanEmdeBoas7::high(int x, int u) const
{
    if (u <= 2) return 0;
    return x / sqrtFloor(u);
}

int VanEmdeBoas7::low(int x, int u) const
{
    if (u <= 2) return x;
    return x % sqrtFloor(u);
}

int VanEmdeBoas7::index(int h, int l, int u) const
{
    if (u <= 2) return h;
    return h * sqrtFloor(u) + l;
}

/* ---- Init ---- */

bool VanEmdeBoas7::init(int universeSize)
{
    if (universeSize < 2) return false;

    // Round up to next power of 2
    int u = 1;
    while (u < universeSize) u <<= 1;

    destroyNode(m_root);
    m_universe = u;
    m_root = createNode(u);
    m_stats.universeSize = u;
    m_stats.numElements = 0;
    return m_root != nullptr;
}

/* ---- Recursive insert ---- */

void VanEmdeBoas7::insertRec(VEBNode* node, int key, double value)
{
    if (!node) return;

    if (node->min == -1) {
        // Empty node: store directly as min
        node->min = key;
        node->max = key;
        node->minValue = value;
        return;
    }

    if (key == node->min) {
        node->minValue = value;
        return;
    }

    if (key < node->min) {
        // Swap: new key becomes min, old min goes to cluster
        std::swap(key, node->min);
        std::swap(value, node->minValue);
    }

    int h = high(key, node->universe);
    int l = low(key, node->universe);

    if (node->universe > 2) {
        if (!node->clusters[h]) {
            node->clusters[h] = createNode(sqrtFloor(node->universe));
        }

        if (node->clusters[h]->min == -1) {
            // Cluster was empty: update summary
            insertRec(node->summary, h, 0.0);
        }

        insertRec(node->clusters[h], l, value);
    }

    // Update max
    if (key > node->max)
        node->max = key;
}

/* ---- Recursive remove ---- */

void VanEmdeBoas7::removeRec(VEBNode* node, int key)
{
    if (!node || node->min == -1) return;

    if (node->min == node->max) {
        // Only one element
        if (node->min == key) {
            node->min = -1;
            node->max = -1;
        }
        return;
    }

    if (node->universe == 2) {
        // Base case: two elements
        if (key == 0) {
            node->min = 1;
        } else {
            node->min = 0;
        }
        node->max = node->min;
        return;
    }

    if (key == node->min) {
        // Find new min from summary (bottom-up minimum finding)
        int h = node->summary->min;
        if (h == -1) {
            // No clusters have elements; this shouldn't happen since max != min
            node->min = node->max;
            return;
        }
        int l = node->clusters[h]->min;
        node->min = index(h, l, node->universe);
        node->minValue = node->clusters[h]->minValue;
        key = node->min;
    }

    int h = high(key, node->universe);
    int l = low(key, node->universe);

    if (node->universe > 2 && node->clusters[h]) {
        removeRec(node->clusters[h], l);

        if (node->clusters[h]->min == -1) {
            // Cluster became empty
            removeRec(node->summary, h);
        }
    }

    // Update max (bottom-up)
    if (key == node->max) {
        if (node->summary && node->summary->max != -1) {
            int maxH = node->summary->max;
            int maxL = node->clusters[maxH]->max;
            node->max = index(maxH, maxL, node->universe);
        } else {
            node->max = node->min;
        }
    }
}

/* ---- Recursive contains ---- */

bool VanEmdeBoas7::containsRec(VEBNode* node, int key) const
{
    if (!node) return false;
    if (node->min == key || node->max == key) return true;
    if (node->universe == 2) return false;

    int h = high(key, node->universe);
    int l = low(key, node->universe);

    if (h < node->clusters.size() && node->clusters[h])
        return containsRec(node->clusters[h], l);
    return false;
}

/* ---- Recursive successor ---- */

int VanEmdeBoas7::successorRec(VEBNode* node, int key) const
{
    if (!node || node->min == -1) return -1;

    if (node->universe == 2) {
        if (key == 0 && node->max == 1) return 1;
        return -1;
    }

    // If key < min, min is the successor
    if (key < node->min) return node->min;

    int h = high(key, node->universe);
    int l = low(key, node->universe);
    int sqrtLow = sqrtFloor(node->universe);

    // Check within same cluster
    if (h < node->clusters.size() && node->clusters[h] && node->clusters[h]->max != -1 && l < node->clusters[h]->max) {
        int s = successorRec(node->clusters[h], l);
        if (s != -1) return index(h, s, node->universe);
    }

    // Check summary for next non-empty cluster
    int nextH = node->summary ? successorRec(node->summary, h) : -1;
    if (nextH != -1) {
        int minL = node->clusters[nextH]->min;
        return index(nextH, minL, node->universe);
    }

    return -1;
}

/* ---- Recursive predecessor ---- */

int VanEmdeBoas7::predecessorRec(VEBNode* node, int key) const
{
    if (!node || node->min == -1) return -1;

    if (node->universe == 2) {
        if (key == 1 && node->min == 0) return 0;
        return -1;
    }

    if (key > node->max) return node->max;

    int h = high(key, node->universe);
    int l = low(key, node->universe);

    // Check within same cluster
    if (h < node->clusters.size() && node->clusters[h] && node->clusters[h]->min != -1 && l > node->clusters[h]->min) {
        int p = predecessorRec(node->clusters[h], l);
        if (p != -1) return index(h, p, node->universe);
    }

    // Check summary for previous non-empty cluster
    int prevH = node->summary ? predecessorRec(node->summary, h) : -1;
    if (prevH != -1) {
        int maxL = node->clusters[prevH]->max;
        return index(prevH, maxL, node->universe);
    }

    // If nothing in previous clusters, it's the min (if key > min)
    if (node->min != -1 && key > node->min) return node->min;
    return -1;
}

/* ---- Collect keys ---- */

void VanEmdeBoas7::collectKeys(VEBNode* node, int offset, QVector<int>& keys) const
{
    if (!node || node->min == -1) return;
    keys.append(offset + node->min);
    if (node->universe > 2) {
        for (int i = 0; i < node->clusters.size(); ++i) {
            if (node->clusters[i] && node->clusters[i]->min != -1) {
                int childOffset = offset + i * sqrtFloor(node->universe);
                collectKeys(node->clusters[i], childOffset, keys);
            }
        }
    } else {
        if (node->max != node->min)
            keys.append(offset + node->max);
    }
}

/* ---- Public API ---- */

void VanEmdeBoas7::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root || key < 0 || key >= m_universe) return;

    bool isNew = !containsRec(m_root, key);
    insertRec(m_root, key, value);

    if (isNew) {
        m_stats.numElements++;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit keyInserted(key);
    }
}

void VanEmdeBoas7::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root || key < 0 || key >= m_universe) return;
    if (!containsRec(m_root, key)) return;

    removeRec(m_root, key);
    m_stats.numElements--;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit keyRemoved(key);
}

bool VanEmdeBoas7::contains(int key) const
{
    if (!m_root || key < 0 || key >= m_universe) return false;
    return containsRec(m_root, key);
}

double VanEmdeBoas7::value(int key) const
{
    if (!m_root || key < 0 || key >= m_universe)
        return std::numeric_limits<double>::quiet_NaN();

    VEBNode* node = m_root;
    while (node) {
        if (key == node->min) return node->minValue;
        if (node->universe == 2) {
            return (key == node->max) ? node->values[1] : std::numeric_limits<double>::quiet_NaN();
        }
        int h = high(key, node->universe);
        int l = low(key, node->universe);
        if (h >= node->clusters.size() || !node->clusters[h])
            return std::numeric_limits<double>::quiet_NaN();
        node = node->clusters[h];
        key = l;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

int VanEmdeBoas7::successor(int key) const
{
    if (!m_root) return -1;
    return successorRec(m_root, key);
}

int VanEmdeBoas7::predecessor(int key) const
{
    if (!m_root) return -1;
    return predecessorRec(m_root, key);
}

int VanEmdeBoas7::minimum() const { return m_root ? m_root->min : -1; }
int VanEmdeBoas7::maximum() const { return m_root ? m_root->max : -1; }

QVector<int> VanEmdeBoas7::allKeys() const
{
    QVector<int> keys;
    collectKeys(m_root, 0, keys);
    std::sort(keys.begin(), keys.end());
    return keys;
}

/* ---- Reset ---- */

void VanEmdeBoas7::resetStatistics()
{
    destroyNode(m_root);
    m_root = nullptr;
    m_universe = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
