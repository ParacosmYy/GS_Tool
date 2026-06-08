/**
 * @file KMeans23.cpp
 * @brief KMeans23 实现
 *
 * 实现K均值聚类：Canopy预聚类初始化与三角不等式加速。
 */

#include "utils/cluster230/KMeans23.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMeans23::KMeans23(QObject *parent) : QObject(parent) {}
KMeans23::~KMeans23() = default;

/* ---- Configuration ---- */

void KMeans23::setParameters(int k, double canopyT1, double canopyT2)
{
    m_k = qMax(2, k);
    m_canopyT1 = canopyT1;
    m_canopyT2 = canopyT2;
}

/* ---- Squared Euclidean distance ---- */

double KMeans23::squaredDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- Canopy pre-clustering ---- */

QVector<int> KMeans23::canopyPrecluster(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n == 0) return {};

    // Auto-estimate thresholds from data if not set
    double t1 = m_canopyT1;
    double t2 = m_canopyT2;
    if (t1 <= 0.0 || t2 <= 0.0) {
        // Use mean nearest-neighbor distance
        double meanDist = 0.0;
        int samples = qMin(n, 200);
        for (int i = 0; i < samples; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int j = 0; j < samples; ++j) {
                if (i == j) continue;
                double d = squaredDist(data[i], data[j]);
                minD = qMin(minD, d);
            }
            meanDist += qSqrt(minD);
        }
        meanDist /= samples;
        t2 = meanDist * 0.5;
        t1 = meanDist * 1.5;
    }

    QVector<bool> covered(n, false);
    QVector<int> canopyCenters;

    for (int i = 0; i < n; ++i) {
        if (covered[i]) continue;
        canopyCenters.append(i);
        double t2sq = t2 * t2;
        double t1sq = t1 * t1;
        for (int j = 0; j < n; ++j) {
            if (covered[j]) continue;
            double d = squaredDist(data[i], data[j]);
            if (d < t1sq) covered[j] = true;
        }
        // Strong threshold: mark points within t2 as part of this canopy
        for (int j = 0; j < n; ++j) {
            if (squaredDist(data[i], data[j]) < t2sq) covered[j] = true;
        }
    }
    return canopyCenters;
}

/* ---- Initialize centroids from canopies ---- */

void KMeans23::initCentroids(const QVector<QVector<double>>& data, const QVector<int>& canopies)
{
    m_centroids.clear();
    m_centroids.resize(m_k);
    for (int c = 0; c < m_k; ++c)
        m_centroids[c].resize(m_d, 0.0);

    if (canopies.size() >= m_k) {
        // Pick k evenly spaced canopy centers
        int step = qMax(1, canopies.size() / m_k);
        for (int c = 0; c < m_k; ++c) {
            int idx = canopies[qMin(c * step, canopies.size() - 1)];
            m_centroids[c] = data[idx];
        }
    } else {
        // Fallback: random initialization
        int n = data.size();
        for (int c = 0; c < m_k; ++c) {
            int idx = (c * n) / m_k;
            m_centroids[c] = data[qMin(idx, n - 1)];
        }
    }
}

/* ---- Inter-cluster distances ---- */

void KMeans23::computeClusterDistances()
{
    int kk = m_k * m_k;
    m_clusterDist.resize(kk);
    for (int i = 0; i < m_k; ++i) {
        for (int j = 0; j < m_k; ++j) {
            if (i == j) {
                m_clusterDist[i * m_k + j] = 0.0;
            } else {
                m_clusterDist[i * m_k + j] = qSqrt(squaredDist(m_centroids[i], m_centroids[j]));
            }
        }
    }
}

/* ---- Lloyd step with triangle inequality ---- */

