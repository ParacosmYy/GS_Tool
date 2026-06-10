/**
 * @file BirchClustering14.cpp
 * @brief BirchClustering14 实现
 *
 * 实现BIRCH聚类：自适应阈值与CF树重平衡的内存受限流式聚类维护。
 */

#include "utils/cluster274/BirchClustering14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- CFEntry helpers ---- */

double BirchClustering14::CFEntry::radius() const
{
    if (n == 0 || dim == 0) return 0.0;
    double sumSq = 0.0;
    for (int i = 0; i < dim; ++i) {
        double c = centroid(i);
        double ss = squareSum[i] / n;
        sumSq += ss - c * c;
    }
    return qSqrt(qMax(0.0, sumSq));
}

double BirchClustering14::CFEntry::centroid(int d) const
{
    return (n > 0 && d < linearSum.size()) ? linearSum[d] / n : 0.0;
}

/* ---- Construction / Destruction ---- */

BirchClustering14::BirchClustering14(QObject *parent)
    : QObject(parent)
{
    m_nodes.append(CFNode{});  // Root node
}

BirchClustering14::~BirchClustering14() = default;

/* ---- Configuration ---- */

void BirchClustering14::setDimensions(int d) { m_dim = qBound(1, d, 1000); }
void BirchClustering14::setThreshold(double t) { m_threshold = qBound(0.01, t, 1e6); m_adaptiveThreshold = m_threshold; }
void BirchClustering14::setBranchingFactor(int b) { m_branchFactor = qBound(2, b, 500); }

/* ---- Distance computation ---- */

double BirchClustering14::distToCF(const QVector<double>& point, const CFEntry& cf) const
{
    double dSq = 0.0;
    for (int i = 0; i < qMin(point.size(), cf.linearSum.size()); ++i) {
        double diff = point[i] - cf.centroid(i);
        dSq += diff * diff;
    }
    return qSqrt(dSq);
}

/* ---- Merge two CF entries ---- */

BirchClustering14::CFEntry BirchClustering14::mergeCF(const CFEntry& a, const CFEntry& b) const
{
    CFEntry result;
    result.n = a.n + b.n;
    result.dim = a.dim;
    result.linearSum.resize(result.dim);
    result.squareSum.resize(result.dim);
    for (int i = 0; i < result.dim; ++i) {
        result.linearSum[i] = a.linearSum[i] + b.linearSum[i];
        result.squareSum[i] = a.squareSum[i] + b.squareSum[i];
    }
    return result;
}

/* ---- Add point to CF entry ---- */

void BirchClustering14::addToCF(CFEntry& cf, const QVector<double>& point) const
{
    cf.n++;
    for (int i = 0; i < qMin(point.size(), cf.dim); ++i) {
        cf.linearSum[i] += point[i];
        cf.squareSum[i] += point[i] * point[i];
    }
}

/* ---- Find closest leaf entry ---- */

int BirchClustering14::findClosestLeaf(const QVector<double>& point, int nodeIdx) const
{
    const CFNode& node = m_nodes[nodeIdx];
    if (node.isLeaf) {
        double minDist = 1e18;
        int best = 0;
        for (int i = 0; i < node.entries.size(); ++i) {
            double d = distToCF(point, node.entries[i]);
            if (d < minDist) { minDist = d; best = i; }
        }
        return best;
    }
    // Non-leaf: find closest child centroid
    double minDist = 1e18;
    int bestChild = 0;
    for (int i = 0; i < node.entries.size(); ++i) {
        double d = distToCF(point, node.entries[i]);
        if (d < minDist) { minDist = d; bestChild = i; }
    }
    if (bestChild < node.children.size())
        return findClosestLeaf(point, node.children[bestChild]);
    return 0;
}

/* ---- Insert into leaf ---- */

bool BirchClustering14::insertIntoLeaf(const QVector<double>& point, int leafIdx)
{
    CFNode& leaf = m_nodes[leafIdx];
    int closest = -1;
    double minDist = 1e18;
    for (int i = 0; i < leaf.entries.size(); ++i) {
        double d = distToCF(point, leaf.entries[i]);
        if (d < minDist) { minDist = d; closest = i; }
    }

    if (closest >= 0 && minDist <= m_adaptiveThreshold) {
        // Absorb into existing entry
        addToCF(leaf.entries[closest], point);
        return false;
    }

    // Create new entry
    CFEntry entry;
    entry.n = 1;
    entry.dim = m_dim;
    entry.linearSum = point;
    entry.squareSum.resize(m_dim);
    for (int i = 0; i < m_dim; ++i) entry.squareSum[i] = point[i] * point[i];
    leaf.entries.append(entry);

    return leaf.entries.size() > m_branchFactor;
}

