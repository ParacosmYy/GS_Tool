/**
 * @file KMeans21.cpp
 * @brief KMeans21 实现
 *
 * 实现平衡K均值：k-means++初始化、最小费用流平衡分配、Lloyd迭代重平衡。
 */

#include "utils/cluster216/KMeans21.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMeans21::KMeans21(QObject *parent) : QObject(parent) {}
KMeans21::~KMeans21() = default;

/* ---- Configuration ---- */

void KMeans21::setParameters(int clusters, int maxIter, double balanceTolerance)
{
    m_clusters = qMax(2, clusters);
    m_maxIter = qMax(10, maxIter);
    m_balanceTol = qBound(0.01, balanceTolerance, 0.5);
}

/* ---- Squared Euclidean distance ---- */

double KMeans21::squaredDist(const QVector<double>& a,
                              const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}

/* ---- K-means++ initialization ---- */

void KMeans21::initializeCentroids(const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_centroids.resize(m_clusters);
    m_centroids[0] = data[0];

    QVector<double> minDist(n, std::numeric_limits<double>::max());
    for (int k = 1; k < m_clusters; ++k) {
        double total = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = squaredDist(data[i], m_centroids[k - 1]);
            minDist[i] = qMin(minDist[i], d);
            total += minDist[i];
        }
        // Weighted selection
        double threshold = total * 0.5;
        double cum = 0.0;
        int sel = k;
        for (int i = 0; i < n; ++i) {
            cum += minDist[i];
            if (cum >= threshold) { sel = i; break; }
        }
        m_centroids[k] = data[sel];
    }
}

/* ---- Balanced assignment via min-cost flow (greedy approximation) ---- */

QVector<int> KMeans21::balancedAssign(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int targetSize = n / m_clusters;
    QVector<int> labels(n, 0);
    QVector<int> clusterCount(m_clusters, 0);

    // Compute all distances and sort by cost
    QVector<QPair<double, QPair<int, int>>> costList;
    costList.reserve(n * m_clusters);
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < m_clusters; ++k) {
            double d = squaredDist(data[i], m_centroids[k]);
            costList.append({d, {i, k}});
        }
    }
    std::sort(costList.begin(), costList.end());

    // Greedy assignment respecting balance constraints
    int maxPerCluster = targetSize + qCeil(n * m_balanceTol / m_clusters);
    for (auto& item : costList) {
        int sample = item.second.first;
        int cluster = item.second.second;
        if (labels[sample] == 0 && clusterCount[cluster] < maxPerCluster) {
            // Mark unassigned (use 1-based to detect unassigned)
            labels[sample] = cluster + 1;
            clusterCount[cluster]++;
        }
    }

    // Fix any unassigned samples
    for (int i = 0; i < n; ++i) {
        if (labels[i] == 0) {
            int best = 0;
            double bestD = std::numeric_limits<double>::max();
            for (int k = 0; k < m_clusters; ++k) {
                double d = squaredDist(data[i], m_centroids[k]);
                if (d < bestD) { bestD = d; best = k; }
            }
            labels[i] = best + 1;
        }
    }

    // Convert to 0-based
    for (int i = 0; i < n; ++i) labels[i]--;
    return labels;
}

/* ---- Update centroids ---- */

void KMeans21::updateCentroids(const QVector<QVector<double>>& data,
                                const QVector<int>& labels)
{
    for (int k = 0; k < m_clusters; ++k) {
        QVector<double> sum(m_dim, 0.0);
        int count = 0;
        for (int i = 0; i < data.size(); ++i) {
            if (labels[i] == k) {
                for (int d = 0; d < m_dim; ++d) sum[d] += data[i][d];
                count++;
            }
        }
        if (count > 0) {
            for (int d = 0; d < m_dim; ++d)
                m_centroids[k][d] = sum[d] / count;
        }
    }
}

/* ---- Fit ---- */

void KMeans21::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_clusters) return;
    m_dim = data[0].size();

    initializeCentroids(data);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<int> newLabels = balancedAssign(data);

        // Check convergence
        bool changed = false;
        if (m_labels.size() == n) {
            for (int i = 0; i < n; ++i) {
                if (m_labels[i] != newLabels[i]) { changed = true; break; }
            }
        } else {
            changed = true;
        }

        m_labels = newLabels;
        updateCentroids(data, m_labels);

        // Compute inertia
        double iner = inertia(data);
        m_stats.inertia = iner;
        m_stats.iterations = iter + 1;

        if (!changed) break;
    }

    // Compute balance ratio
    QVector<int> counts(m_clusters, 0);
    for (int l : m_labels) counts[l]++;
    int minC = *std::min_element(counts.begin(), counts.end());
    int maxC = *std::max_element(counts.begin(), counts.end());
    m_stats.balanceRatio = maxC > 0 ? double(minC) / maxC : 0.0;

    m_stats.numSamples = n;
    m_stats.numClusters = m_clusters;
    m_stats.dimensions = m_dim;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fittingCompleted(m_clusters, m_stats.inertia, timer.elapsed());
}

/* ---- Predict ---- */

QVector<int> KMeans21::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size());
    for (int i = 0; i < data.size(); ++i) {
        int best = 0;
        double bestD = std::numeric_limits<double>::max();
        for (int k = 0; k < m_clusters; ++k) {
            double d = squaredDist(data[i], m_centroids[k]);
            if (d < bestD) { bestD = d; best = k; }
        }
        labels[i] = best;
    }
    return labels;
}

/* ---- Centroids ---- */

QVector<QVector<double>> KMeans21::centroids() const { return m_centroids; }

/* ---- Inertia ---- */

double KMeans21::inertia(const QVector<QVector<double>>& data) const
{
    double sum = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        if (i < m_labels.size()) {
            sum += squaredDist(data[i], m_centroids[m_labels[i]]);
        }
    }
    return sum;
}

/* ---- Reset ---- */

void KMeans21::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
    m_labels.clear();
    m_dim = 0;
}
