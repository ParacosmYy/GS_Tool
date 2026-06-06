/**
 * @file BirchClustering5.cpp
 * @brief BirchClustering5 实现
 *
 * 实现BIRCH聚类：CF树增量插入、叶节点细化、全局K-Means聚类。
 */

#include "utils/cluster164/BirchClustering5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

BirchClustering5::BirchClustering5(QObject* parent)
    : QObject(parent)
{
}

BirchClustering5::~BirchClustering5() = default;

void BirchClustering5::setThreshold(double threshold)
{
    m_threshold = qMax(1e-10, threshold);
}

void BirchClustering5::setBranchFactor(int b)
{
    m_branchFactor = qMax(2, b);
}

QVector<double> BirchClustering5::centroid(const CFEntry& cf)
{
    if (cf.n == 0) return QVector<double>();
    QVector<double> c(cf.ls.size());
    for (int i = 0; i < cf.ls.size(); ++i)
        c[i] = cf.ls[i] / cf.n;
    return c;
}

double BirchClustering5::cfDistance(const CFEntry& a, const CFEntry& b)
{
    QVector<double> ca = centroid(a);
    QVector<double> cb = centroid(b);
    double sum = 0.0;
    int dim = qMin(ca.size(), cb.size());
    for (int i = 0; i < dim; ++i) {
        double d = ca[i] - cb[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

BirchClustering5::CFEntry BirchClustering5::mergeCF(const CFEntry& a, const CFEntry& b)
{
    CFEntry result;
    result.n = a.n + b.n;
    result.ss = a.ss + b.ss;
    int dim = qMax(a.ls.size(), b.ls.size());
    result.ls.resize(dim, 0.0);
    for (int i = 0; i < a.ls.size(); ++i) result.ls[i] += a.ls[i];
    for (int i = 0; i < b.ls.size(); ++i) result.ls[i] += b.ls[i];
    return result;
}

int BirchClustering5::findNearest(const QVector<double>& point) const
{
    int bestIdx = -1;
    double bestDist = std::numeric_limits<double>::max();
    for (int i = 0; i < m_leafEntries.size(); ++i) {
        QVector<double> c = centroid(m_leafEntries[i]);
        double dist = 0.0;
        int dim = qMin(c.size(), point.size());
        for (int d = 0; d < dim; ++d) {
            double diff = c[d] - point[d];
            dist += diff * diff;
        }
        dist = qSqrt(dist);
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = i;
        }
    }
    return bestIdx;
}

bool BirchClustering5::insert(const QVector<double>& point)
{
    if (point.isEmpty()) return false;

    /* If no leaf entries, create first one */
    if (m_leafEntries.isEmpty()) {
        CFEntry cf;
        cf.n = 1;
        cf.ls = point;
        double ss = 0.0;
        for (double v : point) ss += v * v;
        cf.ss = ss;
        m_leafEntries.append(cf);
        m_allPoints.append(point);
        m_pointToEntry.append(0);
        return true;
    }

    /* Find nearest leaf CF */
    int nearIdx = findNearest(point);
    if (nearIdx < 0) return false;

    /* Check threshold: compute radius if point absorbed */
    CFEntry& cf = m_leafEntries[nearIdx];
    CFEntry testCf = mergeCF(cf, CFEntry{1, point, 0.0});
    for (double v : point) testCf.ss += v * v;

    /* Average distance check using centroid distance */
    QVector<double> newCentroid = centroid(testCf);
    double avgDist = 0.0;
    if (testCf.n > 1) {
        double radius = 0.0;
        /* Estimate radius from CF statistics */
        double sumSq = 0.0;
        for (int i = 0; i < newCentroid.size(); ++i)
            sumSq += (testCf.ls[i] / testCf.n) * (testCf.ls[i] / testCf.n);
        radius = qSqrt(qMax(0.0, (testCf.ss - sumSq * testCf.n) / testCf.n));
        avgDist = radius;
    }

    if (avgDist <= m_threshold || cf.n == 0) {
        /* Absorb into existing CF */
        cf.n++;
        for (int i = 0; i < point.size(); ++i) {
            if (i < cf.ls.size()) cf.ls[i] += point[i];
        }
        double ss = 0.0;
        for (double v : point) ss += v * v;
        cf.ss += ss;
        m_allPoints.append(point);
        m_pointToEntry.append(nearIdx);
    } else {
        /* Create new leaf CF */
        CFEntry newCf;
        newCf.n = 1;
        newCf.ls = point;
        double ss = 0.0;
        for (double v : point) ss += v * v;
        newCf.ss = ss;
        m_leafEntries.append(newCf);
        m_allPoints.append(point);
        m_pointToEntry.append(m_leafEntries.size() - 1);

        /* If too many leaf entries, merge closest pair */
        if (m_leafEntries.size() > m_branchFactor) {
            double minDist = std::numeric_limits<double>::max();
            int mi = 0, mj = 1;
            for (int i = 0; i < m_leafEntries.size(); ++i) {
                for (int j = i + 1; j < m_leafEntries.size(); ++j) {
                    double d = cfDistance(m_leafEntries[i], m_leafEntries[j]);
                    if (d < minDist) { minDist = d; mi = i; mj = j; }
                }
            }
            /* Merge mi into mj, remap */
            m_leafEntries[mi] = mergeCF(m_leafEntries[mi], m_leafEntries[mj]);
            m_leafEntries.removeAt(mj);
            /* Remap point-to-entry indices */
            for (int p = 0; p < m_pointToEntry.size(); ++p) {
                if (m_pointToEntry[p] == mj) m_pointToEntry[p] = mi;
                else if (m_pointToEntry[p] > mj) m_pointToEntry[p]--;
            }
        }
    }
    return true;
}

void BirchClustering5::buildTree(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_leafEntries.clear();
    m_allPoints.clear();
    m_pointToEntry.clear();

    for (const auto& point : data) {
        insert(point);
    }

    m_stats.totalRuns++;
    m_stats.lastLeafCount = m_leafEntries.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit treeBuilt(m_leafEntries.size());
}

QVector<int> BirchClustering5::kMeans(const QVector<CFEntry>& entries, int k) const
{
    if (entries.isEmpty() || k <= 0) return QVector<int>();
    k = qMin(k, entries.size());

    /* Initialize centroids from first k entries */
    QVector<QVector<double>> centroids(k);
    for (int i = 0; i < k; ++i)
        centroids[i] = centroid(entries[i]);

    QVector<int> labels(entries.size(), 0);
    const int maxIter = 100;

    for (int iter = 0; iter < maxIter; ++iter) {
        bool changed = false;
        /* Assign each entry to nearest centroid */
        for (int i = 0; i < entries.size(); ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                QVector<double> ec = centroid(entries[i]);
                int dim = qMin(ec.size(), centroids[c].size());
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = ec[dd] - centroids[c][dd];
                    d += diff * diff;
                }
                if (d < bestDist) { bestDist = d; bestC = c; }
            }
            if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
        }
        if (!changed) break;

        /* Recompute centroids weighted by CF count */
        for (int c = 0; c < k; ++c) {
            CFEntry sum;
            sum.n = 0;
            bool first = true;
            for (int i = 0; i < entries.size(); ++i) {
                if (labels[i] == c) {
                    if (first) { sum = entries[i]; first = false; }
                    else sum = mergeCF(sum, entries[i]);
                }
            }
            centroids[c] = centroid(sum);
        }
    }
    return labels;
}

QVector<int> BirchClustering5::globalCluster(int k)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result(m_allPoints.size(), 0);
    if (m_leafEntries.isEmpty()) return result;

    QVector<int> entryLabels = kMeans(m_leafEntries, k);

    /* Map point-level labels */
    for (int i = 0; i < m_allPoints.size(); ++i) {
        int entryIdx = m_pointToEntry[i];
        if (entryIdx >= 0 && entryIdx < entryLabels.size())
            result[i] = entryLabels[entryIdx];
    }

    m_stats.lastClusterCount = k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(k);
    return result;
}

QVector<BirchClustering5::CFEntry> BirchClustering5::getLeafEntries() const
{
    return m_leafEntries;
}

void BirchClustering5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
