/**
 * @file BirchClustering15.cpp
 * @brief BirchClustering15 实现
 *
 * 实现BIRCH聚类：动态阈值调整与子簇合并支持增量式内存感知流数据聚类。
 */

#include "utils/cluster288/BirchClustering15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BirchClustering15::BirchClustering15(QObject *parent)
    : QObject(parent) {}

BirchClustering15::~BirchClustering15() = default;

/* ---- Configuration ---- */

void BirchClustering15::setThreshold(double T) { m_threshold = qBound(0.01, T, 100.0); }
void BirchClustering15::setBranchingFactor(int B) { m_branching = qBound(2, B, 500); }
void BirchClustering15::setMaxPoints(int maxPts) { m_maxPoints = qBound(100, maxPts, 10000000); }
void BirchClustering15::setNumClusters(int k) { m_k = qBound(1, k, 10000); }

/* ---- CF radius computation ---- */

double BirchClustering15::cfRadius(const CFNode& cf) const
{
    if (cf.n <= 1) return 0.0;
    double r = 0.0;
    for (int i = 0; i < cf.linearSum.size(); ++i) {
        double centroid = cf.linearSum[i] / cf.n;
        double var = cf.squareSum[i] / cf.n - centroid * centroid;
        r += qMax(var, 0.0);
    }
    return qSqrt(r);
}

/* ---- Distance from point to CF centroid ---- */

double BirchClustering15::distToCF(const QVector<double>& point, const CFNode& cf) const
{
    if (cf.n == 0) return 0.0;
    double d = 0.0;
    for (int i = 0; i < point.size(); ++i) {
        double diff = point[i] - cf.linearSum[i] / cf.n;
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Merge point into CF ---- */

void BirchClustering15::mergeIntoCF(CFNode& cf, const QVector<double>& point)
{
    cf.n++;
    for (int i = 0; i < point.size(); ++i) {
        cf.linearSum[i] += point[i];
        cf.squareSum[i] += point[i] * point[i];
    }
}

/* ---- Merge two CFs ---- */

void BirchClustering15::mergeCFs(CFNode& dst, const CFNode& src)
{
    dst.n += src.n;
    for (int i = 0; i < dst.linearSum.size(); ++i) {
        dst.linearSum[i] += src.linearSum[i];
        dst.squareSum[i] += src.squareSum[i];
    }
}

/* ---- Find closest leaf subcluster ---- */

int BirchClustering15::findClosestLeaf(const QVector<double>& point) const
{
    int best = -1;
    double bestDist = 1e300;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (!m_nodes[i].isLeaf || m_nodes[i].n == 0) continue;
        double d = distToCF(point, m_nodes[i]);
        if (d < bestDist) {
            bestDist = d;
            best = i;
        }
    }
    return best;
}

/* ---- Split an overfull node ---- */

int BirchClustering15::splitNode(int nodeIdx)
{
    // Create a new node by splitting: take the farthest pair of children
    const CFNode& node = m_nodes[nodeIdx];
    if (node.childIndices.size() < 2) return -1;

    // Find two farthest child centroids
    int farA = 0, farB = 1;
    double maxDist = 0.0;
    for (int i = 0; i < node.childIndices.size(); ++i) {
        for (int j = i + 1; j < node.childIndices.size(); ++j) {
            int ci = node.childIndices[i], cj = node.childIndices[j];
            double d = 0.0;
            int dims = m_nodes[ci].linearSum.size();
            for (int k = 0; k < dims; ++k) {
                double diff = m_nodes[ci].linearSum[k] / qMax(m_nodes[ci].n, 1)
                            - m_nodes[cj].linearSum[k] / qMax(m_nodes[cj].n, 1);
                d += diff * diff;
            }
            if (d > maxDist) { maxDist = d; farA = i; farB = j; }
        }
    }

    // Assign children to nearest of the two seeds
    int seedA = node.childIndices[farA];
    int seedB = node.childIndices[farB];
    QVector<double> centA(m_dims), centB(m_dims);
    for (int k = 0; k < m_dims; ++k) {
        centA[k] = m_nodes[seedA].linearSum[k] / qMax(m_nodes[seedA].n, 1);
        centB[k] = m_nodes[seedB].linearSum[k] / qMax(m_nodes[seedB].n, 1);
    }

    // New sibling node
    CFNode sibling;
    sibling.isLeaf = node.isLeaf;
    sibling.linearSum.resize(m_dims);
    sibling.squareSum.resize(m_dims);
    sibling.parent = node.parent;

    QVector<int> keep, move;
    for (int i = 0; i < node.childIndices.size(); ++i) {
        int ci = node.childIndices[i];
        double dA = 0.0, dB = 0.0;
        int nci = qMax(m_nodes[ci].n, 1);
        for (int k = 0; k < m_dims; ++k) {
            double c = m_nodes[ci].linearSum[k] / nci;
            dA += (c - centA[k]) * (c - centA[k]);
            dB += (c - centB[k]) * (c - centB[k]);
        }
        if (dA <= dB) keep.append(ci);
        else move.append(ci);
    }

    // Rebuild current node
    m_nodes[nodeIdx].childIndices = keep;
    m_nodes[nodeIdx].n = 0;
    m_nodes[nodeIdx].linearSum.fill(0.0);
    m_nodes[nodeIdx].squareSum.fill(0.0);
    for (int ci : keep) mergeCFs(m_nodes[nodeIdx], m_nodes[ci]);

    sibling.childIndices = move;
    for (int ci : move) {
        mergeCFs(sibling, m_nodes[ci]);
        m_nodes[ci].parent = m_nodes.size();
    }

    int siblingIdx = m_nodes.size();
    m_nodes.append(sibling);
    return siblingIdx;
}

/* ---- Dynamic threshold adjustment ---- */

void BirchClustering15::adjustThreshold()
{
    int leafCount = 0;
    for (const auto& nd : m_nodes) {
        if (nd.isLeaf && nd.n > 0) leafCount++;
    }
    // If leaf count exceeds branching factor * 4, increase threshold
    if (leafCount > m_branching * 4) {
        m_threshold *= 1.2;
    } else if (leafCount < m_branching / 2 && m_threshold > 0.05) {
        m_threshold *= 0.9;
    }
}

/* ---- Collect leaf CFs ---- */

void BirchClustering15::collectLeaves(int idx, QVector<int>& leaves) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    if (m_nodes[idx].isLeaf) {
        if (m_nodes[idx].n > 0) leaves.append(idx);
        return;
    }
    for (int ci : m_nodes[idx].childIndices)
        collectLeaves(ci, leaves);
}

