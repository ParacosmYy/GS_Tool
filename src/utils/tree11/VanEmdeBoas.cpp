/**
 * @file VanEmdeBoas.cpp
 * @brief van Emde Boas树实现
 */

#include "VanEmdeBoas.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

VanEmdeBoas::VanEmdeBoas(int universeSize, QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_universeSize(universeSize)
    , m_count(0)
{
    /* 将universeSize向上取到最近的2的幂 */
    int u = 2;
    while (u < universeSize) u <<= 1;
    m_universeSize = u;

    m_root = createNode(m_universeSize);
}

VanEmdeBoas::~VanEmdeBoas()
{
    destroyNode(m_root);
}

void VanEmdeBoas::insert(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (x < 0 || x >= m_universeSize) return;
    if (!contains(x)) {
        insertRec(m_root, x);
        m_count++;
        m_stats.totalInsertions++;
    }

    m_timeSum += timer.elapsed();
    int total = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit elementInserted(x);
}

void VanEmdeBoas::remove(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (x < 0 || x >= m_universeSize) {
        m_timeSum += timer.elapsed();
        return;
    }

    if (contains(x)) {
        removeRec(m_root, x);
        m_count--;
        m_stats.totalDeletions++;
    }

    m_timeSum += timer.elapsed();
    int total = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit elementRemoved(x);
}

bool VanEmdeBoas::contains(int x) const
{
    QElapsedTimer timer;
    timer.start();

    bool result = false;
    if (x >= 0 && x < m_universeSize)
        result = containsRec(m_root, x);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

int VanEmdeBoas::predecessor(int x) const
{
    QElapsedTimer timer;
    timer.start();

    int result = -1;
    if (x >= 0 && x < m_universeSize)
        result = predecessorRec(m_root, x);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

int VanEmdeBoas::successor(int x) const
{
    QElapsedTimer timer;
    timer.start();

    int result = -1;
    if (x >= 0 && x < m_universeSize)
        result = successorRec(m_root, x);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

int VanEmdeBoas::minimum() const
{
    return minimumRec(m_root);
}

int VanEmdeBoas::maximum() const
{
    return maximumRec(m_root);
}

int VanEmdeBoas::size() const
{
    return m_count;
}

bool VanEmdeBoas::isEmpty() const
{
    return m_count == 0;
}

QVector<int> VanEmdeBoas::toSortedVector() const
{
    QVector<int> result;
    result.reserve(m_count);
    collectSorted(m_root, 0, result);
    return result;
}

int VanEmdeBoas::universeSize() const
{
    return m_universeSize;
}

int VanEmdeBoas::upperSqrt(int u) const
{
    return 1 << static_cast<int>(std::ceil(std::log2(u) / 2.0));
}

int VanEmdeBoas::lowerSqrt(int u) const
{
    return 1 << static_cast<int>(std::floor(std::log2(u) / 2.0));
}

int VanEmdeBoas::high(int x, int u) const
{
    return x / lowerSqrt(u);
}

int VanEmdeBoas::low(int x, int u) const
{
    return x % lowerSqrt(u);
}

int VanEmdeBoas::index(int h, int l, int u) const
{
    return h * lowerSqrt(u) + l;
}

VanEmdeBoas::Node* VanEmdeBoas::createNode(int u)
{
    Node* node = new Node();
    node->u = u;
    node->min = -1;
    node->max = -1;

    if (u > 2) {
        int numClusters = upperSqrt(u);
        int clusterSize = lowerSqrt(u);

        node->summary = nullptr;  /* 延迟创建 */
        node->cluster = new Node*[numClusters];
        for (int i = 0; i < numClusters; ++i)
            node->cluster[i] = nullptr;  /* 延迟创建 */
    } else {
        node->summary = nullptr;
        node->cluster = nullptr;
    }

    return node;
}

void VanEmdeBoas::destroyNode(Node* node)
{
    if (!node) return;
    if (node->u > 2 && node->cluster) {
        int numClusters = upperSqrt(node->u);
        for (int i = 0; i < numClusters; ++i)
            destroyNode(node->cluster[i]);
        delete[] node->cluster;
    }
    destroyNode(node->summary);
    delete node;
}

void VanEmdeBoas::insertRec(Node*& node, int x)
{
    if (node->min == -1) {
        /* 空节点 */
        node->min = x;
        node->max = x;
        return;
    }

    if (x < node->min) std::swap(x, node->min);
    if (x == node->min) return;  /* 已存在 */

    if (x > node->max) node->max = x;

    if (node->u > 2) {
        int hi = high(x, node->u);
        int lo = low(x, node->u);
        int numClusters = upperSqrt(node->u);

        /* 延迟创建簇 */
        if (!node->cluster[hi])
            node->cluster[hi] = createNode(lowerSqrt(node->u));

        if (node->cluster[hi]->min == -1) {
            /* 空簇,需要更新summary */
            if (!node->summary)
                node->summary = createNode(upperSqrt(node->u));
            insertRec(node->summary, hi);
        }

        insertRec(node->cluster[hi], lo);
    }
}

void VanEmdeBoas::removeRec(Node*& node, int x)
{
    if (!node || node->min == -1) return;

    if (node->min == node->max) {
        /* 只有一个元素 */
        if (x == node->min) {
            node->min = -1;
            node->max = -1;
        }
        return;
    }

    if (x == node->min) {
        /* 用summary的最小簇的第一个元素替换min */
        if (node->u == 2) {
            node->min = (x == 0) ? 1 : 0;
            node->max = node->min;
            return;
        }

        if (!node->summary || node->summary->min == -1) {
            /* 只有两个元素 */
            node->min = node->max;
            return;
        }

        int firstCluster = node->summary->min;
        x = index(firstCluster, node->cluster[firstCluster]->min, node->u);
        node->min = x;
    }

    if (node->u > 2) {
        int hi = high(x, node->u);
        int lo = low(x, node->u);

        if (node->cluster[hi]) {
            removeRec(node->cluster[hi], lo);

            if (node->cluster[hi]->min == -1) {
                /* 簇变为空,从summary中删除 */
                if (node->summary)
                    removeRec(node->summary, hi);

                /* 更新max */
                if (node->summary && node->summary->min != -1) {
                    int summaryMax = node->summary->max;
                    node->max = index(summaryMax,
                                      node->cluster[summaryMax]->max, node->u);
                } else {
                    node->max = node->min;
                }
            } else {
                /* 更新max */
                if (x == node->max) {
                    node->max = index(hi, node->cluster[hi]->max, node->u);
                }
            }
        }
    } else {
        /* u == 2 */
        node->min = (x == 0) ? 1 : 0;
        node->max = node->min;
    }
}

bool VanEmdeBoas::containsRec(Node* node, int x) const
{
    if (!node) return false;
    if (x == node->min || x == node->max) return true;
    if (node->u <= 2) return false;

    int hi = high(x, node->u);
    int lo = low(x, node->u);

    if (!node->cluster || !node->cluster[hi]) return false;
    return containsRec(node->cluster[hi], lo);
}

int VanEmdeBoas::predecessorRec(Node* node, int x) const
{
    if (!node) return -1;

    if (node->u <= 2) {
        if (x == 1 && node->min == 0) return 0;
        return -1;
    }

    if (node->max != -1 && x > node->max) return node->max;
    if (node->min != -1 && x <= node->min) return -1;

    int hi = high(x, node->u);
    int lo = low(x, node->u);
    int clusterLow = lowerSqrt(node->u);

    /* 检查x所在簇中是否有前驱 */
    if (node->cluster && node->cluster[hi] &&
        node->cluster[hi]->min != -1 &&
        lo > node->cluster[hi]->min) {
        int offset = predecessorRec(node->cluster[hi], lo);
        return index(hi, offset, node->u);
    }

    /* 在summary中查找前一个非空簇 */
    int prevCluster = -1;
    if (node->summary)
        prevCluster = predecessorRec(node->summary, hi);

    if (prevCluster != -1 && node->cluster && node->cluster[prevCluster]) {
        return index(prevCluster, node->cluster[prevCluster]->max, node->u);
    }

    /* 检查min本身是否为前驱 */
    if (node->min != -1 && x > node->min) return node->min;

    return -1;
}

int VanEmdeBoas::successorRec(Node* node, int x) const
{
    if (!node) return -1;

    if (node->u <= 2) {
        if (x == 0 && node->max == 1) return 1;
        return -1;
    }

    if (node->min != -1 && x < node->min) return node->min;
    if (node->max != -1 && x >= node->max) return -1;

    int hi = high(x, node->u);
    int lo = low(x, node->u);

    /* 检查x所在簇中是否有后继 */
    if (node->cluster && node->cluster[hi] &&
        node->cluster[hi]->max != -1 &&
        lo < node->cluster[hi]->max) {
        int offset = successorRec(node->cluster[hi], lo);
        return index(hi, offset, node->u);
    }

    /* 在summary中查找下一个非空簇 */
    int nextCluster = -1;
    if (node->summary)
        nextCluster = successorRec(node->summary, hi);

    if (nextCluster != -1 && node->cluster && node->cluster[nextCluster]) {
        return index(nextCluster, node->cluster[nextCluster]->min, node->u);
    }

    return -1;
}

int VanEmdeBoas::minimumRec(Node* node) const
{
    return node ? node->min : -1;
}

int VanEmdeBoas::maximumRec(Node* node) const
{
    return node ? node->max : -1;
}

void VanEmdeBoas::collectSorted(Node* node, int offset,
                                QVector<int>& result) const
{
    if (!node || node->min == -1) return;

    /* min不存储在簇中,直接输出 */
    result.append(offset + node->min);

    if (node->min == node->max) return;  /* 只有一个元素 */

    if (node->u > 2 && node->cluster) {
        int numClusters = upperSqrt(node->u);
        int clusterSize = lowerSqrt(node->u);
        for (int i = 0; i < numClusters; ++i) {
            if (node->cluster[i]) {
                int clusterOffset = offset + i * clusterSize;
                collectSorted(node->cluster[i], clusterOffset, result);
            }
        }
    } else if (node->u == 2) {
        /* 对于u==2,如果min!=max,则还有另一个元素 */
        if (node->max != node->min && node->max != -1)
            result.append(offset + node->max);
    }
}

VanEmdeBoas::Stats VanEmdeBoas::stats() const { return m_stats; }

void VanEmdeBoas::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
