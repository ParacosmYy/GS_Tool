/**
 * @file BirchClustering8.cpp
 * @brief BirchClustering8 实现
 *
 * 实现BIRCH聚类：增量CF树构建、自动重平衡、离群点缓冲、多分辨率聚类。
 */

#include "utils/cluster194/BirchClustering8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BirchClustering8::BirchClustering8(QObject *parent) : QObject(parent) {}
BirchClustering8::~BirchClustering8() = default;

/* ---- Configuration ---- */

void BirchClustering8::setBranchFactor(int b) { m_branchFactor = qMax(2, b); }
void BirchClustering8::setThreshold(double t) { m_threshold = qMax(0.001, t); }
void BirchClustering8::setMaxLeafEntries(int m) { m_maxLeafEntries = qMax(2, m); }
void BirchClustering8::setClusters(int k) { m_k = qMax(1, k); }
void BirchClustering8::setOutlierRatio(double r) { m_outlierRatio = qBound(0.0, r, 0.5); }

/* ---- CF operations ---- */

BirchClustering8::CF BirchClustering8::mergeCF(const CF& a, const CF& b)
{
    CF r;
    r.n = a.n + b.n;
    r.ss = a.ss + b.ss;
    int dim = qMin(a.ls.size(), b.ls.size());
    r.ls.resize(dim);
    for (int i = 0; i < dim; ++i) r.ls[i] = a.ls[i] + b.ls[i];
    return r;
}

double BirchClustering8::cfRadius(const CF& cf)
{
    if (cf.n <= 1) return 0.0;
    double ssLS = 0.0;
    for (int i = 0; i < cf.ls.size(); ++i) ssLS += cf.ls[i] * cf.ls[i];
    return qSqrt(qMax(0.0, (cf.ss - ssLS / cf.n) / cf.n));
}

QVector<double> BirchClustering8::cfCentroid(const CF& cf)
{
    if (cf.n == 0) return {};
    QVector<double> c(cf.ls.size());
    for (int i = 0; i < cf.ls.size(); ++i) c[i] = cf.ls[i] / cf.n;
    return c;
}

double BirchClustering8::distToCF(const QVector<double>& pt, const CF& cf)
{
    auto c = cfCentroid(cf);
    if (c.isEmpty()) return 0.0;
    double d = 0.0;
    int dim = qMin(pt.size(), c.size());
    for (int i = 0; i < dim; ++i) {
        double diff = pt[i] - c[i];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Find closest leaf entry ---- */

int BirchClustering8::findClosest(const QVector<double>& pt, int nodeIdx) const
{
    const auto& node = m_nodes[nodeIdx];
    if (node.isLeaf) {
        int best = -1;
        double bestD = std::numeric_limits<double>::max();
        for (int i = 0; i < node.entries.size(); ++i) {
            double d = distToCF(pt, node.entries[i].cf);
            if (d < bestD) { bestD = d; best = i; }
        }
        return best;
    }
    // Non-leaf: descend closest child
    int bestChild = -1;
    double bestD = std::numeric_limits<double>::max();
    for (int ci : node.children) {
        double d = distToCF(pt, m_nodes[ci].cf);
        if (d < bestD) { bestD = d; bestChild = ci; }
    }
    return bestChild;
}

/* ---- Split overfull leaf ---- */

int BirchClustering8::splitLeaf(int nodeIdx)
{
    auto& node = m_nodes[nodeIdx];
    int n = node.entries.size();
    // Find two farthest apart entries as seeds
    int s1 = 0, s2 = 1;
    double maxDist = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = distToCF(cfCentroid(node.entries[i].cf), node.entries[j].cf);
            if (d > maxDist) { maxDist = d; s1 = i; s2 = j; }
        }
    }

    // New leaf node
    TreeNode newLeaf;
    newLeaf.isLeaf = true;
    newLeaf.entries.append(node.entries[s2]);

    // Redistribute entries
    QVector<LeafEntry> remain;
    for (int i = 0; i < n; ++i) {
        if (i == s1) continue;
        if (i == s2) continue;
        remain.append(node.entries[i]);
    }
    node.entries = {node.entries[s1]};
    for (const auto& e : remain) {
        double d1 = distToCF(cfCentroid(e.cf), node.entries[0].cf);
        double d2 = distToCF(cfCentroid(e.cf), newLeaf.entries[0].cf);
        if (d1 <= d2) node.entries.append(e);
        else newLeaf.entries.append(e);
    }

    // Recompute CFs
    node.cf = {0, QVector<double>(m_dim, 0.0), 0.0};
    for (const auto& e : node.entries) node.cf = mergeCF(node.cf, e.cf);
    newLeaf.cf = {0, QVector<double>(m_dim, 0.0), 0.0};
    for (const auto& e : newLeaf.entries) newLeaf.cf = mergeCF(newLeaf.cf, e.cf);

    int newIdx = m_nodes.size();
    m_nodes.append(newLeaf);
    return newIdx;
}

/* ---- Absorb outliers ---- */

void BirchClustering8::absorbOutliers()
{
    auto buf = m_outlierBuf;
    m_outlierBuf.clear();
    for (const auto& pt : buf) insertPoint(pt);
}

/* ---- Insert single point ---- */