/* ---- Insert a single point ---- */

void BirchClustering15::insertPoint(const QVector<double>& point)
{
    if (point.isEmpty()) return;
    if (m_dims == 0) m_dims = point.size();

    // Initialize root if empty
    if (m_root < 0) {
        CFNode root;
        root.isLeaf = true;
        root.linearSum.resize(m_dims);
        root.squareSum.resize(m_dims);
        m_nodes.append(root);
        m_root = 0;
    }

    // Find closest leaf subcluster
    int closest = findClosestLeaf(point);
    if (closest < 0) {
        // Create new leaf
        CFNode leaf;
        leaf.isLeaf = true;
        leaf.linearSum.resize(m_dims);
        leaf.squareSum.resize(m_dims);
        mergeIntoCF(leaf, point);
        m_nodes.append(leaf);
        return;
    }

    // Check if point fits within threshold
    CFNode testCF = m_nodes[closest];
    mergeIntoCF(testCF, point);
    double newRadius = cfRadius(testCF);

    if (newRadius <= m_threshold) {
        // Absorb point into this subcluster
        mergeIntoCF(m_nodes[closest], point);
    } else {
        // Create new subcluster
        CFNode newLeaf;
        newLeaf.isLeaf = true;
        newLeaf.linearSum.resize(m_dims);
        newLeaf.squareSum.resize(m_dims);
        mergeIntoCF(newLeaf, point);
        m_nodes.append(newLeaf);

        // Update parent CF summaries upward
        int parent = m_nodes[closest].parent;
        while (parent >= 0) {
            mergeIntoCF(m_nodes[parent], point);
            // Check for split needed
            if (m_nodes[parent].childIndices.size() > m_branching) {
                splitNode(parent);
            }
            parent = m_nodes[parent].parent;
        }
    }

    // Periodically adjust threshold
    if (m_stats.numPoints % 500 == 0)
        adjustThreshold();

    m_stats.numPoints++;
}

