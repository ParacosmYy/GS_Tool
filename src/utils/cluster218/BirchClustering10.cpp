/**
 * @file BirchClustering10.cpp
 * @brief BirchClustering10 实现
 *
 * 实现BIRCH聚类：自适应阈值CF树、离群点缓冲区、周期重吸收。
 */

#include "utils/cluster218/BirchClustering10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- CFNode destructor ---- */

BirchClustering10::CFNode::~CFNode()
{
    for (auto* c : children) delete c;
}

/* ---- Construction / Destruction ---- */

BirchClustering10::BirchClustering10(QObject *parent) : QObject(parent)
{
    m_root = new CFNode{true, {}, {}};
}

BirchClustering10::~BirchClustering10() { clearNode(m_root); }

/* ---- Configuration ---- */

void BirchClustering10::setParameters(double threshold, int branching, int outlierCap)
{
    m_threshold = qMax(0.01, threshold);
    m_branching = qMax(2, branching);
    m_outlierCap = qMax(10, outlierCap);
}

/* ---- CF helpers ---- */

double BirchClustering10::cfRadius(const CFEntry& cf) const
{
    if (cf.n == 0) return 0.0;
    // Radius = sqrt(ss/n - ||ls/n||^2)
    double r = cf.ss / cf.n;
    for (int i = 0; i < cf.ls.size(); ++i) {
        double c = cf.ls[i] / cf.n;
        r -= c * c;
    }
    return qSqrt(qMax(0.0, r));
}

void BirchClustering10::cfMerge(CFEntry& dst, const CFEntry& src) const
{
    if (dst.n == 0) {
        dst.ls = src.ls;
    } else {
        for (int i = 0; i < dst.ls.size(); ++i)
            dst.ls[i] += src.ls[i];
    }
    dst.n += src.n;
    dst.ss += src.ss;
}

BirchClustering10::CFEntry BirchClustering10::pointToCF(const QVector<double>& point) const
{
    CFEntry cf;
    cf.n = 1;
    cf.ls = point;
    cf.ss = 0.0;
    for (auto v : point) cf.ss += v * v;
    return cf;
}

double BirchClustering10::euclidean(const QVector<double>& a,
                                     const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return qSqrt(d);
}

int BirchClustering10::closestChild(const QVector<CFNode*>& children,
                                      const QVector<double>& point) const
{
    double bestDist = std::numeric_limits<double>::max();
    int bestIdx = 0;
    for (int i = 0; i < children.size(); ++i) {
        if (children[i]->cf.n == 0) continue;
        QVector<double> centroid(cf.ls.size());
        for (int d = 0; d < centroid.size(); ++d)
            centroid[d] = children[i]->cf.ls[d] / children[i]->cf.n;
        double dist = euclidean(centroid, point);
        if (dist < bestDist) { bestDist = dist; bestIdx = i; }
    }
    return bestIdx;
}

/* ---- CF-tree insertion ---- */

bool BirchClustering10::insertIntoTree(const QVector<double>& point)
{
    if (m_dim == 0) m_dim = point.size();
    if (point.size() != m_dim) return false;

    // Navigate to leaf
    CFNode* current = m_root;
    while (!current->isLeaf) {
        int idx = closestChild(current->children, point);
        current = current->children[idx];
    }

    // Try to absorb into closest leaf entry (child of leaf parent)
    CFNode* parent = current->parent ? current->parent : m_root;
    if (parent->children.isEmpty()) {
        // First point: create leaf child
        CFNode* leaf = new CFNode{true, pointToCF(point), {}};
        leaf->parent = parent;
        parent->children.append(leaf);
        parent->isLeaf = false;
        updateCFPath(leaf);
        return true;
    }

    // Find closest leaf child
    int bestIdx = closestChild(parent->children, point);
    CFNode* bestLeaf = parent->children[bestIdx];
    CFEntry testCF = bestLeaf->cf;
    cfMerge(testCF, pointToCF(point));

    if (cfRadius(testCF) <= m_threshold) {
        // Absorb into existing leaf
        bestLeaf->cf = testCF;
        updateCFPath(bestLeaf);
        return true;
    }

    // Create new leaf entry
    if (parent->children.size() < m_branching) {
        CFNode* newLeaf = new CFNode{true, pointToCF(point), {}};
        newLeaf->parent = parent;
        parent->children.append(newLeaf);
        updateCFPath(newLeaf);
        return true;
    }

    // Overflow: split leaf
    CFNode* newLeaf = new CFNode{true, pointToCF(point), {}};
    newLeaf->parent = parent;
    parent->children.append(newLeaf);
    splitLeaf(parent);
    return true;
}

