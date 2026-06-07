/**
 * @file KMeans19.cpp
 * @brief KMeans19 实现
 *
 * 实现流式Mini-Batch K-means：蓄水池采样、质心动量更新、K-means++初始化。
 */

#include "utils/cluster202/KMeans19.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <QtGlobal>

/* ---- Construction / Destruction ---- */

KMeans19::KMeans19(QObject *parent) : QObject(parent) {}
KMeans19::~KMeans19() = default;

/* ---- Configuration ---- */

void KMeans19::setNumClusters(int k) { m_k = qMax(1, k); }
void KMeans19::setBatchSize(int size) { m_batchSize = qMax(1, size); }
void KMeans19::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void KMeans19::setMomentum(double beta) { m_momentum = qBound(0.0, beta, 1.0); }

/* ---- Distance ---- */

double KMeans19::distance(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Nearest centroid ---- */

int KMeans19::nearestCentroid(const QVector<double>& point) const
{
    int best = 0;
    double bestDist = std::numeric_limits<double>::max();
    for (int c = 0; c < m_centroids.size(); ++c) {
        double d = distance(point, m_centroids[c]);
        if (d < bestDist) { bestDist = d; best = c; }
    }
    return best;
}

/* ---- K-means++ initialization ---- */

void KMeans19::kmeansPlusPlusInit(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;

    m_centroids.clear();
    m_centroids.resize(m_k);

    // First centroid: random
    int first = QRandomGenerator::global()->bounded(n);
    m_centroids[0] = data[first];

    QVector<double> minDist(n, std::numeric_limits<double>::max());
    for (int c = 1; c < m_k; ++c) {
        // Update distances to nearest existing centroid
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = distance(data[i], m_centroids[c - 1]);
            minDist[i] = qMin(minDist[i], d * d);
            totalDist += minDist[i];
        }
        // Weighted random selection
        double r = QRandomGenerator::global()->generateDouble() * totalDist;
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_centroids[c] = data[chosen];
    }

    // Initialize velocity
    int dim = m_centroids[0].size();
    m_velocity = QVector<QVector<double>>(m_k, QVector<double>(dim, 0.0));
    m_clusterCounts.resize(m_k);
    m_clusterCounts.fill(0);
}

/* ---- Batch fit ---- */

QVector<int> KMeans19::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};
    kmeansPlusPlusInit(data);

    QVector<int> assignments(n, 0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;

        // Assign points to nearest centroid
        for (int i = 0; i < n; ++i) {
            int newCluster = nearestCentroid(data[i]);
            if (newCluster != assignments[i]) { assignments[i] = newCluster; changed = true; }
        }
        if (!changed) break;

        // Recompute centroids
        QVector<QVector<double>> sums(m_k, QVector<double>(data[0].size(), 0.0));
        QVector<int> counts(m_k, 0);
        for (int i = 0; i < n; ++i) {
            int c = assignments[i];
            for (int d = 0; d < data[i].size(); ++d) sums[c][d] += data[i][d];
            counts[c]++;
        }
        for (int c = 0; c < m_k; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < sums[c].size(); ++d)
                    m_centroids[c][d] = sums[c][d] / counts[c];
            }
        }
    }

    m_streamBuffer = data;
    m_stats.totalFits++;
    m_stats.numSamples = n;
    m_stats.numClusters = m_k;
    m_stats.inertia = computeInertia(data);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit clusteringCompleted(m_k, m_stats.inertia, timer.elapsed());
    return assignments;
}

/* ---- Reservoir sampling ---- */

QVector<QVector<double>> KMeans19::reservoirSample(int batchSize) const
{
    int n = m_streamBuffer.size();
    if (n <= batchSize) return m_streamBuffer;

    QVector<QVector<double>> reservoir(batchSize);
    for (int i = 0; i < batchSize; ++i) reservoir[i] = m_streamBuffer[i];

    for (int i = batchSize; i < n; ++i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        if (j < batchSize) reservoir[j] = m_streamBuffer[i];
    }
    return reservoir;
}

/* ---- Momentum update ---- */

void KMeans19::momentumUpdate(const QVector<QVector<double>>& batch, const QVector<int>& assignments)
{
    QVector<QVector<double>> sums(m_k, QVector<double>(m_centroids[0].size(), 0.0));
    QVector<int> counts(m_k, 0);

    for (int i = 0; i < batch.size(); ++i) {
        int c = assignments[i];
        for (int d = 0; d < batch[i].size(); ++d) sums[c][d] += batch[i][d];
        counts[c]++;
    }

    for (int c = 0; c < m_k; ++c) {
        if (counts[c] == 0) continue;
        for (int d = 0; d < sums[c].size(); ++d) {
            double target = sums[c][d] / counts[c];
            double grad = target - m_centroids[c][d];
            m_velocity[c][d] = m_momentum * m_velocity[c][d] + (1.0 - m_momentum) * grad;
            m_centroids[c][d] += m_velocity[c][d];
        }
    }
}

/* ---- Partial fit ---- */

void KMeans19::partialFit(const QVector<QVector<double>>& batch)
{
    QElapsedTimer timer;
    timer.start();
    if (batch.isEmpty()) return;

    // Append to stream buffer
    m_streamBuffer.append(batch);

    if (m_centroids.isEmpty()) {
        kmeansPlusPlusInit(batch);
    }

    QVector<QVector<double>> sample = reservoirSample(m_batchSize);
    QVector<int> assignments(sample.size());
    for (int i = 0; i < sample.size(); ++i)
        assignments[i] = nearestCentroid(sample[i]);

    momentumUpdate(sample, assignments);

    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;
}

/* ---- Predict ---- */

QVector<int> KMeans19::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> result(data.size());
    for (int i = 0; i < data.size(); ++i) result[i] = nearestCentroid(data[i]);
    return result;
}

/* ---- Centroids ---- */

QVector<QVector<double>> KMeans19::centroids() const { return m_centroids; }

/* ---- Inertia ---- */

double KMeans19::computeInertia(const QVector<QVector<double>>& data) const
{
    double inertia = 0.0;
    for (const auto& pt : data) {
        int c = nearestCentroid(pt);
        double d = distance(pt, m_centroids[c]);
        inertia += d * d;
    }
    return inertia;
}

/* ---- Reset ---- */

void KMeans19::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
    m_velocity.clear();
    m_streamBuffer.clear();
    m_clusterCounts.clear();
}
