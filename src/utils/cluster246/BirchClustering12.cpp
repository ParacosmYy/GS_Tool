/**
 * @file BirchClustering12.cpp
 * @brief BirchClustering12 实现
 *
 * 实现BIRCH聚类：增量CF条目合并与基于直径的子树分裂。
 */

#include "utils/cluster246/BirchClustering12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

BirchClustering12::BirchClustering12(QObject *parent) : QObject(parent) {}
BirchClustering12::~BirchClustering12() = default;

/* ---- Configuration ---- */

void BirchClustering12::setBranchingFactor(int B) { m_branchingFactor = qMax(2, B); }
void BirchClustering12::setDiameterThreshold(double T) { m_diameterThreshold = qMax(1e-10, T); }

/* ---- Compute centroid of a CF entry ---- */

QVector<double> BirchClustering12::cfCentroid(const CFEntry& cf) const
{
    if (cf.n == 0) return {};
    QVector<double> c(cf.linearSum.size());
    for (int i = 0; i < c.size(); ++i)
        c[i] = cf.linearSum[i] / cf.n;
    return c;
}

/* ---- Compute diameter of a point set ---- */

double BirchClustering12::computeDiameter(const QVector<QVector<double>>& pts) const
{
    if (pts.size() < 2) return 0.0;
    double maxDist = 0.0;
    for (int i = 0; i < pts.size(); ++i) {
        for (int j = i + 1; j < pts.size(); ++j) {
            double d = 0.0;
            int dim = qMin(pts[i].size(), pts[j].size());
            for (int k = 0; k < dim; ++k) {
                double diff = pts[i][k] - pts[j][k];
                d += diff * diff;
            }
            maxDist = qMax(maxDist, d);
        }
    }
    return qSqrt(maxDist);
}

/* ---- Merge two CF entries ---- */

void BirchClustering12::mergeCF(CFEntry& dst, const CFEntry& src) const
{
    dst.n += src.n;
    if (dst.linearSum.size() < src.linearSum.size())
        dst.linearSum.resize(src.linearSum.size(), 0.0);
    if (dst.squareSum.size() < src.squareSum.size())
        dst.squareSum.resize(src.squareSum.size(), 0.0);
    for (int i = 0; i < src.linearSum.size(); ++i)
        dst.linearSum[i] += src.linearSum[i];
    for (int i = 0; i < src.squareSum.size(); ++i)
        dst.squareSum[i] += src.squareSum[i];
}

/* ---- Create a new node ---- */

int BirchClustering12::createNode(bool isLeaf)
{
    int idx = m_nodes.size();
    m_nodes.append({isLeaf, {}, {}, {}, -1});
    return idx;
}

/* ---- Find closest leaf entry for a point ---- */

int BirchClustering12::findClosestLeaf(int nodeIdx, const QVector<double>& point) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return -1;
    const auto& node = m_nodes[nodeIdx];

    if (node.isLeaf) return nodeIdx;

    // Find child with closest centroid
    double minDist = std::numeric_limits<double>::max();
    int bestChild = -1;
    for (int ci : node.children) {
        if (ci < 0 || ci >= m_nodes.size()) continue;
        auto cen = cfCentroid(m_nodes[ci].cf);
        double d = 0.0;
        int dim = qMin(cen.size(), point.size());
        for (int k = 0; k < dim; ++k) {
            double diff = cen[k] - point[k];
            d += diff * diff;
        }
        if (d < minDist) { minDist = d; bestChild = ci; }
    }
    return findClosestLeaf(bestChild, point);
}

/* ---- Split a leaf node ---- */

int BirchClustering12::splitLeaf(int nodeIdx)
{
    auto& node = m_nodes[nodeIdx];
    int total = node.leafPoints.size();
    if (total < 2) return nodeIdx;

    // Find the two most distant points as seeds
    double maxDist = -1.0;
    int seedA = 0, seedB = 1;
    for (int i = 0; i < total; ++i) {
        for (int j = i + 1; j < total; ++j) {
            double d = 0.0;
            int dim = qMin(node.leafPoints[i].size(), node.leafPoints[j].size());
            for (int k = 0; k < dim; ++k) {
                double diff = node.leafPoints[i][k] - node.leafPoints[j][k];
                d += diff * diff;
            }
            if (d > maxDist) { maxDist = d; seedA = i; seedB = j; }
        }
    }

    // Distribute points to two new leaves
    int newLeaf1 = createNode(true);
    int newLeaf2 = createNode(true);
    auto& nl1 = m_nodes[newLeaf1];
    auto& nl2 = m_nodes[newLeaf2];

    for (int i = 0; i < total; ++i) {
        double dA = 0.0, dB = 0.0;
        int dim = qMin(node.leafPoints[i].size(), node.leafPoints[seedA].size());
        for (int k = 0; k < dim; ++k) {
            double diff = node.leafPoints[i][k] - node.leafPoints[seedA][k];
            dA += diff * diff;
        }
        dim = qMin(node.leafPoints[i].size(), node.leafPoints[seedB].size());
        for (int k = 0; k < dim; ++k) {
            double diff = node.leafPoints[i][k] - node.leafPoints[seedB][k];
            dB += diff * diff;
        }
        if (dA <= dB) nl1.leafPoints.append(node.leafPoints[i]);
        else nl2.leafPoints.append(node.leafPoints[i]);
    }

    // Update CF entries for new leaves
    for (const auto& pt : nl1.leafPoints) {
        nl1.cf.n++;
        if (nl1.cf.linearSum.size() < pt.size()) {
            nl1.cf.linearSum.resize(pt.size(), 0.0);
            nl1.cf.squareSum.resize(pt.size(), 0.0);
        }
        for (int k = 0; k < pt.size(); ++k) {
            nl1.cf.linearSum[k] += pt[k];
            nl1.cf.squareSum[k] += pt[k] * pt[k];
        }
    }
    for (const auto& pt : nl2.leafPoints) {
        nl2.cf.n++;
        if (nl2.cf.linearSum.size() < pt.size()) {
            nl2.cf.linearSum.resize(pt.size(), 0.0);
            nl2.cf.squareSum.resize(pt.size(), 0.0);
        }
        for (int k = 0; k < pt.size(); ++k) {
            nl2.cf.linearSum[k] += pt[k];
            nl2.cf.squareSum[k] += pt[k] * pt[k];
        }
    }

    // Link new leaves
    nl1.next = newLeaf2;
    nl2.next = node.next;

    m_stats.numSplits++;
    node.children = {newLeaf1, newLeaf2};
    node.isLeaf = false;
    node.leafPoints.clear();
    mergeCF(node.cf, nl1.cf);
    mergeCF(node.cf, nl2.cf);
    node.cf.n = nl1.cf.n + nl2.cf.n;

    return nodeIdx;
}

