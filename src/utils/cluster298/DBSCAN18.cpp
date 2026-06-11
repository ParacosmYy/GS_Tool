/**
 * @file DBSCAN18.cpp
 * @brief DBSCAN18 实现
 *
 * 实现DBSCAN密度聚类：局部离群因子集成与自适应minPts实现密度比噪声校准聚类。
 */

#include "utils/cluster298/DBSCAN18.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DBSCAN18::DBSCAN18(QObject *parent)
    : QObject(parent) {}

DBSCAN18::~DBSCAN18() = default;

/* ---- Configuration ---- */

void DBSCAN18::setEpsilon(double eps) { m_eps = qBound(1e-6, eps, 1e6); }
void DBSCAN18::setBaseMinPts(int minPts) { m_baseMinPts = qBound(2, minPts, 1024); }
void DBSCAN18::setLofThreshold(double threshold) { m_lofThreshold = qBound(1.0, threshold, 100.0); }

/* ---- Euclidean distance ---- */

double DBSCAN18::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(qMax(0.0, sum));
}

/* ---- Region query: find neighbors within epsilon ---- */

QVector<int> DBSCAN18::regionQuery(const QVector<QVector<double>>& data, int idx) const
{
    QVector<int> neighbors;
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        if (distance(data[idx], data[i]) <= m_eps)
            neighbors.append(i);
    }
    return neighbors;
}

/* ---- Local reachability density ---- */

double DBSCAN18::localReachDensity(const QVector<QVector<double>>& data,
                                    int idx,
                                    const QVector<QVector<int>>& neighborhoods) const
{
    const auto& neighbors = neighborhoods[idx];
    if (neighbors.isEmpty()) return 0.0;

    double reachDist = 0.0;
    for (int nIdx : neighbors) {
        double dist = distance(data[idx], data[nIdx]);
        // Reachability distance = max(core-dist, actual dist)
        double coreDist = 0.0;
        const auto& nNeighbors = neighborhoods[nIdx];
        if (nNeighbors.size() >= m_baseMinPts) {
            // k-distance is the distance to the minPts-th neighbor
            double kDist = 1e300;
            for (int nn : nNeighbors) {
                double d = distance(data[nIdx], data[nn]);
                if (d < kDist) kDist = d;
            }
            coreDist = kDist;
        }
        reachDist += qMax(coreDist, dist);
    }

    // LRD = 1 / average reachability distance
    double avgReach = reachDist / neighbors.size();
    return (avgReach > 1e-300) ? 1.0 / avgReach : 1e300;
}

/* ---- Adaptive minPts based on local density ---- */

int DBSCAN18::adaptiveMinPts(int idx, const QVector<double>& densities) const
{
    int n = densities.size();
    if (n == 0) return m_baseMinPts;

    // Compute median density
    QVector<double> sorted = densities;
    std::sort(sorted.begin(), sorted.end());
    double median = sorted[n / 2];

    // Adjust minPts: higher density -> lower minPts, lower density -> higher minPts
    double ratio = (median > 1e-300) ? densities[idx] / median : 1.0;
    int adapted = static_cast<int>(m_baseMinPts / qMax(0.1, ratio));
    return qBound(2, adapted, m_baseMinPts * 4);
}

/* ---- Expand cluster from seed ---- */

void DBSCAN18::expandCluster(const QVector<QVector<double>>& data,
                              int pointIdx, int clusterId,
                              QVector<int>& labels,
                              const QVector<QVector<int>>& neighborhoods,
                              const QVector<int>& adaptedMinPts)
{
    QVector<int> seeds = neighborhoods[pointIdx];
    seeds.append(pointIdx);

    int qi = 0;
    while (qi < seeds.size()) {
        int current = seeds[qi++];
        if (labels[current] == 0) {
            // Was noise, now part of cluster
            labels[current] = clusterId;
        }
        if (labels[current] != -1 && labels[current] != clusterId) continue;
        labels[current] = clusterId;

        int minPts = adaptedMinPts[current];
        if (neighborhoods[current].size() >= minPts) {
            // Core point: add its neighbors to seeds
            for (int nIdx : neighborhoods[current]) {
                if (labels[nIdx] == -1 || labels[nIdx] == 0) {
                    seeds.append(nIdx);
                }
            }
        }
    }
}

/* ---- Compute LOF scores ---- */