void BirchClustering10::splitLeaf(CFNode* leafParent)
{
    if (leafParent->children.size() <= m_branching) return;

    // Find two farthest apart leaf entries
    double maxDist = -1.0;
    int idx1 = 0, idx2 = 1;
    for (int i = 0; i < leafParent->children.size(); ++i) {
        for (int j = i + 1; j < leafParent->children.size(); ++j) {
            auto& a = leafParent->children[i]->cf;
            auto& b = leafParent->children[j]->cf;
            if (a.n == 0 || b.n == 0) continue;
            QVector<double> ca(m_dim), cb(m_dim);
            for (int d = 0; d < m_dim; ++d) {
                ca[d] = a.ls[d] / a.n;
                cb[d] = b.ls[d] / b.n;
            }
            double d = euclidean(ca, cb);
            if (d > maxDist) { maxDist = d; idx1 = i; idx2 = j; }
        }
    }

    // Redistribute children into two groups
    QVector<CFNode*> group1, group2;
    group1.append(leafParent->children[idx1]);
    group2.append(leafParent->children[idx2]);

    for (int i = 0; i < leafParent->children.size(); ++i) {
        if (i == idx1 || i == idx2) continue;
        // Assign to closer group
        auto& child = leafParent->children[i];
        if (child->cf.n == 0) { group1.append(child); continue; }
        auto& g1 = group1[0]->cf;
        auto& g2 = group2[0]->cf;
        QVector<double> c1(m_dim), c2(m_dim), cc(m_dim);
        for (int d = 0; d < m_dim; ++d) {
            c1[d] = g1.ls[d] / g1.n;
            c2[d] = g2.ls[d] / g2.n;
            cc[d] = child->cf.ls[d] / child->cf.n;
        }
        if (euclidean(cc, c1) < euclidean(cc, c2))
            group1.append(child);
        else
            group2.append(child);
    }

    // Keep group1 in current parent, group2 goes to new node
    leafParent->children = group1;
    leafParent->cf = {};
    for (auto* c : group1) cfMerge(leafParent->cf, c->cf);

    CFNode* newParent = new CFNode{false, {}, {}};
    newParent->children = group2;
    for (auto* c : group2) { c->parent = newParent; cfMerge(newParent->cf, c->cf); }

    // Propagate split upward
    if (leafParent == m_root) {
        CFNode* newRoot = new CFNode{false, {}, {}};
        leafParent->parent = newRoot;
        newParent->parent = newRoot;
        newRoot->children.append(leafParent);
        newRoot->children.append(newParent);
        cfMerge(newRoot->cf, leafParent->cf);
        cfMerge(newRoot->cf, newParent->cf);
        m_root = newRoot;
    } else {
        newParent->parent = leafParent->parent;
        leafParent->parent->children.append(newParent);
        updateCFPath(newParent);
        if (leafParent->parent->children.size() > m_branching)
            splitLeaf(leafParent->parent); // Recursive split
    }
}

void BirchClustering10::updateCFPath(CFNode* node)
{
    while (node) {
        node->cf = {};
        if (!node->children.isEmpty()) {
            for (auto* c : node->children) cfMerge(node->cf, c->cf);
        }
        node = node->parent;
    }
}

