/**
 * @file KMeans33.cpp
 * @brief KMeans33 实现
 *
 * 实现K均值聚类：均衡约束分配与最小簇大小强制实现公平划分的负载均衡聚类。
 */

#include "utils/cluster300/KMeans33.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

KMeans33::KMeans33(QObject *parent)
    : QObject(parent) {}

KMeans33::~KMeans33() = default;

/* ---- Configuration ---- */

void KMeans33::setK(int k) { m_k = qBound(2, k, 256); }
void KMeans33::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 10000); }
void KMeans33::setConvergenceThreshold(double tol) { m_tol = qBound(1e-12, tol, 1.0); }
void KMeans33::setMinClusterSize(int minSize) { m_minSize = qBound(1, minSize, 10000); }
void KMeans33::setBalanceRatio(double ratio) { m_balanceRatio = qBound(0.1, ratio, 10.0); }

/* ---- Squared Euclidean distance ---- */

double KMeans33::distSq(const QVector<double>& a, const QVector<double>& b) const
{
    double d = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}

/* ---- K-means++ initialization ---- */

void KMeans33::initialize(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    m_dims = data[0].size();
    m_clusters.clear();

    // First center: index 0
    QVector<int> centers;
    centers.append(0);

    for (int k = 1; k < m_k; ++k) {
        QVector<double> dists(n, 0.0);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = 1e300;
            for (int c : centers) {
                double d = distSq(data[i], data[c]);
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            totalDist += minD;
        }
        if (totalDist < 1e-300) break;

        // Weighted sampling
        double threshold = qrand() / static_cast<double>(RAND_MAX) * totalDist;
        double cumSum = 0.0;
        int next = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= threshold) { next = i; break; }
        }
        centers.append(next);
    }

    // Build initial clusters
    for (int c : centers) {
        Cluster clust;
        clust.centroid = data[c];
        clust.count = 0;
        clust.intraVariance = 0.0;
        m_clusters.append(clust);
    }
}

/* ---- Balanced assignment with min-size enforcement ---- */

QVector<int> KMeans33::balancedAssign(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int K = m_clusters.size();
    QVector<int> assign(n, 0);
    QVector<int> counts(K, 0);

    // Target balanced size per cluster
    int targetSize = qMax(m_minSize, n / K);
    double maxAllowed = targetSize * m_balanceRatio;

    // First pass: assign to nearest, respecting balance constraint
    for (int i = 0; i < n; ++i) {
        double bestDist = 1e300;
        int bestK = 0;

        // Sort clusters by distance
        QVector<QPair<double, int>> dists;
        for (int k = 0; k < K; ++k) {
            double d = distSq(data[i], m_clusters[k].centroid);
            dists.append({d, k});
        }
        std::sort(dists.begin(), dists.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        // Pick nearest cluster that hasn't exceeded capacity
        for (const auto& [d, k] : dists) {
            if (counts[k] < static_cast<int>(maxAllowed)) {
                bestK = k;
                bestDist = d;
                break;
            }
        }
        assign[i] = bestK;
        counts[bestK]++;
    }

    // Second pass: enforce minimum cluster size
    for (int k = 0; k < K; ++k) {
        while (counts[k] < m_minSize) {
            // Steal from the largest cluster
            int largest = 0;
            for (int j = 1; j < K; ++j)
                if (counts[j] > counts[largest]) largest = j;

            if (counts[largest] <= m_minSize) break;

            // Find closest point in largest to cluster k
            double minDist = 1e300;
            int victim = -1;
            for (int i = 0; i < n; ++i) {
                if (assign[i] == largest) {
                    double d = distSq(data[i], m_clusters[k].centroid);
                    if (d < minDist) { minDist = d; victim = i; }
                }
            }
            if (victim >= 0) {
                assign[victim] = k;
                counts[largest]--;
                counts[k]++;
            } else {
                break;
            }
        }
    }

    return assign;
}

/* ---- Update centroids ---- */

void KMeans33::updateCentroids(const QVector<QVector<double>>& data,
                                const QVector<int>& assignments)
{
    int K = m_clusters.size();
    int dim = m_dims;

    QVector<QVector<double>> sums(K, QVector<double>(dim, 0.0));
    QVector<int> counts(K, 0);

    for (int i = 0; i < data.size(); ++i) {
        int k = assignments[i];
        for (int d = 0; d < dim; ++d)
            sums[k][d] += data[i][d];
        counts[k]++;
    }

    for (int k = 0; k < K; ++k) {
        m_clusters[k].count = counts[k];
        if (counts[k] > 0) {
            for (int d = 0; d < dim; ++d)
                m_clusters[k].centroid[d] = sums[k][d] / counts[k];
        }
    }
}

/* ---- Compute inertia ---- */

double KMeans33::computeInertia(const QVector<QVector<double>>& data,
                                 const QVector<int>& assignments) const
{
    double inertia = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        int k = assignments[i];
        inertia += distSq(data[i], m_clusters[k].centroid);
    }
    return inertia;
}

/* ---- Main fit ---- */

KMeans33::FitResult KMeans33::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;
    int n = data.size();
    if (n < m_k) return result;

    initialize(data);

    double prevInertia = 1e300;
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        QVector<int> assign = balancedAssign(data);
        updateCentroids(data, assign);
        double inertia = computeInertia(data, assign);

        if (qAbs(prevInertia - inertia) < m_tol * qAbs(prevInertia))
            break;
        prevInertia = inertia;
    }

    // Final assignment
    result.assignments = balancedAssign(data);
    result.inertia = computeInertia(data, result.assignments);
    result.iterations = iter;
    result.balanced = true;

    // Compute per-cluster intra variance
    for (int i = 0; i < n; ++i) {
        int k = result.assignments[i];
        m_clusters[k].intraVariance += distSq(data[i], m_clusters[k].centroid);
    }
    for (auto& c : m_clusters) {
        if (c.count > 0) c.intraVariance /= c.count;
    }

    result.clusters = m_clusters;
    result.elapsedMs = timer.elapsed();

    m_stats.totalFits++;
    m_stats.maxIterations = qMax(m_stats.maxIterations, iter);
    m_inertiaSum += result.inertia;
    m_stats.avgInertia = m_inertiaSum / m_stats.totalFits;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitDone(m_k, result.inertia, iter, result.elapsedMs);
    return result;
}

/* ---- Predict ---- */

QVector<int> KMeans33::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size(), 0);
    if (m_clusters.isEmpty()) return labels;

    for (int i = 0; i < data.size(); ++i) {
        double bestDist = 1e300;
        for (int k = 0; k < m_clusters.size(); ++k) {
            double d = distSq(data[i], m_clusters[k].centroid);
            if (d < bestDist) { bestDist = d; labels[i] = k; }
        }
    }
    return labels;
}

/* ---- Reset ---- */

void KMeans33::resetStatistics()
{
    m_stats = Stats{};
    m_inertiaSum = 0.0;
    m_timeSum = 0.0;
    m_clusters.clear();
}