/* ---- Split leaf node ---- */

int BirchClustering14::splitLeaf(int leafIdx)
{
    CFNode& old = m_nodes[leafIdx];
    int half = old.entries.size() / 2;

    CFNode newNode;
    newNode.isLeaf = true;
    newNode.parent = old.parent;

    for (int i = half; i < old.entries.size(); ++i)
        newNode.entries.append(old.entries[i]);
    old.entries.resize(half);

    int newIdx = m_nodes.size();
    m_nodes.append(newNode);
    m_stats.numSplits++;
    return newIdx;
}

/* ---- Propagate split upward ---- */

void BirchClustering14::propagateSplit(int nodeIdx, const CFEntry& newEntry, int newNodeIdx)
{
    CFNode& node = m_nodes[nodeIdx];
    node.entries.append(newEntry);
    node.children.append(newNodeIdx);

    if (nodeIdx == m_root && node.entries.size() > m_branchFactor) {
        // Split root: create new root
        int half = node.entries.size() / 2;
        CFNode newRoot;
        newRoot.isLeaf = false;
        newRoot.parent = -1;

        CFNode rightChild;
        rightChild.isLeaf = node.isLeaf;
        rightChild.parent = 0;

        for (int i = half; i < node.entries.size(); ++i) {
            rightChild.entries.append(node.entries[i]);
            rightChild.children.append(node.children[i]);
        }

        CFEntry leftSummary;
        leftSummary.dim = m_dim;
        leftSummary.n = 0;
        leftSummary.linearSum.resize(m_dim, 0.0);
        leftSummary.squareSum.resize(m_dim, 0.0);
        for (int i = 0; i < half; ++i) leftSummary = mergeCF(leftSummary, node.entries[i]);

        CFEntry rightSummary;
        rightSummary.dim = m_dim;
        rightSummary.n = 0;
        rightSummary.linearSum.resize(m_dim, 0.0);
        rightSummary.squareSum.resize(m_dim, 0.0);
        for (int i = 0; i < rightChild.entries.size(); ++i)
            rightSummary = mergeCF(rightSummary, rightChild.entries[i]);

        node.entries.resize(half);
        node.children.resize(half);

        int rightIdx = m_nodes.size();
        m_nodes.append(rightChild);

        newRoot.entries = {leftSummary, rightSummary};
        newRoot.children = {m_root, rightIdx};
        m_nodes.append(newRoot);
        m_root = m_nodes.size() - 1;
        return;
    }

    if (node.entries.size() > m_branchFactor && node.parent >= 0) {
        CFEntry summary;
        summary.dim = m_dim;
        summary.n = 0;
        summary.linearSum.resize(m_dim, 0.0);
        summary.squareSum.resize(m_dim, 0.0);
        for (const auto& e : node.entries) summary = mergeCF(summary, e);
        int splitIdx = splitLeaf(nodeIdx);  // Reuse split logic
        propagateSplit(node.parent, summary, splitIdx);
    }
}

/* ---- Adapt threshold based on tree density ---- */

void BirchClustering14::adaptThreshold()
{
    // Count total leaf entries
    int totalEntries = 0;
    for (const auto& n : m_nodes) {
        if (n.isLeaf) totalEntries += n.entries.size();
    }
    // Adaptive: increase threshold if tree is too dense, decrease if sparse
    double density = static_cast<double>(totalEntries) /
                     qMax(1, m_nodes.size() * m_branchFactor);
    if (density > 0.8)
        m_adaptiveThreshold *= 1.05;
    else if (density < 0.2)
        m_adaptiveThreshold *= 0.95;
    m_adaptiveThreshold = qBound(m_threshold * 0.1, m_adaptiveThreshold, m_threshold * 10.0);
}

/* ---- Tree height ---- */

int BirchClustering14::treeHeight() const
{
    int h = 0;
    int cur = m_root;
    while (cur >= 0 && !m_nodes[cur].isLeaf) {
        h++;
        cur = m_nodes[cur].children.isEmpty() ? -1 : m_nodes[cur].children[0];
    }
    return h + 1;
}

/* ---- Insert single point ---- */

