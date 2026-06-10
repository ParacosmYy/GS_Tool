/**
 * @file KMeans29.cpp
 * @brief KMeans29 实现
 *
 * 实现K均值聚类：小批量随机梯度与核心集构造大规模数据可扩展聚类。
 */

#include "utils/cluster272/KMeans29.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

KMeans29::KMeans29(QObject *parent)
    : QObject(parent) {}

KMeans29::~KMeans29() = default;

/* ---- Configuration ---- */

void KMeans29::setClusters(int k) { m_k = qBound(1, k, 200); }
void KMeans29::setMaxIterations(int iters) { m_maxIter = qBound(1, iters, 10000); }
void KMeans29::setBatchSize(int size) { m_batchSize = qBound(8, size, 8192); }
void KMeans29::setTolerance(double tol) { m_tol = qBound(1e-12, tol, 1.0); }

/* ---- K-means++ initialization ---- */

void KMeans29::initCentroids(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    int d = data[0].size();

    m_centroids.resize(m_k);
    m_counts.resize(m_k);

    // Pick first centroid at index 0
    m_centroids[0] = data[0];

    QVector<double> dist(n, 1e18);
    for (int c = 1; c < m_k; ++c) {
        // Update squared distances to nearest existing centroid
        for (int i = 0; i < n; ++i) {
            double dSq = 0.0;
            for (int j = 0; j < d; ++j) {
                double diff = data[i][j] - m_centroids[c - 1][j];
                dSq += diff * diff;
            }
            dist[i] = qMin(dist[i], dSq);
        }
        // Weighted random selection
        double total = 0.0;
        for (double d2 : dist) total += d2;
        double r = total * (static_cast<double>(qrand()) / RAND_MAX);
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += dist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_centroids[c] = data[chosen];
    }
    m_counts.fill(0);
}

/* ---- Nearest centroid lookup ---- */

int KMeans29::nearestCentroid(const QVector<double>& point, double& distSq) const
{
    int best = 0;
    double bestDist = 1e18;
    for (int c = 0; c < m_k; ++c) {
        double d = 0.0;
        for (int j = 0; j < point.size(); ++j) {
            double diff = point[j] - m_centroids[c][j];
            d += diff * diff;
        }
        if (d < bestDist) { bestDist = d; best = c; }
    }
    distSq = bestDist;
    return best;
}

/* ---- Compute total inertia ---- */

double KMeans29::computeInertia(const QVector<QVector<double>>& data,
                                 const QVector<int>& labels) const
{
    double inertia = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double dSq = 0.0;
        for (int j = 0; j < data[i].size(); ++j) {
            double diff = data[i][j] - m_centroids[labels[i]][j];
            dSq += diff * diff;
        }
        inertia += dSq;
    }
    return inertia;
}

/* ---- Sample mini-batch indices ---- */

QVector<int> KMeans29::sampleBatch(int n) const
{
    int bSize = qMin(m_batchSize, n);
    QVector<int> indices(bSize);
    for (int i = 0; i < bSize; ++i)
        indices[i] = static_cast<int>(static_cast<double>(qrand()) / RAND_MAX * n) % n;
    return indices;
}

/* ---- Build coreset via importance sampling ---- */

void KMeans29::buildCoreset(const QVector<QVector<double>>& data, int coresetSize)
{
    int n = data.size();
    if (n == 0 || coresetSize <= 0) return;
    int d = data[0].size();

    // Compute mean point
    QVector<double> mean(d, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j)
            mean[j] += data[i][j];
    for (int j = 0; j < d; ++j) mean[j] /= n;

    // Compute distances to mean for importance sampling
    QVector<double> dist(n, 0.0);
    double total = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < d; ++j) {
            double diff = data[i][j] - mean[j];
            dist[i] += diff * diff;
        }
        dist[i] += 1e-6;  // Avoid zero probability
        total += dist[i];
    }

    // Sample coreset proportional to distance
    m_coreset.clear();
    int cs = qMin(coresetSize, n);
    for (int s = 0; s < cs; ++s) {
        double r = total * (static_cast<double>(qrand()) / RAND_MAX);
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += dist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_coreset.append(data[chosen]);
    }
}

/* ---- Full fit with Lloyd iterations ---- */

QVector<int> KMeans29::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    // Use coreset for init if available, otherwise use full data
    const auto& initData = m_coreset.isEmpty() ? data : m_coreset;
    initCentroids(initData);

    QVector<int> labels(n, 0);
    int iter = 0;
    double prevInertia = 1e18;

    for (; iter < m_maxIter; ++iter) {
        // Assign each point to nearest centroid
        for (int i = 0; i < n; ++i) {
            double dSq;
            labels[i] = nearestCentroid(data[i], dSq);
        }

        // Update centroids: compute new means
        QVector<QVector<double>> newCentroids(m_k, QVector<double>(data[0].size(), 0.0));
        QVector<int> counts(m_k, 0);
        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            counts[c]++;
            for (int j = 0; j < data[i].size(); ++j)
                newCentroids[c][j] += data[i][j];
        }
        for (int c = 0; c < m_k; ++c) {
            if (counts[c] > 0) {
                for (int j = 0; j < data[0].size(); ++j)
                    newCentroids[c][j] /= counts[c];
            }
            m_centroids[c] = newCentroids[c];
        }
        m_counts = counts;

        double curInertia = computeInertia(data, labels);
        if (qAbs(prevInertia - curInertia) < m_tol) break;
        prevInertia = curInertia;
    }

    double inertia = computeInertia(data, labels);
    double elapsed = timer.elapsed();
    m_stats.numClusters = m_k;
    m_stats.numSamples = n;
    m_stats.numIterations = iter;
    m_stats.inertia = inertia;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingDone(iter, inertia, elapsed);

    return labels;
}

/* ---- Mini-batch stochastic update ---- */

void KMeans29::miniBatchUpdate(const QVector<QVector<double>>& batch)
{
    if (batch.isEmpty() || m_centroids.isEmpty()) return;
    int d = batch[0].size();

    // Assign batch points to nearest centroid
    QVector<int> assigns(batch.size());
    for (int i = 0; i < batch.size(); ++i) {
        double dSq;
        assigns[i] = nearestCentroid(batch[i], dSq);
    }

    // Stochastic gradient update: move centroid toward batch mean
    for (int c = 0; c < m_k; ++c) {
        QVector<double> batchMean(d, 0.0);
        int cnt = 0;
        for (int i = 0; i < batch.size(); ++i) {
            if (assigns[i] == c) {
                for (int j = 0; j < d; ++j)
                    batchMean[j] += batch[i][j];
                cnt++;
            }
        }
        if (cnt > 0) {
            double eta = 1.0 / (1.0 + m_counts[c]);  // Learning rate decay
            for (int j = 0; j < d; ++j) {
                batchMean[j] /= cnt;
                m_centroids[c][j] = (1.0 - eta) * m_centroids[c][j] + eta * batchMean[j];
            }
            m_counts[c] += cnt;
        }
    }
}

/* ---- Predict single point ---- */

int KMeans29::predict(const QVector<double>& point) const
{
    double dSq;
    return nearestCentroid(point, dSq);
}

/* ---- Accessors ---- */

QVector<QVector<double>> KMeans29::centroids() const { return m_centroids; }
double KMeans29::inertia() const { return m_stats.inertia; }

/* ---- Reset ---- */

void KMeans29::resetStatistics()
{
    m_centroids.clear();
    m_counts.clear();
    m_coreset.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