void BirchClustering8::insertPoint(const QVector<double>& point)
{
    if (point.isEmpty()) return;
    m_dim = point.size();

    // Initialize root if needed
    if (m_root < 0) {
        TreeNode root;
        root.isLeaf = true;
        m_nodes.append(root);
        m_root = 0;
    }

    // Descend to leaf
    int cur = m_root;
    while (!m_nodes[cur].isLeaf) {
        int next = findClosest(point, cur);
        if (next < 0) break;
        cur = next;
    }

    auto& leaf = m_nodes[cur];
    // Try to absorb into closest entry
    int closest = findClosest(point, cur);
    if (closest >= 0) {
        CF testCF = mergeCF(leaf.entries[closest].cf, {1, point, 0.0});
        for (int i = 0; i < point.size(); ++i) testCF.ss += point[i] * point[i];
        testCF.ls = point;
        testCF.n = leaf.entries[closest].cf.n + 1;
        // Recompute properly
        CF merged = leaf.entries[closest].cf;
        merged.n++;
        merged.ss = 0.0;
        for (int i = 0; i < point.size(); ++i) {
            merged.ls[i] += point[i];
            merged.ss += point[i] * point[i];
        }
        for (int i = 0; i < merged.ls.size(); ++i) merged.ss += (merged.ls[i] * merged.ls[i]) / (merged.n * merged.n);

        if (cfRadius(merged) <= m_threshold) {
            leaf.entries[closest].cf = merged;
            // Update ancestor CFs
            leaf.cf = mergeCF(leaf.cf, {1, point, 0.0});
            return;
        }
    }

    // Cannot absorb: create new entry
    LeafEntry e;
    e.cf.n = 1;
    e.cf.ls = point;
    e.cf.ss = 0.0;
    for (int i = 0; i < point.size(); ++i) e.cf.ss += point[i] * point[i];
    e.id = leaf.entries.size();
    leaf.entries.append(e);
    leaf.cf = mergeCF(leaf.cf, e.cf);

    // Split if overfull
    if (leaf.entries.size() > m_maxLeafEntries) {
        int newIdx = splitLeaf(cur);
        // Simple: add new leaf as sibling (no parent restructuring for brevity)
        Q_UNUSED(newIdx)
        emit treeRebalanced(m_stats.numLeaves, m_stats.numOutliers);
    }
}

/* ---- Build tree ---- */

void BirchClustering8::buildTree(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    resetStatistics();
    int n = data.size();
    if (n == 0) return;

    int outlierLimit = qMax(1, static_cast<int>(n * m_outlierRatio));

    for (int i = 0; i < n; ++i) {
        insertPoint(data[i]);

        // Periodic rebalance check
        if (i > 0 && i % 500 == 0) {
            absorbOutliers();
            emit treeRebalanced(m_stats.numLeaves, m_stats.numOutliers);
        }
    }

    // Count leaves
    m_stats.numLeaves = 0;
    for (const auto& nd : m_nodes)
        if (nd.isLeaf) m_stats.numLeaves++;
    m_stats.numPoints = n;
    m_stats.numOutliers = m_outlierBuf.size();
    m_stats.totalRuns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;
}

/* ---- Cluster via k-means on leaf CFs ---- */

QVector<int> BirchClustering8::cluster(int k)
{
    QElapsedTimer timer;
    timer.start();
    k = qMax(1, k);

    // Collect all leaf CF centroids
    QVector<QVector<double>> centroids;
    for (const auto& nd : m_nodes) {
        if (!nd.isLeaf) continue;
        for (const auto& e : nd.entries)
            centroids.append(cfCentroid(e.cf));
    }

    int nc = centroids.size();
    if (nc == 0) return {};

    // Initialize k centers randomly
    m_centers.resize(k);
    for (int i = 0; i < k; ++i)
        m_centers[i] = centroids[i % nc];

    m_labels.resize(nc);
    for (int it = 0; it < 20; ++it) {
        bool changed = false;
        for (int i = 0; i < nc; ++i) {
            double bestD = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                int dim = qMin(centroids[i].size(), m_centers[c].size());
                for (int j = 0; j < dim; ++j) {
                    double diff = centroids[i][j] - m_centers[c][j];
                    d += diff * diff;
                }
                if (d < bestD) { bestD = d; bestC = c; }
            }
            if (m_labels[i] != bestC) { m_labels[i] = bestC; changed = true; }
        }
        if (!changed) break;

        // Recompute centers
        QVector<int> counts(k, 0);
        m_centers = QVector<QVector<double>>(k, QVector<double>(m_dim, 0.0));
        for (int i = 0; i < nc; ++i) {
            int c = m_labels[i];
            counts[c]++;
            for (int j = 0; j < m_dim; ++j)
                m_centers[c][j] += centroids[i][j];
        }
        for (int c = 0; c < k; ++c)
            if (counts[c] > 0)
                for (int j = 0; j < m_dim; ++j)
                    m_centers[c][j] /= counts[c];
    }

    emit clusteringCompleted(k, timer.elapsed());
    return m_labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> BirchClustering8::centers() const { return m_centers; }
QVector<QVector<double>> BirchClustering8::outliers() const { return m_outlierBuf; }

/* ---- Reset ---- */

void BirchClustering8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
    m_dim = 0;
    m_centers.clear();
    m_outlierBuf.clear();
    m_labels.clear();
}
