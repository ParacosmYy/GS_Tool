/**
 * @file KMeansClusterer.cpp
 * @brief K-Means聚类器实现
 */

#include "utils/cluster/KMeansClusterer.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

KMeansClusterer::KMeansClusterer(QObject* parent)
    : QObject(parent), m_k(3), m_maxIterations(100),
      m_tolerance(1e-6), m_timeSum(0.0) {}

void KMeansClusterer::setK(int k) { m_k = qMax(1, k); }
void KMeansClusterer::setMaxIterations(int maxIter) { m_maxIterations = qMax(1, maxIter); }
void KMeansClusterer::setTolerance(double tol) { m_tolerance = qMax(1e-12, tol); }

QList<KMeansClusterer::Cluster> KMeansClusterer::cluster(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<Cluster> result;
    if (data.isEmpty() || m_k <= 0) return result;

    int n = data.size();
    int k = qMin(m_k, n);

    /* K-Means++ 初始化 */
    QVector<double> centroids;
    std::mt19937 rng(42);
    centroids.append(data[rng() % n]);

    for (int c = 1; c < k; ++c) {
        QVector<double> dists;
        dists.reserve(n);
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (double cen : centroids) {
                double d = (data[i] - cen) * (data[i] - cen);
                if (d < minD) minD = d;
            }
            dists.append(minD);
        }
        double totalDist = 0.0;
        for (double d : dists) totalDist += d;
        if (totalDist < 1e-15) break;

        double r = static_cast<double>(rng()) / rng.max() * totalDist;
        double cumulative = 0.0;
        for (int i = 0; i < n; ++i) {
            cumulative += dists[i];
            if (cumulative >= r) { centroids.append(data[i]); break; }
        }
    }

    k = centroids.size();
    QVector<int> assignments(n, 0);

    /* 迭代优化 */
    int iter = 0;
    for (; iter < m_maxIterations; ++iter) {
        bool changed = false;

        /* 分配步骤 */
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = qAbs(data[i] - centroids[c]);
                if (d < minD) { minD = d; bestC = c; }
            }
            if (assignments[i] != bestC) { assignments[i] = bestC; changed = true; }
        }

        if (!changed) break;

        /* 更新步骤 */
        for (int c = 0; c < k; ++c) {
            double sum = 0.0;
            int count = 0;
            for (int i = 0; i < n; ++i) {
                if (assignments[i] == c) { sum += data[i]; ++count; }
            }
            if (count > 0) centroids[c] = sum / count;
        }

        emit iterationStep(iter, computeInertia(data, buildClusters(data, assignments, centroids)));
    }

    result = buildClusters(data, assignments, centroids);
    ++m_stats.totalClusterings;
    m_stats.totalIterations += iter;
    m_stats.avgIterations = static_cast<double>(m_stats.totalIterations) / m_stats.totalClusterings;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringComplete(k, iter);
    return result;
}

int KMeansClusterer::findOptimalK(const QVector<double>& data, int maxK) const
{
    if (data.isEmpty()) return 1;
    maxK = qMin(maxK, data.size());
    if (maxK <= 1) return 1;

    QVector<double> inertias;
    for (int k = 1; k <= maxK; ++k) {
        const_cast<KMeansClusterer*>(this)->m_k = k;
        QList<Cluster> clusters = const_cast<KMeansClusterer*>(this)->cluster(data);
        inertias.append(computeInertia(data, clusters));
    }

    /* 肘部法则: 找到曲率最大的K */
    int bestK = 1;
    double maxCurvature = 0.0;
    for (int i = 1; i < inertias.size() - 1; ++i) {
        double curvature = qAbs(inertias[i - 1] - 2 * inertias[i] + inertias[i + 1]);
        if (curvature > maxCurvature) { maxCurvature = curvature; bestK = i + 1; }
    }
    return bestK;
}

double KMeansClusterer::silhouetteScore(const QVector<double>& data,
                                         const QList<Cluster>& clusters) const
{
    if (clusters.size() < 2 || data.isEmpty()) return 0.0;
    int n = data.size();
    QVector<int> assignments(n, -1);

    for (int c = 0; c < clusters.size(); ++c) {
        for (int idx : clusters[c].memberIndices) assignments[idx] = c;
    }

    double totalScore = 0.0;
    int validCount = 0;
    for (int i = 0; i < n; ++i) {
        if (assignments[i] < 0) continue;
        int myCluster = assignments[i];

        /* a: 簇内平均距离 */
        double a = 0.0;
        int aCount = 0;
        for (int idx : clusters[myCluster].memberIndices) {
            if (idx != i) { a += qAbs(data[i] - data[idx]); ++aCount; }
        }
        a = (aCount > 0) ? a / aCount : 0.0;

        /* b: 最近其他簇的平均距离 */
        double b = std::numeric_limits<double>::max();
        for (int c = 0; c < clusters.size(); ++c) {
            if (c == myCluster || clusters[c].memberIndices.isEmpty()) continue;
            double avgDist = 0.0;
            for (int idx : clusters[c].memberIndices) avgDist += qAbs(data[i] - data[idx]);
            avgDist /= clusters[c].memberIndices.size();
            if (avgDist < b) b = avgDist;
        }

        if (clusters.size() < 2) { totalScore += 0.0; }
        else { totalScore += (b - a) / qMax(a, b); }
        ++validCount;
    }
    return (validCount > 0) ? totalScore / validCount : 0.0;
}

double KMeansClusterer::distance(double a, double b) const { return qAbs(a - b); }

double KMeansClusterer::computeInertia(const QVector<double>& data,
                                        const QList<Cluster>& clusters) const
{
    double inertia = 0.0;
    for (const auto& cl : clusters) {
        for (int idx : cl.memberIndices) {
            double d = data[idx] - cl.centroid;
            inertia += d * d;
        }
    }
    return inertia;
}

QList<KMeansClusterer::Cluster> KMeansClusterer::buildClusters(
    const QVector<double>& data, const QVector<int>& assignments,
    const QVector<double>& centroids) const
{
    QList<Cluster> result;
    for (int c = 0; c < centroids.size(); ++c) {
        Cluster cl;
        cl.centroid = centroids[c];
        cl.variance = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            if (assignments[i] == c) cl.memberIndices.append(i);
        }
        if (!cl.memberIndices.isEmpty()) {
            double sqSum = 0.0;
            for (int idx : cl.memberIndices) {
                double d = data[idx] - cl.centroid;
                sqSum += d * d;
            }
            cl.variance = sqSum / cl.memberIndices.size();
        }
        result.append(cl);
    }
    return result;
}

void KMeansClusterer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
