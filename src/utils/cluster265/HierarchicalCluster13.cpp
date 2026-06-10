/**
 * @file HierarchicalCluster13.cpp
 * @brief HierarchicalCluster13 实现
 *
 * 实现层次聚类：Ward最小方差法与Lance-Williams递推凝聚合并聚合聚类。
 */

#include "utils/cluster265/HierarchicalCluster13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HierarchicalCluster13::HierarchicalCluster13(QObject *parent)
    : QObject(parent) {}

HierarchicalCluster13::~HierarchicalCluster13() = default;

/* ---- Configuration ---- */

void HierarchicalCluster13::setLinkage(int method)
{
    m_linkage = qBound(0, method, 3);
}

/* ---- Distance computation ---- */

double HierarchicalCluster13::sqEuclidean(const QVector<double>& a,
                                           const QVector<double>& b) const
{
    double sum = 0.0;
    for (int i = 0; i < a.size() && i < b.size(); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

/* ---- Lance-Williams recurrence ---- */

double HierarchicalCluster13::lanceWilliams(double dij, double dik,
                                             double djk,
                                             int si, int sj, int sk) const
{
    Q_UNUSED(djk)
    int total = si + sj + sk;
    if (total == 0) return 0.0;

    switch (m_linkage) {
    case 0: {
        // Ward's method: minimize total within-cluster variance
        double ai = static_cast<double>(si + sk) / total;
        double aj = static_cast<double>(sj + sk) / total;
        double beta = static_cast<double>(-sk) / total;
        return qSqrt(ai * dij * dij + aj * dik * dik + beta * djk * djk);
    }
    case 1: // Single linkage: minimum distance
        return qMin(dij, dik);
    case 2: // Complete linkage: maximum distance
        return qMax(dij, dik);
    case 3: { // Average linkage: weighted average
        double w = static_cast<double>(sk) / total;
        return dij * (1.0 - w) + dik * w;
    }
    default:
        return dij;
    }
}

/* ---- Find minimum in condensed distance matrix ---- */

int HierarchicalCluster13::findMinIndex(const QVector<double>& dist,
                                         int activeCount,
                                         const QVector<int>& activeMap) const
{
    double best = std::numeric_limits<double>::max();
    int idx = -1;
    for (int i = 0; i < activeCount; ++i) {
        for (int j = i + 1; j < activeCount; ++j) {
            int ci = activeMap[i];
            int cj = activeMap[j];
            int lo = qMin(ci, cj);
            int hi = qMax(ci, cj);
            int k = lo * m_n - lo * (lo + 1) / 2 + hi - lo - 1;
            if (k >= 0 && k < dist.size() && dist[k] < best) {
                best = dist[k];
                idx = k;
            }
        }
    }
    return idx;
}

/* ---- Full agglomerative clustering ---- */

QVector<HierarchicalCluster13::MergeStep> HierarchicalCluster13::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n < 2) return {};

    m_dim = data[0].size();
    m_merges.clear();

    // Cluster sizes (initially each cluster has 1 point)
    QVector<int> clusterSize(m_n, 1);
    // Active cluster flags
    QVector<bool> active(m_n, true);

    // Build condensed distance matrix (upper triangular)
    int dmSize = m_n * (m_n - 1) / 2;
    QVector<double> dist(dmSize);
    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            dist[i * m_n - i * (i + 1) / 2 + j - i - 1] =
                qSqrt(sqEuclidean(data[i], data[j]));
        }
    }

    int numActive = m_n;

    for (int step = 0; step < m_n - 1; ++step) {
        // Find pair with minimum distance among active clusters
        double minDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;

        for (int i = 0; i < m_n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < m_n; ++j) {
                if (!active[j]) continue;
                int k = i * m_n - i * (i + 1) / 2 + j - i - 1;
                if (dist[k] < minDist) {
                    minDist = dist[k];
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI < 0) break;

        // Record merge step
        MergeStep merge;
        merge.clusterA = bestI;
        merge.clusterB = bestJ;
        merge.distance = minDist;
        merge.newSize = clusterSize[bestI] + clusterSize[bestJ];
        m_merges.append(merge);

        // Update distances using Lance-Williams recurrence
        for (int k = 0; k < m_n; ++k) {
            if (!active[k] || k == bestI || k == bestJ) continue;

            int idxIK = qMin(bestI, k) * m_n - qMin(bestI, k) * (qMin(bestI, k) + 1) / 2
                        + qMax(bestI, k) - qMin(bestI, k) - 1;
            int idxJK = qMin(bestJ, k) * m_n - qMin(bestJ, k) * (qMin(bestJ, k) + 1) / 2
                        + qMax(bestJ, k) - qMin(bestJ, k) - 1;

            double dik = (idxIK >= 0 && idxIK < dist.size()) ? dist[idxIK] : 0.0;
            double djk = (idxJK >= 0 && idxJK < dist.size()) ? dist[idxJK] : 0.0;

            double newDist = lanceWilliams(minDist, dik, djk,
                                           clusterSize[bestI],
                                           clusterSize[bestJ],
                                           clusterSize[k]);

            // Store updated distance in bestI's slot
            int newIdx = qMin(bestI, k) * m_n - qMin(bestI, k) * (qMin(bestI, k) + 1) / 2
                         + qMax(bestI, k) - qMin(bestI, k) - 1;
            if (newIdx >= 0 && newIdx < dist.size())
                dist[newIdx] = newDist;
        }

        // Merge bestJ into bestI
        clusterSize[bestI] += clusterSize[bestJ];
        active[bestJ] = false;
        numActive--;
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = m_n;
    m_stats.dimension = m_dim;
    m_stats.numMerges = m_merges.size();
    m_stats.finalDistance = m_merges.isEmpty() ? 0.0 : m_merges.last().distance;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringUpdated(numActive, m_stats.finalDistance, elapsed);

    return m_merges;
}

/* ---- Cut dendrogram at threshold ---- */

QVector<int> HierarchicalCluster13::cutDendrogram(double threshold) const
{
    m_labels.resize(m_n);
    // Initialize each point as its own cluster
    for (int i = 0; i < m_n; ++i)
        m_labels[i] = i;

    // Apply merges up to threshold
    for (const auto& merge : m_merges) {
        if (merge.distance > threshold) break;
        int target = merge.clusterA;
        int source = merge.clusterB;
        for (int i = 0; i < m_n; ++i) {
            if (m_labels[i] == source) m_labels[i] = target;
        }
    }

    // Relabel to 0..K-1
    QVector<int> mapping(m_n, -1);
    int nextLabel = 0;
    for (int i = 0; i < m_n; ++i) {
        if (mapping[m_labels[i]] < 0)
            mapping[m_labels[i]] = nextLabel++;
        m_labels[i] = mapping[m_labels[i]];
    }
    return m_labels;
}

/* ---- Cut to exactly k clusters ---- */

QVector<int> HierarchicalCluster13::cutToKClusters(int k) const
{
    if (m_merges.isEmpty()) return QVector<int>(m_n, 0);
    int cutStep = qMax(0, m_n - k);
    double threshold = (cutStep < m_merges.size())
                           ? m_merges[cutStep].distance : 1e18;
    return cutDendrogram(threshold);
}

/* ---- Accessors ---- */

QVector<HierarchicalCluster13::MergeStep> HierarchicalCluster13::mergeHistory() const
{
    return m_merges;
}

/* ---- Reset ---- */

void HierarchicalCluster13::resetStatistics()
{
    m_merges.clear();
    m_labels.clear();
    m_n = 0;
    m_dim = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
