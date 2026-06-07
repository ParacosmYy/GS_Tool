/**
 * @file BirchClustering7.cpp
 * @brief BirchClustering7 实现
 *
 * 实现BIRCH聚类：多探针CF树插入、子簇质量索引、内存有界增量聚类。
 */

#include "utils/cluster189/BirchClustering7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BirchClustering7::BirchClustering7(QObject *parent) : QObject(parent) {}
BirchClustering7::~BirchClustering7() = default;

/* ---- Configuration ---- */

void BirchClustering7::setBranchFactor(int b) { m_branchFactor = qMax(2, b); }
void BirchClustering7::setThreshold(double t) { m_threshold = qMax(1e-10, t); }
void BirchClustering7::setMaxMemory(int m) { m_maxMemory = qMax(100, m); }
void BirchClustering7::setDimensions(int d) { m_dimensions = qMax(1, d); }

/* ---- CF centroid ---- */

QVector<double> BirchClustering7::cfCentroid(const CFEntry& cf) const
{
    QVector<double> c(m_dimensions, 0.0);
    if (cf.n == 0) return c;
    for (int i = 0; i < m_dimensions; ++i)
        c[i] = cf.ls[i] / cf.n;
    return c;
}

/* ---- Distance between point and CF centroid ---- */

double BirchClustering7::cfDistance(const CFEntry& cf,
                                     const QVector<double>& point) const
{
    if (cf.n == 0) return std::numeric_limits<double>::max();
    double sum = 0.0;
    for (int i = 0; i < m_dimensions; ++i) {
        double diff = point[i] - cf.ls[i] / cf.n;
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Subcluster Quality Index ---- */

double BirchClustering7::computeSQI(const CFEntry& cf) const
{
    if (cf.n < 2) return 0.0;
    // SQI = sqrt((ss/n) - ||ls/n||^2), the radius of the subcluster
    double centroidNormSq = 0.0;
    for (int i = 0; i < m_dimensions; ++i) {
        double ci = cf.ls[i] / cf.n;
        centroidNormSq += ci * ci;
    }
    double radiusSq = (cf.ss / cf.n) - centroidNormSq;
    return qSqrt(qMax(0.0, radiusSq));
}

/* ---- Merge two CF entries ---- */

BirchClustering7::CFEntry BirchClustering7::mergeCF(const CFEntry& a,
                                                      const CFEntry& b) const
{
    CFEntry merged;
    merged.n = a.n + b.n;
    merged.ls.resize(m_dimensions, 0.0);
    for (int i = 0; i < m_dimensions; ++i)
        merged.ls[i] = a.ls[i] + b.ls[i];
    merged.ss = a.ss + b.ss;
    merged.sqi = computeSQI(merged);
    return merged;
}

/* ---- Multi-probe search: find closest leaf + probe neighbors ---- */

int BirchClustering7::multiProbeSearch(const QVector<double>& point) const
{
    if (m_leaves.isEmpty()) return -1;

    double bestDist = std::numeric_limits<double>::max();
    int bestIdx = 0;

    // Probe the closest and nearby entries within threshold*2
    for (int i = 0; i < m_leaves.size(); ++i) {
        double d = cfDistance(m_leaves[i], point);
        if (d < bestDist) {
            bestDist = d;
            bestIdx = i;
        }
    }

    // Multi-probe: check if merging into best improves SQI
    // If not, try second-best within threshold * 2
    double probeRadius = m_threshold * 2.0;
    for (int i = 0; i < m_leaves.size(); ++i) {
        if (i == bestIdx) continue;
        double d = cfDistance(m_leaves[i], point);
        if (d < probeRadius) {
            CFEntry merged = mergeCF(m_leaves[i], CFEntry{1, point,
                0.0, 0.0, -1});
            // Recompute SQI properly
            for (int j = 0; j < m_dimensions; ++j)
                merged.ss += point[j] * point[j];
            merged.sqi = computeSQI(merged);
            if (merged.sqi < m_leaves[bestIdx].sqi)
                bestIdx = i;
        }
    }
    return bestIdx;
}

/* ---- Rebuild tree: merge closest pairs until under memory bound ---- */

void BirchClustering7::rebuildTree()
{
    while (m_leaves.size() > m_maxMemory / 2) {
        double bestDist = std::numeric_limits<double>::max();
        int bestI = 0, bestJ = 1;

        for (int i = 0; i < m_leaves.size(); ++i) {
            for (int j = i + 1; j < m_leaves.size(); ++j) {
                auto ci = cfCentroid(m_leaves[i]);
                auto cj = cfCentroid(m_leaves[j]);
                double d = 0.0;
                for (int k = 0; k < m_dimensions; ++k) {
                    double diff = ci[k] - cj[k];
                    d += diff * diff;
                }
                d = qSqrt(d);
                if (d < bestDist) {
                    bestDist = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        m_leaves[bestI] = mergeCF(m_leaves[bestI], m_leaves[bestJ]);
        m_leaves.removeAt(bestJ);
        m_stats.rebuildCount++;
    }
}

/* ---- Insert a single point ---- */

void BirchClustering7::insertPoint(const QVector<double>& point)
{
    if (point.size() != m_dimensions) return;

    CFEntry pointCF;
    pointCF.n = 1;
    pointCF.ls = point;
    pointCF.ss = 0.0;
    for (int i = 0; i < m_dimensions; ++i)
        pointCF.ss += point[i] * point[i];

    if (m_leaves.isEmpty()) {
        pointCF.sqi = 0.0;
        m_leaves.append(pointCF);
        return;
    }

    int idx = multiProbeSearch(point);
    CFEntry candidate = mergeCF(m_leaves[idx], pointCF);
    candidate.sqi = computeSQI(candidate);

    if (candidate.sqi <= m_threshold) {
        m_leaves[idx] = candidate;
    } else {
        // Create new leaf entry
        pointCF.sqi = 0.0;
        m_leaves.append(pointCF);
    }

    // Memory-bounded rebuild
    if (m_leaves.size() > m_maxMemory)
        rebuildTree();
}

/* ---- Batch fit ---- */

QVector<int> BirchClustering7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_leaves.clear();
    m_centroids.clear();
    int N = data.size();
    if (N == 0) return {};

    // Determine dimensions from first point
    m_dimensions = data[0].size();

    // Phase 1: Build CF-tree incrementally
    for (int i = 0; i < N; ++i)
        insertPoint(data[i]);

    // Phase 2: Final clustering on leaf entries
    int k = qMax(1, m_leaves.size() / m_branchFactor);
    QVector<int> leafLabels = finalizeClustering(k);

    // Map original points to final cluster labels
    QVector<int> labels(N);
    for (int i = 0; i < N; ++i) {
        int closestLeaf = 0;
        double bestDist = std::numeric_limits<double>::max();
        for (int j = 0; j < m_leaves.size(); ++j) {
            double d = cfDistance(m_leaves[j], data[i]);
            if (d < bestDist) {
                bestDist = d;
                closestLeaf = j;
            }
        }
        labels[i] = leafLabels[closestLeaf];
    }

    // Compute centroids
    int maxLabel = *std::max_element(leafLabels.begin(), leafLabels.end());
    m_centroids.resize(maxLabel + 1);
    for (int c = 0; c <= maxLabel; ++c) {
        m_centroids[c].resize(m_dimensions, 0.0);
        auto cen = cfCentroid(m_leaves[0]);
        for (int j = 0; j < m_leaves.size(); ++j) {
            if (leafLabels[j] == c) {
                cen = cfCentroid(m_leaves[j]);
                break;
            }
        }
        m_centroids[c] = cen;
    }

    m_stats.totalRuns++;
    m_stats.numPoints = N;
    m_stats.numSubclusters = m_leaves.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(m_leaves.size(), m_stats.rebuildCount,
                             timer.elapsed());
    return labels;
}

/* ---- Final clustering via agglomerative merge ---- */

QVector<int> BirchClustering7::finalizeClustering(int k)
{
    int n = m_leaves.size();
    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) labels[i] = i;

    if (n <= k) return labels;

    // Merge closest pairs until k clusters remain
    QVector<CFEntry> entries = m_leaves;
    QVector<int> mapping(n);
    for (int i = 0; i < n; ++i) mapping[i] = i;

    while (n > k) {
        double bestDist = std::numeric_limits<double>::max();
        int bestI = 0, bestJ = 1;

        for (int i = 0; i < entries.size(); ++i) {
            if (entries[i].n == 0) continue;
            for (int j = i + 1; j < entries.size(); ++j) {
                if (entries[j].n == 0) continue;
                auto ci = cfCentroid(entries[i]);
                auto cj = cfCentroid(entries[j]);
                double d = 0.0;
                for (int dim = 0; dim < m_dimensions; ++dim) {
                    double diff = ci[dim] - cj[dim];
                    d += diff * diff;
                }
                if (d < bestDist) {
                    bestDist = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        entries[bestI] = mergeCF(entries[bestI], entries[bestJ]);
        entries[bestJ].n = 0;
        n--;
    }

    // Assign final labels
    int label = 0;
    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].n == 0) continue;
        for (int j = 0; j < m_leaves.size(); ++j) {
            if (mapping[j] == i || mapping[j] == (entries[i].n > 0 ? i : -1))
                labels[j] = label;
        }
        label++;
    }

    // Simplified: reassign based on closest centroid
    for (int i = 0; i < m_leaves.size(); ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int c = 0; c < label; ++c) {
            if (entries[c].n == 0) continue;
            double d = cfDistance(entries[c], cfCentroid(m_leaves[i]));
            if (d < bestDist) {
                bestDist = d;
                labels[i] = c;
            }
        }
    }

    return labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> BirchClustering7::subclusterCentroids() const
{
    QVector<QVector<double>> centroids;
    for (const auto& leaf : m_leaves)
        centroids.append(cfCentroid(leaf));
    return centroids;
}

QVector<BirchClustering7::CFEntry> BirchClustering7::leafEntries() const
{
    return m_leaves;
}

/* ---- Reset ---- */

void BirchClustering7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_leaves.clear();
    m_centroids.clear();
}
