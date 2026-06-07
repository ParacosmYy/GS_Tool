/**
 * @file BirchClustering9.cpp
 * @brief BirchClustering9 实现
 *
 * 实现BIRCH增量聚类：CF树构建、多探测合并、自适应重平衡。
 */

#include "utils/cluster204/BirchClustering9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BirchClustering9::BirchClustering9(QObject *parent) : QObject(parent) {}
BirchClustering9::~BirchClustering9() = default;

/* ---- Configuration ---- */

void BirchClustering9::setThreshold(double t) { m_threshold = qMax(1e-6, t); }
void BirchClustering9::setBranchingFactor(int b) { m_branching = qMax(2, b); }
void BirchClustering9::setMaxClusters(int k) { m_maxClusters = qMax(1, k); }

/* ---- Euclidean distance ---- */

double BirchClustering9::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Find nearest CF entry ---- */

int BirchClustering9::findNearest(const QVector<double>& point) const
{
    if (m_entries.isEmpty()) return -1;
    int best = 0;
    double bestDist = std::numeric_limits<double>::max();
    for (int i = 0; i < m_entries.size(); ++i) {
        QVector<double> centroid = cfCentroid(m_entries[i]);
        double d = euclidean(point, centroid);
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

/* ---- Absorb point into CF entry ---- */

void BirchClustering9::absorbPoint(CFEntry& cf, const QVector<double>& point)
{
    cf.n++;
    if (cf.ls.isEmpty()) cf.ls.resize(point.size(), 0.0);
    for (int i = 0; i < point.size(); ++i) {
        cf.ls[i] += point[i];
        cf.ss += point[i] * point[i];
    }
}

/* ---- CF centroid ---- */

QVector<double> BirchClustering9::cfCentroid(const CFEntry& cf) const
{
    if (cf.n == 0) return QVector<double>(m_dim, 0.0);
    QVector<double> c(cf.ls.size());
    for (int i = 0; i < cf.ls.size(); ++i)
        c[i] = cf.ls[i] / cf.n;
    return c;
}

/* ---- CF distance (centroid-based) ---- */

double BirchClustering9::cfDistance(const CFEntry& a, const CFEntry& b) const
{
    return euclidean(cfCentroid(a), cfCentroid(b));
}

/* ---- Split over-capacity entry ---- */

void BirchClustering9::splitEntry(int idx)
{
    CFEntry& entry = m_entries[idx];
    if (entry.n < 2) return;

    // Find dimension with largest variance for split
    int bestDim = 0;
    double maxVar = 0.0;
    for (int d = 0; d < entry.ls.size(); ++d) {
        double mean = entry.ls[d] / entry.n;
        double var = entry.ss / entry.n - mean * mean;
        if (var > maxVar) { maxVar = var; bestDim = d; }
    }

    // Create new entry by splitting centroid offset
    CFEntry newEntry;
    newEntry.n = entry.n / 2;
    newEntry.ls.resize(entry.ls.size(), 0.0);
    double splitVal = entry.ls[bestDim] / entry.n;

    // Redistribute: lower half stays, upper half goes to new
    for (int d = 0; d < entry.ls.size(); ++d) {
        double mean = entry.ls[d] / entry.n;
        newEntry.ls[d] = mean * newEntry.n + m_threshold * (d == bestDim ? 1.0 : 0.0);
        entry.ls[d] -= newEntry.ls[d];
    }
    newEntry.ss = newEntry.n * (entry.ss / entry.n) * 0.5;
    entry.n -= newEntry.n;

    m_entries.append(newEntry);
}

/* ---- Insert single point ---- */

void BirchClustering9::insertPoint(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();
    m_dim = point.size();
    m_rawPoints.append(point);

    if (m_entries.isEmpty()) {
        CFEntry cf;
        cf.isLeaf = true;
        absorbPoint(cf, point);
        m_entries.append(cf);
    } else {
        int nearest = findNearest(point);
        double dist = cfDistance(m_entries[nearest], CFEntry{1, point, 0.0, -1, true});

        if (dist <= m_threshold) {
            absorbPoint(m_entries[nearest], point);
        } else {
            CFEntry cf;
            cf.isLeaf = true;
            absorbPoint(cf, point);
            m_entries.append(cf);

            // Split if exceeds branching factor
            if (m_entries.size() > m_branching)
                splitEntry(m_entries.size() - 1);
        }
    }

    m_stats.treeHeight = 1;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Bulk fit ---- */

void BirchClustering9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    for (const auto& pt : data)
        insertPoint(pt);

    m_stats.numPoints = data.size();
    m_stats.numClusters = m_entries.size();
    emit clusteringCompleted(m_stats.numClusters, timer.elapsed());
}

/* ---- Multi-probe merge ---- */

void BirchClustering9::multiProbeMerge(double mergeThreshold)
{
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m_entries.size(); ++i) {
            for (int j = i + 1; j < m_entries.size(); ++j) {
                double d = cfDistance(m_entries[i], m_entries[j]);
                if (d < mergeThreshold) {
                    // Merge j into i
                    m_entries[i].n += m_entries[j].n;
                    for (int k = 0; k < m_entries[i].ls.size(); ++k)
                        m_entries[i].ls[k] += m_entries[j].ls[k];
                    m_entries[i].ss += m_entries[j].ss;
                    m_entries.removeAt(j);
                    changed = true;
                    break;
                }
            }
            if (changed) break;
        }
    }
    m_stats.numClusters = m_entries.size();
}

/* ---- Rebalance tree ---- */

void BirchClustering9::rebalanceTree()
{
    // Rebuild all entries from raw points with current threshold
    QVector<QVector<double>> saved = m_rawPoints;
    m_entries.clear();
    m_rawPoints.clear();
    for (const auto& pt : saved)
        insertPoint(pt);
}

/* ---- Get cluster labels ---- */

QVector<int> BirchClustering9::getLabels() const
{
    int n = m_rawPoints.size();
    QVector<int> labels(n, -1);
    for (int i = 0; i < n; ++i) {
        labels[i] = findNearest(m_rawPoints[i]);
    }
    return labels;
}

/* ---- Reset ---- */

void BirchClustering9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_entries.clear();
    m_rawPoints.clear();
}