void BirchClustering14::insertPoint(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();

    int leafIdx = m_root;
    while (!m_nodes[leafIdx].isLeaf) {
        int closest = findClosestLeaf(point, leafIdx);
        leafIdx = (closest < m_nodes[leafIdx].children.size())
                      ? m_nodes[leafIdx].children[closest] : leafIdx;
    }

    bool split = insertIntoLeaf(point, leafIdx);
    if (split) {
        int newIdx = splitLeaf(leafIdx);
        CFEntry newSummary;
        newSummary.dim = m_dim;
        newSummary.n = 0;
        newSummary.linearSum.resize(m_dim, 0.0);
        newSummary.squareSum.resize(m_dim, 0.0);
        for (const auto& e : m_nodes[newIdx].entries)
            newSummary = mergeCF(newSummary, e);
        if (m_nodes[leafIdx].parent >= 0)
            propagateSplit(m_nodes[leafIdx].parent, newSummary, newIdx);
    }

    m_stats.numPoints++;
    adaptThreshold();

    double elapsed = timer.elapsed();
    m_stats.numSubClusters = 0;
    for (const auto& n : m_nodes) if (n.isLeaf) m_stats.numSubClusters += n.entries.size();
    m_stats.numTreeNodes = m_nodes.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Batch insert ---- */

void BirchClustering14::insertBatch(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    for (const auto& pt : data) insertPoint(pt);
    double elapsed = timer.elapsed();
    emit clusteringDone(0, m_stats.numSubClusters, m_stats.numSplits, elapsed);
}

/* ---- Final clustering step (simple k-means on CF centroids) ---- */

QVector<int> BirchClustering14::fit(int k)
{
    QElapsedTimer timer;
    timer.start();

    auto centroids = subClusterCentroids();
    int n = centroids.size();
    if (n == 0) return {};

    k = qBound(1, k, n);
    QVector<int> labels(n, 0);

    // Initialize centroids from CF entries
    QVector<QVector<double>> means(k);
    for (int c = 0; c < k; ++c)
        means[c] = centroids[c * n / k];

    // Run k-means iterations
    for (int iter = 0; iter < 20; ++iter) {
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double minDist = 1e18;
            int best = 0;
            for (int c = 0; c < k; ++c) {
                double dSq = 0.0;
                for (int d = 0; d < m_dim; ++d) {
                    double diff = centroids[i][d] - means[c][d];
                    dSq += diff * diff;
                }
                if (dSq < minDist) { minDist = dSq; best = c; }
            }
            if (labels[i] != best) { labels[i] = best; changed = true; }
        }
        if (!changed) break;

        // Update means
        QVector<int> counts(k, 0);
        for (auto& m : means) m.fill(0.0);
        for (int i = 0; i < n; ++i) {
            counts[labels[i]]++;
            for (int d = 0; d < m_dim; ++d)
                means[labels[i]][d] += centroids[i][d];
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0)
                for (int d = 0; d < m_dim; ++d) means[c][d] /= counts[c];
        }
    }

    double elapsed = timer.elapsed();
    emit clusteringDone(k, m_stats.numSubClusters, m_stats.numSplits, elapsed);
    return labels;
}

/* ---- Get sub-cluster centroids ---- */

QVector<QVector<double>> BirchClustering14::subClusterCentroids() const
{
    QVector<QVector<double>> result;
    for (const auto& node : m_nodes) {
        if (!node.isLeaf) continue;
        for (const auto& entry : node.entries) {
            QVector<double> c(m_dim);
            for (int d = 0; d < m_dim; ++d) c[d] = entry.centroid(d);
            result.append(c);
        }
    }
    return result;
}

/* ---- Get CF entries ---- */

QVector<BirchClustering14::CFEntry> BirchClustering14::cfEntries() const
{
    QVector<CFEntry> result;
    for (const auto& node : m_nodes)
        if (node.isLeaf) result.append(node.entries);
    return result;
}

/* ---- Rebalance tree ---- */

void BirchClustering14::rebalanceTree()
{
    int oldH = treeHeight();

    // Collect all leaf CF entries
    QVector<CFEntry> allEntries;
    for (const auto& node : m_nodes)
        if (node.isLeaf) allEntries.append(node.entries);

    // Rebuild tree with balanced median splits
    m_nodes.clear();
    m_nodes.append(CFNode{});
    m_root = 0;

    // Re-insert entries in batches
    for (const auto& entry : allEntries) {
        QVector<double> pt(entry.dim);
        for (int d = 0; d < entry.dim; ++d) pt[d] = entry.centroid(d);
        // Weight re-insert by n
        for (int i = 0; i < entry.n; ++i)
            insertPoint(pt);
    }

    int newH = treeHeight();
    m_stats.numRebalances++;
    emit treeRebalanced(oldH, newH);
}

/* ---- Reset ---- */

void BirchClustering14::resetStatistics()
{
    m_nodes.clear();
    m_nodes.append(CFNode{});
    m_root = 0;
    m_adaptiveThreshold = m_threshold;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
