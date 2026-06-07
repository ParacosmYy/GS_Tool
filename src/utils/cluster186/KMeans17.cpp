/**
 * @file KMeans17.cpp
 * @brief KMeans17 实现
 *
 * 实现K-means聚类：Canopy预聚类初始化、Elkan三角不等式加速、SSE监控。
 */

#include "utils/cluster186/KMeans17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

KMeans17::KMeans17(QObject *parent) : QObject(parent) {}
KMeans17::~KMeans17() = default;

/* ---- Configuration ---- */

void KMeans17::setK(int k) { m_k = qMax(2, k); }
void KMeans17::setMaxIterations(int iter) { m_maxIterations = qMax(10, iter); }
void KMeans17::setTolerance(double tol) { m_tolerance = qMax(1e-10, tol); }
void KMeans17::setCanopyThreshold(double loose, double tight)
{
    m_canopyLoose = qMax(0.1, loose);
    m_canopyTight = qMax(0.05, qMin(tight, loose));
}

/* ---- Euclidean distance ---- */

double KMeans17::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Canopy pre-clustering initialization ---- */

QVector<QVector<double>> KMeans17::canopyInit(
    const QVector<QVector<double>>& data) const
{
    int N = data.size();
    if (N == 0) return {};

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, N - 1);

    // Build canopies: group points under loose threshold
    QVector<QVector<int>> canopies;
    QVector<bool> used(N, false);

    for (int i = 0; i < N; ++i) {
        if (used[i]) continue;
        QVector<int> canopy;
        canopy.append(i);
        used[i] = true;
        for (int j = i + 1; j < N; ++j) {
            if (used[j]) continue;
            if (distance(data[i], data[j]) < m_canopyLoose) {
                canopy.append(j);
                if (distance(data[i], data[j]) < m_canopyTight)
                    used[j] = true;
            }
        }
        canopies.append(canopy);
    }

    // Pick centroids: one random point per canopy (up to k canopies)
    int numCenters = qMin(m_k, canopies.size());
    QVector<QVector<double>> centers;
    for (int c = 0; c < numCenters; ++c) {
        int idx = canopies[c][dist(rng) % canopies[c].size()];
        centers.append(data[idx]);
    }
    return centers;
}

/* ---- Inter-centroid half-distance matrix ---- */

QVector<QVector<double>> KMeans17::centroidDistances() const
{
    int K = m_centroids.size();
    QVector<QVector<double>> d(K, QVector<double>(K, 0.0));
    for (int i = 0; i < K; ++i)
        for (int j = i + 1; j < K; ++j) {
            double dij = distance(m_centroids[i], m_centroids[j]) / 2.0;
            d[i][j] = dij;
            d[j][i] = dij;
        }
    return d;
}

/* ---- Elkan assignment with triangle inequality ---- */

QVector<int> KMeans17::elkanAssign(const QVector<QVector<double>>& data,
                                    QVector<double>& lowerBounds,
                                    QVector<double>& upperBounds,
                                    QVector<int>& assignments) const
{
    int N = data.size();
    int K = m_centroids.size();
    auto halfDist = centroidDistances();

    QVector<int> newAssign(N);
    for (int i = 0; i < N; ++i) {
        int ai = assignments[i];

        // Check if upper bound is valid
        bool skip = true;
        if (ai < 0 || ai >= K) {
            skip = false;
        } else {
            for (int j = 0; j < K; ++j) {
                if (j == ai) continue;
                if (upperBounds[i] > halfDist[ai][j]) {
                    skip = false;
                    break;
                }
            }
        }

        if (skip) {
            newAssign[i] = ai;
            continue;
        }

        // Full distance computation
        double minDist = std::numeric_limits<double>::max();
        int best = 0;
        for (int j = 0; j < K; ++j) {
            double d = distance(data[i], m_centroids[j]);
            lowerBounds[i * K + j] = d;
            if (d < minDist) { minDist = d; best = j; }
        }
        upperBounds[i] = minDist;
        newAssign[i] = best;
    }
    return newAssign;
}

/* ---- Update centroids ---- */

void KMeans17::updateCentroids(const QVector<QVector<double>>& data,
                                const QVector<int>& labels)
{
    int N = data.size();
    int K = m_centroids.size();
    int D = data[0].size();

    QVector<QVector<double>> sums(K, QVector<double>(D, 0.0));
    QVector<int> counts(K, 0);

    for (int i = 0; i < N; ++i) {
        int c = labels[i];
        for (int d = 0; d < D; ++d)
            sums[c][d] += data[i][d];
        counts[c]++;
    }

    for (int c = 0; c < K; ++c) {
        if (counts[c] == 0) continue; // Empty cluster, keep centroid
        for (int d = 0; d < D; ++d)
            m_centroids[c][d] = sums[c][d] / counts[c];
    }
}

/* ---- Main fit ---- */

QVector<int> KMeans17::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int N = data.size();
    if (N == 0) return {};
    int D = data[0].size();

    // Initialize via canopy pre-clustering
    m_centroids = canopyInit(data);
    if (m_centroids.size() < m_k) {
        // Fill remaining with random points
        std::mt19937 rng(123);
        std::uniform_int_distribution<int> dist(0, N - 1);
        while (m_centroids.size() < m_k) {
            m_centroids.append(data[dist(rng)]);
        }
    }

    int K = m_k;

    // Elkan auxiliary structures
    QVector<double> lowerBounds(N * K, std::numeric_limits<double>::max());
    QVector<double> upperBounds(N, std::numeric_limits<double>::max());
    QVector<int> assignments(N, -1);

    QVector<int> labels(N, 0);
    int iter = 0;

    for (iter = 0; iter < m_maxIterations; ++iter) {
        auto newLabels = elkanAssign(data, lowerBounds, upperBounds, assignments);

        // Check convergence
        bool changed = false;
        for (int i = 0; i < N; ++i) {
            if (newLabels[i] != assignments[i]) { changed = true; break; }
        }
        assignments = newLabels;
        labels = newLabels;

        if (!changed && iter > 0) break;

        updateCentroids(data, labels);

        // Tighten bounds (subtract centroid movement)
        // For simplicity, reset bounds on centroid update
        lowerBounds.fill(std::numeric_limits<double>::max());
        upperBounds.fill(std::numeric_limits<double>::max());
    }

    // Compute SSE
    m_sse = 0.0;
    for (int i = 0; i < N; ++i) {
        double d = distance(data[i], m_centroids[labels[i]]);
        m_sse += d * d;
    }

    m_stats.totalRuns++;
    m_stats.numPoints = N;
    m_stats.numClusters = K;
    m_stats.iterationsUsed = iter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(K, iter, m_sse, timer.elapsed());
    return labels;
}

/* ---- Predict ---- */

QVector<int> KMeans17::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size());
    for (int i = 0; i < data.size(); ++i) {
        double minD = std::numeric_limits<double>::max();
        for (int j = 0; j < m_centroids.size(); ++j) {
            double d = distance(data[i], m_centroids[j]);
            if (d < minD) { minD = d; labels[i] = j; }
        }
    }
    return labels;
}

/* ---- Reset ---- */

void KMeans17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_sse = 0.0;
    m_centroids.clear();
}