/* ---- Update CF path upward ---- */

void BirchClustering12::updateCFPath(int nodeIdx)
{
    int cur = nodeIdx;
    while (cur >= 0) {
        auto& node = m_nodes[cur];
        // Recompute CF from children or leaf points
        if (!node.isLeaf && !node.children.isEmpty()) {
            CFEntry combined;
            for (int ci : node.children) {
                if (ci >= 0 && ci < m_nodes.size())
                    mergeCF(combined, m_nodes[ci].cf);
            }
            node.cf = combined;
        }
        cur = node.parent;
    }
}

/* ---- Insert a point into the CF tree ---- */

void BirchClustering12::insertPoint(const QVector<double>& point)
{
    // Create root if empty
    if (m_root < 0) {
        m_root = createNode(true);
    }

    // Find closest leaf
    int leafIdx = findClosestLeaf(m_root, point);
    if (leafIdx < 0) return;

    auto& leaf = m_nodes[leafIdx];

    // Check diameter threshold
    QVector<QVector<double>> testPoints = leaf.leafPoints;
    testPoints.append(point);
    double diam = computeDiameter(testPoints);

    if (diam <= m_diameterThreshold || leaf.leafPoints.isEmpty()) {
        // Absorb into this leaf
        leaf.leafPoints.append(point);
        leaf.cf.n++;
        if (leaf.cf.linearSum.size() < m_dim) {
            leaf.cf.linearSum.resize(m_dim, 0.0);
            leaf.cf.squareSum.resize(m_dim, 0.0);
        }
        for (int k = 0; k < m_dim; ++k) {
            leaf.cf.linearSum[k] += point[k];
            leaf.cf.squareSum[k] += point[k] * point[k];
        }
    } else {
        // Need to split
        leaf.leafPoints.append(point);
        // Update CF
        leaf.cf.n++;
        if (leaf.cf.linearSum.size() < m_dim) {
            leaf.cf.linearSum.resize(m_dim, 0.0);
            leaf.cf.squareSum.resize(m_dim, 0.0);
        }
        for (int k = 0; k < m_dim; ++k) {
            leaf.cf.linearSum[k] += point[k];
            leaf.cf.squareSum[k] += point[k] * point[k];
        }
        splitLeaf(leafIdx);
    }
    updateCFPath(leafIdx);
}

/* ---- Assign labels by leaf clusters ---- */

void BirchClustering12::assignLabels(int n)
{
    m_labels.resize(n, -1);
    int label = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (!m_nodes[i].isLeaf) continue;
        for (const auto& pt : m_nodes[i].leafPoints) {
            // Match point to original index by comparing all dims
            for (int j = 0; j < n; ++j) {
                if (m_labels[j] >= 0) continue;
                bool match = true;
                for (int k = 0; k < m_dim; ++k) {
                    if (qAbs(pt[k] - /* stored original data -- simplified */ 0) > 1e-12) {
                        match = false;
                        break;
                    }
                }
                if (match) { m_labels[j] = label; break; }
            }
        }
        if (!m_nodes[i].leafPoints.isEmpty()) label++;
    }
    // Fallback: assign any unlabeled to cluster 0
    for (int i = 0; i < n; ++i)
        if (m_labels[i] < 0) m_labels[i] = 0;
}

/* ---- Main fit ---- */

QVector<int> BirchClustering12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};
    m_dim = data[0].size();

    // Reset tree
    m_nodes.clear();
    m_root = -1;
    m_labels.clear();

    // Insert all points incrementally
    for (int i = 0; i < n; ++i)
        insertPoint(data[i]);

    // Simple label assignment: traverse leaves
    m_labels.resize(n, 0);
    int label = 0;
    int pointIdx = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (!m_nodes[i].isLeaf) continue;
        for (int j = 0; j < m_nodes[i].leafPoints.size() && pointIdx < n; ++j) {
            m_labels[pointIdx++] = label;
        }
        if (!m_nodes[i].leafPoints.isEmpty()) label++;
    }

    m_stats.numSamples = n;
    m_stats.numDimensions = m_dim;
    int leafCount = 0;
    for (const auto& nd : m_nodes)
        if (nd.isLeaf) leafCount++;
    m_stats.numLeafEntries = leafCount;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(label, m_stats.numSplits, timer.elapsed());
    return m_labels;
}

/* ---- Get leaf CF entries ---- */

QVector<BirchClustering12::CFEntry> BirchClustering12::leafEntries() const
{
    QVector<CFEntry> result;
    for (const auto& nd : m_nodes)
        if (nd.isLeaf) result.append(nd.cf);
    return result;
}

/* ---- Reset ---- */

void BirchClustering12::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_labels.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
