/**
 * @file KMeans30.cpp
 * @brief KMeans30 实现
 *
 * 实现K-means聚类：Hartigan-Wong直接更新与簇内平方和最小化的高效质心重定位。
 */

#include "utils/cluster283/KMeans30.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

KMeans30::KMeans30(QObject *parent)
    : QObject(parent) {}

KMeans30::~KMeans30() = default;

/* ---- Configuration ---- */

void KMeans30::setNumClusters(int k) { m_k = qBound(1, k, 128); }
void KMeans30::setMaxIterations(int iters) { m_maxIter = qBound(1, iters, 10000); }
void KMeans30::setTolerance(double tol) { m_tol = qBound(1e-10, tol, 1.0); }
void KMeans30::setSeed(unsigned int seed) { m_seed = seed; }

/* ---- Squared Euclidean distance ---- */

double KMeans30::sqDist(const QVector<double>& a, const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}

/* ---- K-means++ initialization ---- */

void KMeans30::initCentroids(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    m_dim = data[0].size();

    m_centroids.resize(m_k);
    m_labels.resize(n);
    m_clusterWcss.resize(m_k, 0.0);
    m_clusterSizes.resize(m_k, 0);

    // Pick first centroid
    quint64 s = m_seed;
    int first = static_cast<int>(s % static_cast<quint64>(n));
    m_centroids[0] = data[first];

    QVector<double> minDist(n, 1e30);
    for (int c = 1; c < m_k; ++c) {
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = sqDist(data[i], m_centroids[c - 1]);
            if (d < minDist[i]) minDist[i] = d;
            totalDist += minDist[i];
        }

        // Weighted selection using LCG
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        double threshold = totalDist * static_cast<double>(s >> 33) / static_cast<double>(1ULL << 31);
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_centroids[c] = data[chosen];
    }
}

/* ---- Update centroids from assignments ---- */

void KMeans30::updateCentroids(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int dim = m_dim;

    for (int c = 0; c < m_k; ++c) {
        m_centroids[c].fill(0.0, dim);
        m_clusterSizes[c] = 0;
    }

    for (int i = 0; i < n; ++i) {
        int c = m_labels[i];
        m_clusterSizes[c]++;
        for (int d = 0; d < dim; ++d)
            m_centroids[c][d] += data[i][d];
    }

    for (int c = 0; c < m_k; ++c) {
        if (m_clusterSizes[c] > 0) {
            for (int d = 0; d < dim; ++d)
                m_centroids[c][d] /= m_clusterSizes[c];
        }
    }
}

/* ---- Recompute WCSS per cluster ---- */

void KMeans30::recomputeWcss(const QVector<QVector<double>>& data)
{
    m_clusterWcss.fill(0.0, m_k);
    for (int i = 0; i < data.size(); ++i) {
        int c = m_labels[i];
        m_clusterWcss[c] += sqDist(data[i], m_centroids[c]);
    }
}

/* ---- Hartigan-Wong single-point transfer step ---- */

bool KMeans30::hartiganWongStep(const QVector<QVector<double>>& data)
{
    int n = data.size();
    bool moved = false;

    for (int i = 0; i < n; ++i) {
        int oldC = m_labels[i];
        double oldContrib = sqDist(data[i], m_centroids[oldC]);

        // Try moving to each other cluster
        int bestC = oldC;
        double bestDelta = 0.0;

        for (int c = 0; c < m_k; ++c) {
            if (c == oldC) continue;
            double newContrib = sqDist(data[i], m_centroids[c]);
            double delta = newContrib - oldContrib;
            if (delta < bestDelta) {
                bestDelta = delta;
                bestC = c;
            }
        }

        if (bestC != oldC) {
            // Transfer point i from oldC to bestC
            m_clusterSizes[oldC]--;
            m_clusterSizes[bestC]++;

            // Update centroids directly (Hartigan-Wong)
            for (int d = 0; d < m_dim; ++d) {
                if (m_clusterSizes[oldC] > 0)
                    m_centroids[oldC][d] = (m_centroids[oldC][d] * (m_clusterSizes[oldC] + 1)
                                            - data[i][d]) / m_clusterSizes[oldC];
                else
                    m_centroids[oldC][d] = 0.0;

                m_centroids[bestC][d] = (m_centroids[bestC][d] * (m_clusterSizes[bestC] - 1)
                                         + data[i][d]) / m_clusterSizes[bestC];
            }

            m_clusterWcss[oldC] -= oldContrib;
            m_clusterWcss[bestC] += sqDist(data[i], m_centroids[bestC]);

            m_labels[i] = bestC;
            moved = true;
        }
    }
    return moved;
}

/* ---- Main fit ---- */

KMeans30::ClusterResult KMeans30::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0 || m_k <= 0) return result;

    initCentroids(data);

    // Initial assignment
    for (int i = 0; i < n; ++i) {
        int best = 0;
        double bestDist = sqDist(data[i], m_centroids[0]);
        for (int c = 1; c < m_k; ++c) {
            double d = sqDist(data[i], m_centroids[c]);
            if (d < bestDist) { bestDist = d; best = c; }
        }
        m_labels[i] = best;
    }

    // Hartigan-Wong iterations
    for (int iter = 0; iter < m_maxIter; ++iter) {
        updateCentroids(data);
        recomputeWcss(data);

        bool moved = hartiganWongStep(data);
        if (!moved) {
            result.converged = true;
            result.iterations = iter + 1;
            break;
        }
        result.iterations = iter + 1;
    }

    // Final WCSS
    recomputeWcss(data);
    result.totalWcss = 0.0;
    for (int c = 0; c < m_k; ++c)
        result.totalWcss += m_clusterWcss[c];

    result.centroids = m_centroids;
    result.labels = m_labels;
    result.wcss = m_clusterWcss;

    double elapsed = timer.elapsed();
    m_stats.numClusters = m_k;
    m_stats.numPoints = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fitDone(n, m_k, result.totalWcss, elapsed);

    return result;
}

/* ---- Predict ---- */

QVector<int> KMeans30::predict(const QVector<QVector<double>>& samples) const
{
    QVector<int> result(samples.size());
    for (int i = 0; i < samples.size(); ++i) {
        int best = 0;
        double bestDist = sqDist(samples[i], m_centroids[0]);
        for (int c = 1; c < m_k; ++c) {
            double d = sqDist(samples[i], m_centroids[c]);
            if (d < bestDist) { bestDist = d; best = c; }
        }
        result[i] = best;
    }
    return result;
}

/* ---- Compute WCSS ---- */

double KMeans30::computeWcss(const QVector<QVector<double>>& data) const
{
    double wcss = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        int c = (i < m_labels.size()) ? m_labels[i] : 0;
        if (c < m_centroids.size())
            wcss += sqDist(data[i], m_centroids[c]);
    }
    return wcss;
}

/* ---- Reset ---- */

void KMeans30::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
    m_labels.clear();
    m_clusterWcss.clear();
    m_clusterSizes.clear();
}