/* ---- Global clustering via simplified k-means on leaf centroids ---- */

BirchClustering15::ClusterResult BirchClustering15::globalClustering(
    const QVector<QVector<double>>& data)
{
    ClusterResult result;

    QVector<int> leaves;
    collectLeaves(m_root, leaves);
    int numSub = leaves.size();
    if (numSub == 0) return result;

    // Collect subcluster centroids weighted by n
    QVector<QVector<double>> centroids(numSub);
    QVector<int> weights(numSub);
    for (int i = 0; i < numSub; ++i) {
        const CFNode& cf = m_nodes[leaves[i]];
        centroids[i].resize(m_dims);
        for (int d = 0; d < m_dims; ++d)
            centroids[i][d] = cf.linearSum[d] / cf.n;
        weights[i] = cf.n;
    }

    // K-means on subcluster centroids
    int k = qMin(m_k, numSub);
    QVector<QVector<double>> centers(k);
    for (int i = 0; i < k; ++i)
        centers[i] = centroids[i % numSub];

    // Run 10 iterations of k-means
    QVector<int> assign(numSub, 0);
    for (int iter = 0; iter < 10; ++iter) {
        // Assign each subcluster to nearest center
        for (int i = 0; i < numSub; ++i) {
            double bestD = 1e300;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                for (int dd = 0; dd < m_dims; ++dd) {
                    double diff = centroids[i][dd] - centers[c][dd];
                    d += diff * diff;
                }
                if (d < bestD) { bestD = d; assign[i] = c; }
            }
        }
        // Recompute centers as weighted average
        for (int c = 0; c < k; ++c) {
            centers[c].fill(0.0);
            double totalW = 0.0;
            for (int i = 0; i < numSub; ++i) {
                if (assign[i] != c) continue;
                double w = weights[i];
                for (int dd = 0; dd < m_dims; ++dd)
                    centers[c][dd] += centroids[i][dd] * w;
                totalW += w;
            }
            if (totalW > 0)
                for (int dd = 0; dd < m_dims; ++dd)
                    centers[c][dd] /= totalW;
        }
    }

    // Map each original point to cluster label via its leaf subcluster
    result.numClusters = k;
    result.centroids = centers;
    result.labels.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
        // Find closest leaf
        int closest = findClosestLeaf(data[i]);
        if (closest < 0) { result.labels[i] = 0; continue; }
        // Find which subcluster index
        int subIdx = leaves.indexOf(closest);
        result.labels[i] = (subIdx >= 0) ? assign[subIdx] : 0;
    }
    return result;
}

/* ---- Batch fit ---- */

BirchClustering15::ClusterResult BirchClustering15::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = qMin(data.size(), m_maxPoints);
    if (n == 0) return result;

    // Insert all points
    for (int i = 0; i < n; ++i)
        insertPoint(data[i]);

    // Apply global clustering
    result = globalClustering(data);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    QVector<int> leaves;
    collectLeaves(m_root, leaves);
    m_stats.numSubclusters = leaves.size();

    emit fitDone(n, result.numClusters, m_stats.numSubclusters, elapsed);
    return result;
}

/* ---- Get subcluster centroids ---- */

QVector<QVector<double>> BirchClustering15::getSubclusterCentroids() const
{
    QVector<int> leaves;
    const_cast<BirchClustering15*>(this)->collectLeaves(m_root, leaves);
    QVector<QVector<double>> cents;
    for (int idx : leaves) {
        const CFNode& cf = m_nodes[idx];
        QVector<double> c(m_dims);
        for (int d = 0; d < m_dims; ++d)
            c[d] = cf.linearSum[d] / cf.n;
        cents.append(c);
    }
    return cents;
}

/* ---- Reset ---- */

void BirchClustering15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
    m_dims = 0;
}
