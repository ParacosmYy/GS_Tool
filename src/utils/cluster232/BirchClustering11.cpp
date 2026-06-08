/**
 * @file BirchClustering11.cpp
 * @brief BirchClustering11 实现
 *
 * 实现BIRCH聚类：自适应阈值与LRU子树淘汰的内存受限CF树。
 */

#include "utils/cluster232/BirchClustering11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BirchClustering11::BirchClustering11(QObject *parent) : QObject(parent) {}
BirchClustering11::~BirchClustering11() = default;

/* ---- Configuration ---- */

void BirchClustering11::setThreshold(double t) { m_threshold = qMax(1e-6, t); }
void BirchClustering11::setMaxMemory(int maxEntries) { m_maxMemory = qMax(10, maxEntries); }
void BirchClustering11::setBranchingFactor(int b) { m_branchingFactor = qMax(2, b); }

/* ---- CF operations ---- */

BirchClustering11::CFEntry BirchClustering11::mergeCF(const CFEntry& a, const CFEntry& b) const
{
    CFEntry result;
    result.n = a.n + b.n;
    result.ss = a.ss + b.ss;
    result.ls.resize(a.ls.size());
    for (int i = 0; i < a.ls.size(); ++i)
        result.ls[i] = a.ls[i] + b.ls[i];
    return result;
}

double BirchClustering11::cfRadius(const CFEntry& cf) const
{
    if (cf.n == 0) return 0.0;
    int d = cf.ls.size();
    double sum = 0.0;
    for (int i = 0; i < d; ++i) {
        double centroid = cf.ls[i] / cf.n;
        // Radius^2 = (ss/n) - ||centroid||^2
        sum += centroid * centroid;
    }
    return qSqrt(qMax(0.0, cf.ss / cf.n - sum));
}

QVector<double> BirchClustering11::cfCentroid(const CFEntry& cf) const
{
    int d = cf.ls.size();
    QVector<double> c(d, 0.0);
    if (cf.n == 0) return c;
    for (int i = 0; i < d; ++i)
        c[i] = cf.ls[i] / cf.n;
    return c;
}

/* ---- Distance ---- */

double BirchClustering11::euclideanDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Find closest leaf entry ---- */

int BirchClustering11::findClosestEntry(const QVector<double>& point) const
{
    int bestIdx = 0;
    double bestDist = std::numeric_limits<double>::max();
    for (int i = 0; i < m_leafEntries.size(); ++i) {
        QVector<double> cen = cfCentroid(m_leafEntries[i]);
        double d = euclideanDist(point, cen);
        if (d < bestDist) {
            bestDist = d;
            bestIdx = i;
        }
    }
    return bestIdx;
}

/* ---- Adaptive threshold ---- */

double BirchClustering11::adaptThreshold()
{
    // Compute average radius of leaf entries
    double avgR = 0.0;
    int cnt = 0;
    for (const auto& cf : m_leafEntries) {
        double r = cfRadius(cf);
        avgR += r;
        cnt++;
    }
    if (cnt > 0) avgR /= cnt;

    // Increase threshold if many entries, decrease if few
    if (m_leafEntries.size() > m_maxMemory * 0.8)
        return m_threshold * 1.5;
    if (m_leafEntries.size() < m_maxMemory * 0.3 && m_threshold > 1e-6)
        return m_threshold * 0.8;
    return m_threshold;
}

/* ---- Evict LRU entries ---- */

void BirchClustering11::evictLRU()
{
    if (m_leafEntries.size() <= m_maxMemory) return;

    // Find entry with smallest access timestamp (LRU)
    // Merge the two closest entries to free space
    int mergeI = 0, mergeJ = 1;
    double minDist = std::numeric_limits<double>::max();

    int n = m_leafEntries.size();
    for (int i = 0; i < n; ++i) {
        QVector<double> ci = cfCentroid(m_leafEntries[i]);
        for (int j = i + 1; j < n; ++j) {
            QVector<double> cj = cfCentroid(m_leafEntries[j]);
            double d = euclideanDist(ci, cj);
            if (d < minDist) {
                minDist = d;
                mergeI = i;
                mergeJ = j;
            }
        }
    }

    // Merge j into i
    m_leafEntries[mergeI] = mergeCF(m_leafEntries[mergeI], m_leafEntries[mergeJ]);
    m_leafEntries.removeAt(mergeJ);
    m_stats.numEvictions++;
}

/* ---- Rebuild tree ---- */

