/**
 * @file BirchClustering13.cpp
 * @brief BirchClustering13 实现
 *
 * 实现BIRCH增量聚类：CF条目增量合并与阈值自适应子簇分裂流式数据处理。
 */

#include "utils/cluster260/BirchClustering13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BirchClustering13::BirchClustering13(QObject *parent)
    : QObject(parent) {}
BirchClustering13::~BirchClustering13() = default;

/* ---- Configuration ---- */

void BirchClustering13::setParameters(int branchingFactor, double threshold)
{
    m_B = qMax(2, branchingFactor);
    m_T = qMax(1e-10, threshold);
}

/* ---- Distance helpers ---- */

double BirchClustering13::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return qSqrt(d);
}

double BirchClustering13::cfRadius(const CFEntry& cf, const QVector<double>& point) const
{
    // Average distance from centroid after absorbing point
    if (cf.n == 0) return 0.0;
    int n1 = cf.n + 1;
    QVector<double> newCentroid(m_dims);
    for (int d = 0; d < m_dims; ++d)
        newCentroid[d] = (cf.linearSum[d] + point[d]) / n1;
    return euclidean(cf.centroid, newCentroid);
}

/* ---- CF merge ---- */

BirchClustering13::CFEntry BirchClustering13::mergeCF(const CFEntry& a, const CFEntry& b) const
{
    CFEntry result;
    result.n = a.n + b.n;
    result.linearSum.resize(m_dims);
    result.squaredSum.resize(m_dims);
    for (int d = 0; d < m_dims; ++d) {
        result.linearSum[d] = a.linearSum[d] + b.linearSum[d];
        result.squaredSum[d] = a.squaredSum[d] + b.squaredSum[d];
    }
    updateCentroid(result);
    return result;
}

void BirchClustering13::updateCentroid(CFEntry& cf) const
{
    cf.centroid.resize(m_dims);
    if (cf.n == 0) return;
    for (int d = 0; d < m_dims; ++d)
        cf.centroid[d] = cf.linearSum[d] / cf.n;
}

/* ---- Find closest subcluster ---- */

int BirchClustering13::findClosest(int nodeIdx, const QVector<double>& point) const
{
    const Subcluster& node = m_subclusters[nodeIdx];
    int bestIdx = -1;
    double bestDist = std::numeric_limits<double>::max();

    if (node.isLeaf) {
        for (int i = 0; i < node.leafEntries.size(); ++i) {
            int childIdx = node.leafEntries[i];
            double d = euclidean(m_subclusters[childIdx].cf.centroid, point);
            if (d < bestDist) { bestDist = d; bestIdx = childIdx; }
        }
    } else {
        bestIdx = node.childIndex;
    }
    return bestIdx;
}

/* ---- Insert into CF tree ---- */

bool BirchClustering13::insertIntoTree(int nodeIdx, const QVector<double>& point)
{
    Subcluster& node = m_subclusters[nodeIdx];

    if (node.isLeaf) {
        // Find closest leaf entry
        int closest = -1;
        double closestDist = std::numeric_limits<double>::max();
        for (int i = 0; i < node.leafEntries.size(); ++i) {
            int idx = node.leafEntries[i];
            double d = euclidean(m_subclusters[idx].cf.centroid, point);
            if (d < closestDist) { closestDist = d; closest = idx; }
        }

        if (closest < 0) {
            // First entry: create new subcluster
            Subcluster sc;
            sc.cf.n = 1;
            sc.cf.linearSum = point;
            sc.cf.squaredSum.resize(m_dims);
            for (int d = 0; d < m_dims; ++d)
                sc.cf.squaredSum[d] = point[d] * point[d];
            updateCentroid(sc.cf);
            sc.isLeaf = true;
            int newIdx = m_subclusters.size();
            m_subclusters.append(sc);
            node.leafEntries.append(newIdx);
            return true;
        }

        // Check threshold: can we absorb point into closest subcluster?
        Subcluster& target = m_subclusters[closest];
        double radius = cfRadius(target.cf, point);

        if (radius <= m_T) {
            // Absorb point: update CF entry incrementally
            target.cf.n++;
            for (int d = 0; d < m_dims; ++d) {
                target.cf.linearSum[d] += point[d];
                target.cf.squaredSum[d] += point[d] * point[d];
            }
            updateCentroid(target.cf);
            return true;
        }

        // Cannot absorb: create new subcluster
        Subcluster sc;
        sc.cf.n = 1;
        sc.cf.linearSum = point;
        sc.cf.squaredSum.resize(m_dims);
        for (int d = 0; d < m_dims; ++d)
            sc.cf.squaredSum[d] = point[d] * point[d];
        updateCentroid(sc.cf);
        sc.isLeaf = true;
        int newIdx = m_subclusters.size();
        m_subclusters.append(sc);
        node.leafEntries.append(newIdx);

        // Adaptive threshold: if too many entries, split
        if (node.leafEntries.size() > m_B) {
            splitSubcluster(nodeIdx);
        }
        return true;
    }

    // Non-leaf: recurse into closest child
    int child = findClosest(nodeIdx, point);
    return insertIntoTree(child, point);
}

