/**
 * @file MiniBatchKMeans.cpp
 * @brief Mini-Batch K-Means聚类实现
 */

#include "MiniBatchKMeans.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <cstdlib>

MiniBatchKMeans::MiniBatchKMeans(int k, int batchSize, int maxIter,
                                     QObject* parent)
    : QObject(parent)
    , m_k(qMax(2, k))
    , m_batchSize(qMax(1, batchSize))
    , m_maxIter(maxIter)
    , m_seed(42)
    , m_dims(0)
    , m_timeSum(0.0)
{
}

void MiniBatchKMeans::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return;
    m_dims = data[0].size();

    initCentroids(data);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* 随机选择batch */
        QVector<int> indices;
        int bs = qMin(m_batchSize, data.size());
        for (int i = 0; i < bs; ++i)
            indices.append(std::rand() % data.size());

        /* 找每个batch点的最近中心 */
        QVector<int> centerCounts(m_k, 0);
        QVector<QVector<double>> centerSums(m_k, QVector<double>(m_dims, 0.0));

        for (int idx : indices) {
            int c = nearestCentroid(data[idx]);
            centerCounts[c]++;
            for (int d = 0; d < m_dims; ++d)
                centerSums[c][d] += data[idx][d];
        }

        /* 更新中心 */
        for (int c = 0; c < m_k; ++c) {
            if (centerCounts[c] == 0) continue;
            double eta = 1.0 / centerCounts[c];
            for (int d = 0; d < m_dims; ++d)
                m_centroids[c][d] = (1.0 - eta) * m_centroids[c][d] + eta * centerSums[c][d] / centerCounts[c];
        }
    }

    m_stats.totalFitted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitted;

    emit fitted(m_k, m_maxIter);
}

void MiniBatchKMeans::partialFit(const QVector<QVector<double>>& batch)
{
    if (m_centroids.isEmpty()) {
        m_dims = batch[0].size();
        initCentroids(batch);
    }

    QVector<int> centerCounts(m_k, 0);
    QVector<QVector<double>> centerSums(m_k, QVector<double>(m_dims, 0.0));

    for (const auto& point : batch) {
        int c = nearestCentroid(point);
        centerCounts[c]++;
        for (int d = 0; d < m_dims; ++d)
            centerSums[c][d] += point[d];
    }

    for (int c = 0; c < m_k; ++c) {
        if (centerCounts[c] == 0) continue;
        double eta = 1.0 / centerCounts[c];
        for (int d = 0; d < m_dims; ++d)
            m_centroids[c][d] = (1.0 - eta) * m_centroids[c][d] + eta * centerSums[c][d] / centerCounts[c];
    }
}

int MiniBatchKMeans::predict(const QVector<double>& point) const
{
    m_stats.totalPredictions++;
    return nearestCentroid(point);
}

QVector<int> MiniBatchKMeans::predictBatch(const QVector<QVector<double>>& data) const
{
    QVector<int> labels;
    labels.reserve(data.size());
    for (const auto& p : data) {
        labels.append(nearestCentroid(p));
        const_cast<MiniBatchKMeans*>(this)->m_stats.totalPredictions++;
    }
    return labels;
}

double MiniBatchKMeans::inertia(const QVector<QVector<double>>& data) const
{
    double total = 0.0;
    for (const auto& p : data) {
        int c = nearestCentroid(p);
        total += euclidean(p, m_centroids[c]);
    }
    return total;
}

QVector<QVector<double>> MiniBatchKMeans::centroids() const { return m_centroids; }

void MiniBatchKMeans::setSeed(int seed) { m_seed = seed; }

void MiniBatchKMeans::initCentroids(const QVector<QVector<double>>& data)
{
    std::srand(m_seed);
    m_centroids.resize(m_k);
    m_counts.resize(m_k, 0);

    /* K-Means++初始化 */
    int first = std::rand() % data.size();
    m_centroids[0] = data[first];

    for (int c = 1; c < m_k; ++c) {
        QVector<double> dists(data.size());
        double totalDist = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            int nearest = nearestCentroid(data[i]);
            double d = euclidean(data[i], m_centroids[nearest]);
            dists[i] = d * d;
            totalDist += dists[i];
        }

        double r = static_cast<double>(std::rand()) / RAND_MAX * totalDist;
        double cumSum = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            cumSum += dists[i];
            if (cumSum >= r) {
                m_centroids[c] = data[i];
                break;
            }
        }
    }
}

int MiniBatchKMeans::nearestCentroid(const QVector<double>& point) const
{
    int best = 0;
    double bestDist = euclidean(point, m_centroids[0]);
    for (int c = 1; c < m_k; ++c) {
        double d = euclidean(point, m_centroids[c]);
        if (d < bestDist) { bestDist = d; best = c; }
    }
    return best;
}

double MiniBatchKMeans::euclidean(const QVector<double>& a,
                                     const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return std::sqrt(sum);
}

MiniBatchKMeans::Stats MiniBatchKMeans::stats() const { return m_stats; }

void MiniBatchKMeans::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