int KMeans23::lloydStep(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int changes = 0;
    int saved = 0;

    computeClusterDistances();

    // Half inter-cluster distance for each cluster (for triangle inequality test)
    QVector<double> halfDist(m_k);
    for (int c = 0; c < m_k; ++c) {
        double minOther = std::numeric_limits<double>::max();
        for (int j = 0; j < m_k; ++j) {
            if (j == c) continue;
            minOther = qMin(minOther, m_clusterDist[c * m_k + j]);
        }
        halfDist[c] = minOther * 0.5;
    }

    for (int i = 0; i < n; ++i) {
        int curCluster = m_assignments[i].clusterId;
        double curDist = m_assignments[i].distance;

        // Triangle inequality: skip if halfDist to another cluster >= current distance
        if (curCluster >= 0 && curDist <= halfDist[curCluster]) {
            saved++;
            continue;
        }

        // Full distance computation needed
        double bestDist = std::numeric_limits<double>::max();
        int bestCluster = 0;
        for (int c = 0; c < m_k; ++c) {
            double d = squaredDist(data[i], m_centroids[c]);
            if (d < bestDist) {
                bestDist = d;
                bestCluster = c;
            }
        }

        if (bestCluster != curCluster) {
            m_assignments[i].clusterId = bestCluster;
            m_assignments[i].distance = bestDist;
            changes++;
        } else {
            m_assignments[i].distance = bestDist;
        }
    }

    // Update centroids
    QVector<int> counts(m_k, 0);
    for (int c = 0; c < m_k; ++c)
        m_centroids[c].fill(0.0, m_d);

    for (int i = 0; i < n; ++i) {
        int c = m_assignments[i].clusterId;
        if (c < 0) continue;
        counts[c]++;
        for (int j = 0; j < m_d; ++j)
            m_centroids[c][j] += data[i][j];
    }

    m_centerShift.resize(m_k);
    for (int c = 0; c < m_k; ++c) {
        QVector<double> oldCenter = m_centroids[c];
        if (counts[c] > 0) {
            for (int j = 0; j < m_d; ++j)
                m_centroids[c][j] /= counts[c];
        }
        m_centerShift[c] = squaredDist(oldCenter, m_centroids[c]);
    }

    m_stats.numDistSaved += saved;
    return changes;
}

/* ---- Fit ---- */

bool KMeans23::fit(const QVector<QVector<double>>& data, int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_k) return false;
    m_d = data[0].size();

    // Canopy pre-clustering
    QVector<int> canopies = canopyPrecluster(data);
    initCentroids(data, canopies);

    m_assignments.resize(n);
    for (int i = 0; i < n; ++i) {
        m_assignments[i].clusterId = -1;
        m_assignments[i].distance = std::numeric_limits<double>::max();
    }

    m_stats.numDistSaved = 0;
    double prevInertia = std::numeric_limits<double>::max();

    for (int iter = 0; iter < maxIter; ++iter) {
        int changes = lloydStep(data);

        // Compute inertia
        double inertia = 0.0;
        for (int i = 0; i < n; ++i)
            inertia += m_assignments[i].distance;

        emit iterationCompleted(iter, inertia);

        if (qAbs(prevInertia - inertia) < tol) break;
        prevInertia = inertia;

        m_stats.numIterations = iter + 1;
    }

    // Final inertia
    m_stats.inertia = 0.0;
    for (int i = 0; i < n; ++i)
        m_stats.inertia += m_assignments[i].distance;

    m_stats.numClusters = m_k;
    m_stats.numDimensions = m_d;
    m_stats.numPoints = n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_k, m_stats.inertia, timer.elapsed());
    return true;
}

/* ---- Predict ---- */

int KMeans23::predict(const QVector<double>& point) const
{
    if (m_centroids.isEmpty()) return -1;
    double bestDist = std::numeric_limits<double>::max();
    int best = 0;
    for (int c = 0; c < m_k; ++c) {
        double d = squaredDist(point, m_centroids[c]);
        if (d < bestDist) { bestDist = d; best = c; }
    }
    return best;
}

/* ---- Assignments ---- */

QVector<KMeans23::Assignment> KMeans23::assignments() const { return m_assignments; }

/* ---- Centroids ---- */

QVector<QVector<double>> KMeans23::centroids() const { return m_centroids; }

/* ---- Reset ---- */

void KMeans23::resetStatistics()
{
    m_centroids.clear();
    m_assignments.clear();
    m_lowerBounds.clear();
    m_upperBound.clear();
    m_clusterDist.clear();
    m_centerShift.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
