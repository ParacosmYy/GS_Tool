/**
 * @file DBSCAN14.cpp
 * @brief DBSCAN14 实现
 *
 * 实现DBSCAN密度聚类：k距离肘部估计与变epsilon密度自适应邻域。
 */

#include "utils/cluster242/DBSCAN14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DBSCAN14::DBSCAN14(QObject *parent) : QObject(parent) {}
DBSCAN14::~DBSCAN14() = default;

/* ---- Configuration ---- */

void DBSCAN14::setMinPoints(int minPts) { m_minPts = qMax(2, minPts); }
void DBSCAN14::setEpsilon(double eps) { m_eps = qMax(0.0, eps); }
void DBSCAN14::setElbowK(int k) { m_elbowK = qMax(2, k); }
void DBSCAN14::setVariableEpsilon(bool enabled) { m_variableEps = enabled; }

/* ---- Squared Euclidean distance ---- */

double DBSCAN14::sqDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- Range query: find all neighbors within eps radius ---- */

QVector<int> DBSCAN14::rangeQuery(int idx, double eps) const
{
    QVector<int> neighbors;
    double epsSq = eps * eps;
    int n = m_data.size();
    for (int i = 0; i < n; ++i) {
        if (sqDist(m_data[idx], m_data[i]) <= epsSq)
            neighbors.append(i);
    }
    return neighbors;
}

/* ---- k-distance for a single point ---- */

double DBSCAN14::kDistance(int idx, int k) const
{
    int n = m_data.size();
    if (n <= k) return 0.0;

    // Collect all distances from point idx
    QVector<double> dists;
    dists.reserve(n);
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        dists.append(qSqrt(sqDist(m_data[idx], m_data[i])));
    }
    // Partial sort to find k-th smallest
    std::nth_element(dists.begin(), dists.begin() + k - 1, dists.end());
    return dists[k - 1];
}

/* ---- Local density-adaptive epsilon ---- */

double DBSCAN14::localEpsilon(const QVector<int>& region) const
{
    if (region.isEmpty()) return m_eps;
    // Compute mean pairwise distance in the region as local density indicator
    double sumDist = 0.0;
    int count = 0;
    int limit = qMin(region.size(), 50);  // Sample for performance
    for (int i = 0; i < limit; ++i) {
        for (int j = i + 1; j < limit; ++j) {
            sumDist += qSqrt(sqDist(m_data[region[i]], m_data[region[j]]));
            count++;
        }
    }
    if (count == 0) return m_eps;
    // Adaptive: use mean distance * scaling factor
    return (sumDist / count) * 1.5;
}

/* ---- Estimate epsilon via k-distance elbow method ---- */

double DBSCAN14::estimateEpsilon(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n < m_elbowK + 1) return 1.0;

    m_data = data;  // Temporarily store for kDistance
    int k = qMin(m_elbowK, m_minPts);

    // Compute k-distance for every point
    QVector<double> kDists;
    kDists.reserve(n);
    for (int i = 0; i < n; ++i)
        kDists.append(kDistance(i, k));

    // Sort ascending
    std::sort(kDists.begin(), kDists.end());

    // Find elbow: maximum second derivative approximation
    // Use the point of maximum curvature in the sorted k-distance plot
    double maxCurvature = 0.0;
    int elbowIdx = n / 2;
    for (int i = 1; i < n - 1; ++i) {
        double d2 = kDists[i - 1] + kDists[i + 1] - 2.0 * kDists[i];
        if (d2 > maxCurvature) {
            maxCurvature = d2;
            elbowIdx = i;
        }
    }

    return kDists[elbowIdx];
}

/* ---- Fit: run DBSCAN ---- */

QVector<int> DBSCAN14::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n == 0) return m_labels;

    m_labels.resize(n);
    m_labels.fill(-1);  // -1 = unvisited

    // Estimate epsilon if not set
    double eps = m_eps;
    if (eps <= 0.0) {
        eps = estimateEpsilon(data);
        m_stats.estimatedEpsilon = eps;
    }

    int clusterId = 0;
    int numCore = 0, numBorder = 0, numNoise = 0;
    m_clusterEps.clear();

    for (int i = 0; i < n; ++i) {
        if (m_labels[i] >= 0) continue;  // Already assigned

        // Find neighbors
        double pointEps = eps;
        QVector<int> neighbors = rangeQuery(i, pointEps);

        if (neighbors.size() < m_minPts) {
            m_labels[i] = -2;  // Noise (temporarily)
            numNoise++;
            continue;
        }

        // Core point: start expanding cluster
        numCore++;
        m_labels[i] = clusterId;

        // Density-adaptive epsilon for this cluster
        double clusterEps = eps;
        if (m_variableEps) {
            clusterEps = localEpsilon(neighbors);
            if (clusterEps < eps * 0.3) clusterEps = eps * 0.3;
            if (clusterEps > eps * 3.0) clusterEps = eps * 3.0;
        }
        m_clusterEps.append(clusterEps);

        // Expand cluster using seed set
        QVector<int> seeds = neighbors;
        int seedIdx = 0;
        while (seedIdx < seeds.size()) {
            int q = seeds[seedIdx++];
            if (m_labels[q] == -2) {
                // Was noise, now border point
                m_labels[q] = clusterId;
                numBorder++;
                numNoise--;
            }
            if (m_labels[q] >= 0) continue;  // Already assigned to a cluster
            m_labels[q] = clusterId;

            QVector<int> qNeighbors = rangeQuery(q, clusterEps);
            if (qNeighbors.size() >= m_minPts) {
                // Merge new neighbors into seed set
                for (int nn : qNeighbors) {
                    if (m_labels[nn] < 0) seeds.append(nn);
                }
            }
        }

        clusterId++;
    }

    m_stats.numClusters = clusterId;
    m_stats.numCorePoints = numCore;
    m_stats.numBorderPoints = numBorder;
    m_stats.numNoisePoints = numNoise;
    m_stats.numSamples = n;
    m_stats.numDimensions = n > 0 ? data[0].size() : 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(clusterId, numNoise, timer.elapsed());
    return m_labels;
}

/* ---- Accessors ---- */

QVector<int> DBSCAN14::labels() const { return m_labels; }
QVector<double> DBSCAN14::clusterEpsilons() const { return m_clusterEps; }

/* ---- Reset ---- */

void DBSCAN14::resetStatistics()
{
    m_data.clear(); m_labels.clear(); m_clusterEps.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