QVector<double> DBSCAN18::computeLOF(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<double> lofScores(n, 1.0);

    // Precompute all neighborhoods
    QVector<QVector<int>> neighborhoods(n);
    for (int i = 0; i < n; ++i)
        neighborhoods[i] = regionQuery(data, i);

    // Compute local reachability densities
    QVector<double> lrds(n);
    for (int i = 0; i < n; ++i)
        lrds[i] = localReachDensity(data, i, neighborhoods);

    // Compute LOF = average ratio of neighbor LRDs to own LRD
    for (int i = 0; i < n; ++i) {
        if (lrds[i] < 1e-300 || neighborhoods[i].isEmpty()) {
            lofScores[i] = 1e6;  // outlier
            continue;
        }
        double lrdRatio = 0.0;
        for (int nIdx : neighborhoods[i])
            lrdRatio += lrds[nIdx] / lrds[i];
        lofScores[i] = lrdRatio / neighborhoods[i].size();
    }

    return lofScores;
}

/* ---- Main fit with LOF noise calibration and adaptive minPts ---- */

DBSCAN18::ClusterResult DBSCAN18::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0) return result;

    m_dims = data[0].size();
    result.labels.resize(n, -1);  // -1 = unvisited
    result.lofScores.resize(n, 1.0);

    // Step 1: Precompute all neighborhoods
    QVector<QVector<int>> neighborhoods(n);
    for (int i = 0; i < n; ++i)
        neighborhoods[i] = regionQuery(data, i);

    // Step 2: Compute local densities for adaptive minPts
    QVector<double> densities(n);
    for (int i = 0; i < n; ++i)
        densities[i] = static_cast<double>(neighborhoods[i].size());

    // Step 3: Compute adaptive minPts per point
    QVector<int> adaptedMinPts(n);
    for (int i = 0; i < n; ++i)
        adaptedMinPts[i] = adaptiveMinPts(i, densities);

    // Step 4: Main DBSCAN loop with adaptive minPts
    int clusterId = 0;
    for (int i = 0; i < n; ++i) {
        if (result.labels[i] != -1) continue;  // already processed

        if (neighborhoods[i].size() < adaptedMinPts[i]) {
            result.labels[i] = 0;  // noise (may be re-latered)
            continue;
        }

        clusterId++;
        expandCluster(data, i, clusterId, result.labels, neighborhoods, adaptedMinPts);
    }

    // Step 5: LOF-based noise calibration
    result.lofScores = computeLOF(data);

    // Re-evaluate noise points: if LOF is low, they might be border points
    for (int i = 0; i < n; ++i) {
        if (result.labels[i] == 0 && result.lofScores[i] < m_lofThreshold) {
            // Find nearest cluster neighbor
            double minDist = 1e300;
            int nearestCluster = -1;
            for (int nIdx : neighborhoods[i]) {
                if (result.labels[nIdx] > 0) {
                    double d = distance(data[i], data[nIdx]);
                    if (d < minDist) { minDist = d; nearestCluster = result.labels[nIdx]; }
                }
            }
            if (nearestCluster > 0)
                result.labels[i] = nearestCluster;
        }
    }

    result.numClusters = clusterId;
    result.numNoise = 0;
    for (int i = 0; i < n; ++i)
        if (result.labels[i] == 0) result.numNoise++;

    // Compute approximate silhouette
    double silSum = 0.0;
    int silCount = 0;
    for (int i = 0; i < n; ++i) {
        if (result.labels[i] <= 0) continue;
        double aDist = 0.0, bDist = 1e300;
        int aCount = 0;
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double d = distance(data[i], data[j]);
            if (result.labels[j] == result.labels[i]) { aDist += d; aCount++; }
            else if (result.labels[j] > 0) { bDist = qMin(bDist, d); }
        }
        if (aCount > 0) aDist /= aCount;
        if (bDist < 1e300 && (aDist + bDist) > 1e-300) {
            silSum += (bDist - aDist) / qMax(aDist, bDist);
            silCount++;
        }
    }
    result.avgSilhouette = (silCount > 0) ? silSum / silCount : 0.0;

    m_stats.totalFits++;
    m_stats.numPoints = n;
    m_stats.dimensions = m_dims;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    double avgLof = 0.0;
    for (double l : result.lofScores) avgLof += l;
    avgLof /= n;

    emit fitDone(result.numClusters, result.numNoise, avgLof, elapsed);
    return result;
}

/* ---- Reset ---- */

void DBSCAN18::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