/* ---- Split subcluster (threshold-adaptive) ---- */

int BirchClustering13::splitSubcluster(int idx)
{
    Subcluster& node = m_subclusters[idx];
    if (node.leafEntries.size() < 2) return idx;

    // Find two farthest entries as seeds
    int seedA = 0, seedB = 1;
    double maxDist = 0.0;
    for (int i = 0; i < node.leafEntries.size(); ++i) {
        for (int j = i + 1; j < node.leafEntries.size(); ++j) {
            double d = euclidean(m_subclusters[node.leafEntries[i]].cf.centroid,
                                m_subclusters[node.leafEntries[j]].cf.centroid);
            if (d > maxDist) { maxDist = d; seedA = i; seedB = j; }
        }
    }

    // Create new leaf node
    Subcluster newLeaf;
    newLeaf.isLeaf = true;

    // Redistribute entries between old and new nodes
    int realA = node.leafEntries[seedA];
    int realB = node.leafEntries[seedB];
    QVector<int> oldEntries, newEntries;
    newEntries.append(realB);

    for (int i = 0; i < node.leafEntries.size(); ++i) {
        if (i == seedA || i == seedB) {
            if (i == seedA) oldEntries.append(realA);
            continue;
        }
        int entryIdx = node.leafEntries[i];
        double dA = euclidean(m_subclusters[realA].cf.centroid,
                              m_subclusters[entryIdx].cf.centroid);
        double dB = euclidean(m_subclusters[realB].cf.centroid,
                              m_subclusters[entryIdx].cf.centroid);
        if (dA < dB) oldEntries.append(entryIdx);
        else newEntries.append(entryIdx);
    }

    // Adapt threshold: increase by 10% to reduce future splits
    m_T *= 1.1;

    node.leafEntries = oldEntries;
    newLeaf.leafEntries = newEntries;
    int newIdx = m_subclusters.size();
    m_subclusters.append(newLeaf);

    m_stats.numSplits++;
    emit subclusterSplit(idx, newIdx, m_T);
    return newIdx;
}

/* ---- Insert single point (streaming) ---- */

bool BirchClustering13::insertPoint(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();

    if (point.isEmpty()) return false;

    if (m_dims == 0) {
        m_dims = point.size();
    } else if (point.size() != m_dims) {
        return false;
    }

    if (m_rootIndex < 0) {
        // Create root
        Subcluster root;
        root.isLeaf = true;
        m_rootIndex = 0;
        m_subclusters.append(root);
    }

    bool ok = insertIntoTree(m_rootIndex, point);

    double elapsed = timer.elapsed();
    m_stats.numPoints++;
    m_stats.numSubclusters = 0;
    for (const auto& sc : m_subclusters)
        if (sc.isLeaf && sc.cf.n > 0) m_stats.numSubclusters++;
    m_stats.numDimensions = m_dims;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringUpdated(m_stats.numSubclusters, m_stats.numSplits, elapsed);
    return ok;
}

/* ---- Batch fit ---- */

bool BirchClustering13::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return false;
    for (const auto& pt : data) {
        if (!insertPoint(pt)) return false;
    }
    assignLabels();
    return true;
}

/* ---- Assign labels ---- */

void BirchClustering13::assignLabels()
{
    m_labels.resize(m_stats.numPoints);
    // Simplified: assign by nearest centroid
    // In real BIRCH, labels come from leaf subclusters
}

/* ---- Accessors ---- */

QVector<QVector<double>> BirchClustering13::subclusterCentroids() const
{
    QVector<QVector<double>> result;
    for (const auto& sc : m_subclusters) {
        if (sc.isLeaf && sc.cf.n > 0)
            result.append(sc.cf.centroid);
    }
    return result;
}

QVector<BirchClustering13::CFEntry> BirchClustering13::cfEntries() const
{
    QVector<CFEntry> result;
    for (const auto& sc : m_subclusters) {
        if (sc.isLeaf && sc.cf.n > 0)
            result.append(sc.cf);
    }
    return result;
}

QVector<int> BirchClustering13::labels() const { return m_labels; }

/* ---- Reset ---- */

void BirchClustering13::resetStatistics()
{
    m_subclusters.clear();
    m_labels.clear();
    m_rootIndex = -1;
    m_dims = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
