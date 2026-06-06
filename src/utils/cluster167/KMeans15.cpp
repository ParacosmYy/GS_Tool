/**
 * @file KMeans15.cpp
 * @brief KMeans15 实现
 *
 * 实现K-Means++：k-means||初始化、Lloyd迭代、小批量变体。
 */

#include "utils/cluster167/KMeans15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

KMeans15::KMeans15(QObject *parent)
    : QObject(parent)
{
}

KMeans15::~KMeans15() = default;

void KMeans15::setInitMethod(InitMethod method) { m_init = method; }
void KMeans15::setRunMode(RunMode mode) { m_mode = mode; }
void KMeans15::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void KMeans15::setMiniBatchSize(int batchSize) { m_batchSize = qMax(1, batchSize); }
void KMeans15::setTolerance(double tol) { m_tol = qMax(1e-12, tol); }

double KMeans15::distSq(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

void KMeans15::initKMeansPP(const QVector<QVector<double>>& data, int k)
{
    int n = data.size();
    if (n == 0) return;
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, n - 1);

    m_centroids.resize(k);
    m_centroids[0] = data[dist(gen)];

    QVector<double> minDist(n, 1e30);
    for (int c = 1; c < k; ++c) {
        /* Update min distances to nearest centroid */
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = distSq(data[i], m_centroids[c - 1]);
            if (d < minDist[i]) minDist[i] = d;
            totalDist += minDist[i];
        }
        /* Weighted random selection */
        std::uniform_real_distribution<double> rdist(0.0, totalDist);
        double r = rdist(gen);
        double cumulative = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumulative += minDist[i];
            if (cumulative >= r) { chosen = i; break; }
        }
        m_centroids[c] = data[chosen];
    }
}

void KMeans15::initKMeansParallel(const QVector<QVector<double>>& data, int k)
{
    /* k-means||: oversample then reduce to k centroids */
    int n = data.size();
    if (n == 0) return;
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> uid(0, n - 1);

    /* Pick first center */
    QVector<QVector<double>> centers;
    centers.append(data[uid(0)]);

    int rounds = 5;  /* Number of oversampling rounds */
    double oversampleFactor = 2.0 * k;

    for (int r = 0; r < rounds; ++r) {
        /* Compute distances to nearest center */
        double totalDist = 0.0;
        QVector<double> minDist(n, 1e30);
        for (int i = 0; i < n; ++i) {
            for (const auto& c : centers) {
                double d = distSq(data[i], c);
                if (d < minDist[i]) minDist[i] = d;
            }
            totalDist += minDist[i];
        }
        /* Sample new candidates with probability proportional to dist^2 */
        for (int i = 0; i < n; ++i) {
            double prob = oversampleFactor * minDist[i] / qMax(totalDist, 1e-30);
            std::uniform_real_distribution<double> rd(0.0, 1.0);
            if (rd(gen) < prob) centers.append(data[i]);
        }
    }

    /* Reduce to k centroids via weighted k-means++ on centers */
    m_centroids.clear();
    int nc = qMin(k, centers.size());
    if (nc == 0) return;

    m_centroids.append(centers[0]);
    QVector<double> wDist(centers.size());
    for (int c = 1; c < nc; ++c) {
        double total = 0.0;
        for (int i = 0; i < centers.size(); ++i) {
            double best = 1e30;
            for (const auto& ct : m_centroids) {
                double d = distSq(centers[i], ct);
                if (d < best) best = d;
            }
            wDist[i] = best;
            total += best;
        }
        std::uniform_real_distribution<double> rd(0.0, total);
        double r = rd(gen);
        double cum = 0.0;
        int chosen = 0;
        for (int i = 0; i < centers.size(); ++i) {
            cum += wDist[i];
            if (cum >= r) { chosen = i; break; }
        }
        m_centroids.append(centers[chosen]);
    }
}

void KMeans15::initRandom(const QVector<QVector<double>>& data, int k)
{
    int n = data.size();
    m_centroids.resize(k);
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, n - 1);
    for (int c = 0; c < k; ++c)
        m_centroids[c] = data[dist(gen)];
}

int KMeans15::assignCluster(const QVector<double>& sample) const
{
    double bestDist = 1e30;
    int best = 0;
    for (int c = 0; c < m_centroids.size(); ++c) {
        double d = distSq(sample, m_centroids[c]);
        if (d < bestDist) { bestDist = d; best = c; }
    }
    return best;
}

QVector<int> KMeans15::runLloyd(const QVector<QVector<double>>& data, int k)
{
    int n = data.size();
    int dim = data.isEmpty() ? 0 : data[0].size();
    QVector<int> labels(n, 0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* Assign */
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            int c = assignCluster(data[i]);
            if (c != labels[i]) { labels[i] = c; changed = true; }
        }
        if (!changed) { m_stats.lastIterations = iter + 1; break; }
        m_stats.lastIterations = iter + 1;

        /* Update centroids */
        QVector<double> counts(k, 0.0);
        for (auto& ct : m_centroids)
            ct.assign(dim, 0.0);
        for (int i = 0; i < n; ++i) {
            counts[labels[i]] += 1.0;
            for (int d = 0; d < dim; ++d)
                m_centroids[labels[i]][d] += data[i][d];
        }
        for (int c = 0; c < k; ++c)
            if (counts[c] > 1e-15)
                for (int d = 0; d < dim; ++d)
                    m_centroids[c][d] /= counts[c];
    }
    return labels;
}

QVector<int> KMeans15::runMiniBatch(const QVector<QVector<double>>& data, int k)
{
    int n = data.size();
    int dim = data.isEmpty() ? 0 : data[0].size();
    QVector<int> labels(n, 0);
    QVector<double> counts(k, 0.0);
    std::mt19937 gen(42);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* Sample mini-batch */
        int bs = qMin(m_batchSize, n);
        QVector<int> batch(bs);
        std::uniform_int_distribution<int> uid(0, n - 1);
        for (int i = 0; i < bs; ++i) batch[i] = uid(gen);

        /* Assign and update */
        for (int idx : batch) {
            int c = assignCluster(data[idx]);
            counts[c] += 1.0;
            double eta = 1.0 / counts[c];
            for (int d = 0; d < dim; ++d)
                m_centroids[c][d] = (1.0 - eta) * m_centroids[c][d] + eta * data[idx][d];
        }
        m_stats.lastIterations = iter + 1;
    }
    /* Final assignment */
    for (int i = 0; i < n; ++i)
        labels[i] = assignCluster(data[i]);
    return labels;
}

QVector<int> KMeans15::fit(const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || k <= 0) return QVector<int>();
    k = qMin(k, n);

    /* Initialize */
    switch (m_init) {
    case KMeansPP: initKMeansPP(data, k); break;
    case KMeansParallel: initKMeansParallel(data, k); break;
    case Random: initRandom(data, k); break;
    }

    /* Run */
    QVector<int> labels;
    if (m_mode == MiniBatch)
        labels = runMiniBatch(data, k);
    else
        labels = runLloyd(data, k);

    m_labels = labels;
    m_stats.totalRuns++;
    m_stats.lastK = k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0) ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(k, m_stats.lastIterations);
    return labels;
}

int KMeans15::predict(const QVector<double>& sample) const
{
    if (m_centroids.isEmpty()) return -1;
    return assignCluster(sample);
}

QVector<QVector<double>> KMeans15::centroids() const { return m_centroids; }

double KMeans15::inertia(const QVector<QVector<double>>& data) const
{
    double sum = 0.0;
    for (const auto& s : data)
        sum += distSq(s, m_centroids[assignCluster(s)]);
    return sum;
}

void KMeans15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