void BirchClustering11::rebuildTree()
{
    double newThreshold = adaptThreshold();
    if (qFuzzyCompare(newThreshold, m_threshold) && m_leafEntries.size() <= m_maxMemory)
        return;

    m_threshold = newThreshold;

    // Re-cluster existing entries by merging close ones
    QVector<CFEntry> rebuilt;
    QVector<bool> used(m_leafEntries.size(), false);

    for (int i = 0; i < m_leafEntries.size(); ++i) {
        if (used[i]) continue;
        CFEntry merged = m_leafEntries[i];
        used[i] = true;
        for (int j = i + 1; j < m_leafEntries.size(); ++j) {
            if (used[j]) continue;
            CFEntry testMerge = mergeCF(merged, m_leafEntries[j]);
            if (cfRadius(testMerge) <= m_threshold) {
                merged = testMerge;
                used[j] = true;
            }
        }
        rebuilt.append(merged);
    }
    m_leafEntries = rebuilt;
    m_stats.numLeafEntries = m_leafEntries.size();
}

/* ---- Assign points to centroids ---- */

void BirchClustering11::assignPoints(const QVector<QVector<double>>& ctrs)
{
    int n = m_data.size();
    m_assignments.resize(n);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        int bestC = 0;
        for (int c = 0; c < ctrs.size(); ++c) {
            double d = euclideanDist(m_data[i], ctrs[c]);
            if (d < bestDist) {
                bestDist = d;
                bestC = c;
            }
        }
        m_assignments[i].clusterId = bestC;
        m_assignments[i].distance = bestDist;
    }
}

/* ---- Fit ---- */

bool BirchClustering11::fit(const QVector<QVector<double>>& data, int numClusters)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return false;
    int d = data[0].size();

    m_data = data;
    m_leafEntries.clear();
    m_accessCounter = 0;

    // Phase 1: Build CF tree by inserting points
    for (int i = 0; i < n; ++i) {
        CFEntry pointCF;
        pointCF.n = 1;
        pointCF.ls = data[i];
        pointCF.ss = 0.0;
        for (int j = 0; j < d; ++j)
            pointCF.ss += data[i][j] * data[i][j];

        if (m_leafEntries.isEmpty()) {
            m_leafEntries.append(pointCF);
        } else {
            int closest = findClosestEntry(data[i]);
            CFEntry testMerge = mergeCF(m_leafEntries[closest], pointCF);
            if (cfRadius(testMerge) <= m_threshold) {
                m_leafEntries[closest] = testMerge;
            } else {
                m_leafEntries.append(pointCF);
            }
        }

        // Memory-bounded: evict if needed
        if (m_leafEntries.size() > m_maxMemory) {
            evictLRU();
            if (m_leafEntries.size() > m_maxMemory) {
                rebuildTree();
                emit treeRebuilt(m_leafEntries.size(), m_threshold);
            }
        }
        emit pointInserted(i, m_leafEntries.size() - 1);
    }

    // Phase 2: Merge leaf entries to desired number of clusters
    while (m_leafEntries.size() > numClusters) {
        int mi = 0, mj = 1;
        double minD = std::numeric_limits<double>::max();
        for (int i = 0; i < m_leafEntries.size(); ++i) {
            QVector<double> ci = cfCentroid(m_leafEntries[i]);
            for (int j = i + 1; j < m_leafEntries.size(); ++j) {
                QVector<double> cj = cfCentroid(m_leafEntries[j]);
                double dd = euclideanDist(ci, cj);
                if (dd < minD) { minD = dd; mi = i; mj = j; }
            }
        }
        m_leafEntries[mi] = mergeCF(m_leafEntries[mi], m_leafEntries[mj]);
        m_leafEntries.removeAt(mj);
    }

    // Phase 3: Compute centroids and assign points
    m_centroids.clear();
    for (const auto& cf : m_leafEntries)
        m_centroids.append(cfCentroid(cf));

    assignPoints(m_centroids);

    m_stats.numPoints = n;
    m_stats.numDimensions = d;
    m_stats.numLeafEntries = m_leafEntries.size();
    m_stats.threshold = m_threshold;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(numClusters, m_leafEntries.size(), timer.elapsed());
    return true;
}

/* ---- Accessors ---- */

QVector<BirchClustering11::Assignment> BirchClustering11::assignments() const { return m_assignments; }
QVector<BirchClustering11::CFEntry> BirchClustering11::leafEntries() const { return m_leafEntries; }
QVector<QVector<double>> BirchClustering11::centroids() const { return m_centroids; }

/* ---- Reset ---- */

void BirchClustering11::resetStatistics()
{
    m_leafEntries.clear();
    m_assignments.clear();
    m_centroids.clear();
    m_data.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_accessCounter = 0;
}