void BirchClustering10::adaptThreshold()
{
    if (m_root->children.isEmpty()) return;
    int leafCount = 0;
    collectLeaves(m_root, *(new QVector<CFEntry>()));
    // Count leaf-level entries
    QVector<CFEntry> entries;
    collectLeaves(m_root, entries);
    leafCount = entries.size();

    // Adjust threshold: increase if too many leaves, decrease if too few
    if (leafCount > m_branching * 2)
        m_threshold *= 1.2;
    else if (leafCount < m_branching / 2 && m_threshold > 0.01)
        m_threshold *= 0.8;

    m_stats.threshold = m_threshold;
}

/* ---- Collect leaves ---- */

void BirchClustering10::collectLeaves(CFNode* node, QVector<CFEntry>& entries) const
{
    if (!node) return;
    if (node->isLeaf || node->children.isEmpty()) {
        if (node->cf.n > 0) entries.append(node->cf);
        return;
    }
    for (auto* c : node->children) collectLeaves(c, entries);
}

int BirchClustering10::computeDepth(CFNode* node) const
{
    if (!node || node->children.isEmpty()) return 0;
    int maxD = 0;
    for (auto* c : node->children)
        maxD = qMax(maxD, computeDepth(c));
    return 1 + maxD;
}

void BirchClustering10::clearNode(CFNode* node) { delete node; }

/* ---- Insert single point ---- */

void BirchClustering10::insertPoint(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();

    if (!insertIntoTree(point)) {
        // Send to outlier buffer
        m_outlierBuf.append(point);
        m_stats.numOutliers++;
    }

    m_stats.numPoints++;
    m_stats.treeDepth = computeDepth(m_root);
    adaptThreshold();

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Batch fit ---- */

void BirchClustering10::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < data.size(); ++i) {
        if (!insertIntoTree(data[i])) {
            m_outlierBuf.append(data[i]);
            m_stats.numOutliers++;
        }
        m_stats.numPoints++;
    }

    // Periodic reabsorption
    if (m_outlierBuf.size() > m_outlierCap / 2)
        reabsorbOutliers();

    QVector<CFEntry> entries;
    collectLeaves(m_root, entries);
    m_stats.numLeafEntries = entries.size();
    m_stats.treeDepth = computeDepth(m_root);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(entries.size(), m_stats.numOutliers, timer.elapsed());
}

/* ---- Centroids ---- */

QVector<QVector<double>> BirchClustering10::centroids() const
{
    QVector<CFEntry> entries;
    collectLeaves(m_root, entries);
    QVector<QVector<double>> result;
    for (auto& cf : entries) {
        QVector<double> c(m_dim, 0.0);
        for (int d = 0; d < m_dim; ++d)
            c[d] = cf.ls[d] / cf.n;
        result.append(c);
    }
    return result;
}

/* ---- Predict ---- */

QVector<int> BirchClustering10::predict(const QVector<QVector<double>>& data) const
{
    QVector<QVector<double>> cents = centroids();
    QVector<int> labels(data.size(), 0);
    for (int i = 0; i < data.size(); ++i) {
        double bestD = std::numeric_limits<double>::max();
        for (int c = 0; c < cents.size(); ++c) {
            double d = euclidean(data[i], cents[c]);
            if (d < bestD) { bestD = d; labels[i] = c; }
        }
    }
    return labels;
}

/* ---- Reabsorb outliers ---- */

void BirchClustering10::reabsorbOutliers()
{
    int reabsorbed = 0;
    QVector<QVector<double>> remaining;
    for (auto& pt : m_outlierBuf) {
        if (insertIntoTree(pt)) {
            reabsorbed++;
        } else {
            remaining.append(pt);
        }
    }
    m_outlierBuf = remaining;
    m_stats.numReabsorbed += reabsorbed;
    m_stats.numOutliers = m_outlierBuf.size();
}

/* ---- Reset ---- */

void BirchClustering10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clearNode(m_root);
    m_root = new CFNode{true, {}, {}};
    m_outlierBuf.clear();
    m_dim = 0;
}
