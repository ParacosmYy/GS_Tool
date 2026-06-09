/**
 * @file KMeans25.cpp
 * @brief KMeans25 实现
 *
 * 实现K-means聚类：小批量随机梯度下降与逐迭代收敛跟踪。
 */

#include "utils/cluster244/KMeans25.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>
#include <limits>

/* ---- Construction / Destruction ---- */

KMeans25::KMeans25(QObject *parent) : QObject(parent) {}
KMeans25::~KMeans25() = default;

/* ---- Configuration ---- */

void KMeans25::setK(int k) { m_k = qMax(1, k); }
void KMeans25::setBatchSize(int size) { m_batchSize = qMax(1, size); }
void KMeans25::setTolerance(double tol) { m_tol = qMax(1e-12, tol); }
void KMeans25::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }

/* ---- Squared Euclidean distance ---- */

double KMeans25::sqDistance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- Find nearest centroid ---- */

int KMeans25::nearestCentroid(const QVector<double>& point) const
{
    int best = 0;
    double bestDist = sqDistance(point, m_centroids[0]);
    for (int c = 1; c < m_k; ++c) {
        double dist = sqDistance(point, m_centroids[c]);
        if (dist < bestDist) { bestDist = dist; best = c; }
    }
    return best;
}

/* ---- K-means++ initialization ---- */

void KMeans25::initCentroids(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    int d = data[0].size();

    m_centroids.resize(m_k);
    m_counts.resize(m_k);

    // Pick first center randomly
    int first = static_cast<int>(std::rand() % n);
    m_centroids[0] = data[first];

    QVector<double> dists(n, 1e18);
    for (int c = 1; c < m_k; ++c) {
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double sq = sqDistance(data[i], m_centroids[c - 1]);
            dists[i] = qMin(dists[i], sq);
            totalDist += dists[i];
        }
        // Weighted random selection
        double r = totalDist * static_cast<double>(std::rand()) / RAND_MAX;
        double cumSum = 0.0;
        int sel = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { sel = i; break; }
        }
        m_centroids[c] = data[sel];
    }
}

/* ---- Compute inertia ---- */

double KMeans25::computeInertia(const QVector<QVector<double>>& data) const
{
    double inertia = 0.0;
    for (int i = 0; i < data.size(); ++i)
        inertia += sqDistance(data[i], m_centroids[m_labels[i]]);
    return inertia;
}

/* ---- Fit with mini-batch SGD ---- */

void KMeans25::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return;
    int d = data[0].size();

    m_history.clear();
    m_labels.resize(n);
    m_counts.fill(0, m_k);

    initCentroids(data);

    // Mini-batch K-means iterations
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Sample a mini-batch
        int bSize = qMin(m_batchSize, n);
        QVector<int> batch(bSize);
        for (int i = 0; i < bSize; ++i)
            batch[i] = static_cast<int>(std::rand() % n);

        // Compute assignments for the batch
        QVector<int> batchLabels(bSize);
        for (int i = 0; i < bSize; ++i)
            batchLabels[i] = nearestCentroid(data[batch[i]]);

        // Stochastic gradient update: move centroids toward batch mean
        QVector<QVector<double>> batchSum(m_k, QVector<double>(d, 0.0));
        QVector<int> batchCount(m_k, 0);
        for (int i = 0; i < bSize; ++i) {
            int c = batchLabels[i];
            batchCount[c]++;
            for (int j = 0; j < d; ++j)
                batchSum[c][j] += data[batch[i]][j];
        }

        // Per-centroid learning rate: eta = 1 / count
        double totalShift = 0.0;
        for (int c = 0; c < m_k; ++c) {
            if (batchCount[c] == 0) continue;
            m_counts[c] += batchCount[c];
            double eta = 1.0 / m_counts[c];
            for (int j = 0; j < d; ++j) {
                double target = batchSum[c][j] / batchCount[c];
                double shift = eta * (target - m_centroids[c][j]);
                m_centroids[c][j] += shift;
                totalShift += shift * shift;
            }
        }

        // Assign all points (for inertia tracking)
        for (int i = 0; i < n; ++i)
            m_labels[i] = nearestCentroid(data[i]);

        double inertia = computeInertia(data);
        IterationRecord rec;
        rec.inertia = inertia;
        rec.centroidShift = qSqrt(totalShift);
        rec.iteration = iter;
        m_history.append(rec);

        // Convergence check
        if (qSqrt(totalShift) < m_tol) break;
    }

    m_stats.numClusters = m_k;
    m_stats.numSamples = n;
    m_stats.numDimensions = d;
    m_stats.totalIterations = m_history.size();
    m_stats.finalInertia = m_history.isEmpty() ? 0.0 : m_history.last().inertia;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fittingCompleted(m_k, m_stats.finalInertia,
                          m_stats.totalIterations, timer.elapsed());
}

/* ---- Predict ---- */

int KMeans25::predict(const QVector<double>& point) const
{
    if (m_centroids.isEmpty()) return -1;
    return nearestCentroid(point);
}

/* ---- Accessors ---- */

QVector<int> KMeans25::labels() const { return m_labels; }
QVector<QVector<double>> KMeans25::centroids() const { return m_centroids; }
QVector<KMeans25::IterationRecord> KMeans25::convergenceHistory() const { return m_history; }

/* ---- Reset ---- */

void KMeans25::resetStatistics()
{
    m_centroids.clear(); m_labels.clear(); m_counts.clear(); m_history.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
