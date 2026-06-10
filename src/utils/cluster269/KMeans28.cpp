/**
 * @file KMeans28.cpp
 * @brief KMeans28 实现
 *
 * 实现K均值聚类：Elkan三角不等式加速与上下界剪枝距离计算优化。
 */

#include "utils/cluster269/KMeans28.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMeans28::KMeans28(QObject *parent)
    : QObject(parent) {}

KMeans28::~KMeans28() = default;

/* ---- Configuration ---- */

void KMeans28::setK(int k)
{
    m_k = qBound(2, k, 200);
}

void KMeans28::setMaxIterations(int iters)
{
    m_maxIter = qBound(10, iters, 2000);
}

void KMeans28::setTolerance(double tol)
{
    m_tol = qBound(1e-10, tol, 1.0);
}

/* ---- Euclidean distance squared ---- */

double KMeans28::distSq(const QVector<double>& a, const QVector<double>& b) const
{
    int d = qMin(a.size(), b.size());
    double sum = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- K-means++ seeding ---- */

void KMeans28::initCentroids(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;

    m_centroids.resize(m_k);
    // First centroid: pick index 0
    m_centroids[0] = data[0];

    QVector<double> minDist(n, 0.0);

    for (int c = 1; c < m_k; ++c) {
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = distSq(data[i], m_centroids[c - 1]);
            if (c == 1 || d < minDist[i])
                minDist[i] = d;
            totalDist += minDist[i];
        }
        // Weighted random selection
        double threshold = qrand() / static_cast<double>(RAND_MAX) * totalDist;
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_centroids[c] = data[chosen];
    }
}

/* ---- Half inter-centroid distances ---- */

QVector<double> KMeans28::computeHalfInterCentroid() const
{
    int k = m_centroids.size();
    QVector<double> s(k, std::numeric_limits<double>::max());

    for (int j = 0; j < k; ++j) {
        for (int l = 0; l < k; ++l) {
            if (j != l) {
                double d = qSqrt(distSq(m_centroids[j], m_centroids[l])) * 0.5;
                if (d < s[j]) s[j] = d;
            }
        }
    }
    return s;
}

/* ---- Elkan iteration with triangle inequality bounds ---- */

int KMeans28::elkanIterate(const QVector<QVector<double>>& data,
                            QVector<QVector<double>>& lowerBounds,
                            QVector<double>& upperBounds,
                            QVector<double>& sCache)
{
    int n = data.size();
    int k = m_centroids.size();
    int dim = (n > 0) ? data[0].size() : 0;
    int changes = 0;

    // Step 1: Compute s_cache (half nearest centroid distance)
    sCache = computeHalfInterCentroid();

    // Step 2: For each point, skip distance computation using bounds
    for (int i = 0; i < n; ++i) {
        // If upper bound <= s(x(i)), skip
        if (upperBounds[i] <= sCache[m_labels[i]])
            continue;

        double minDist = std::numeric_limits<double>::max();
        int bestLabel = m_labels[i];

        for (int j = 0; j < k; ++j) {
            // Elkan pruning: skip if lower bound >= upper bound
            if (j == m_labels[i]) continue;
            if (lowerBounds[i][j] >= upperBounds[i])
                continue;

            // Compute actual distance
            double d = qSqrt(distSq(data[i], m_centroids[j]));
            lowerBounds[i][j] = d;

            if (d < minDist) {
                minDist = d;
                bestLabel = j;
            }
        }

        // Also compute distance to current assignment
        double curDist = qSqrt(distSq(data[i], m_centroids[m_labels[i]]));
        if (curDist < minDist) {
            minDist = curDist;
            bestLabel = m_labels[i];
        }

        if (bestLabel != m_labels[i]) {
            m_labels[i] = bestLabel;
            changes++;
        }
        upperBounds[i] = minDist;
    }

    // Step 3: Update centroids
    QVector<QVector<double>> newCentroids(k, QVector<double>(dim, 0.0));
    QVector<int> counts(k, 0);

    for (int i = 0; i < n; ++i) {
        int c = m_labels[i];
        counts[c]++;
        for (int d = 0; d < dim; ++d)
            newCentroids[c][d] += data[i][d];
    }

    // Handle empty clusters by reinitializing
    for (int j = 0; j < k; ++j) {
        if (counts[j] == 0) {
            // Pick a random point as new centroid
            int ri = qrand() % n;
            newCentroids[j] = data[ri];
        } else {
            for (int d = 0; d < dim; ++d)
                newCentroids[j][d] /= counts[j];
        }
    }

    // Step 4: Update bounds based on centroid movement
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            double shift = qSqrt(distSq(m_centroids[j], newCentroids[j]));
            lowerBounds[i][j] = qMax(0.0, lowerBounds[i][j] - shift);
        }
        double myShift = qSqrt(distSq(m_centroids[m_labels[i]], newCentroids[m_labels[i]]));
        upperBounds[i] += myShift;
    }

    m_centroids = newCentroids;
    return changes;
}

/* ---- Main fit ---- */

QVector<QVector<double>> KMeans28::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_k) return {};

    int dim = data[0].size();
    initCentroids(data);

    // Initialize labels to nearest centroid (brute force)
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) {
        double best = std::numeric_limits<double>::max();
        for (int j = 0; j < m_k; ++j) {
            double d = distSq(data[i], m_centroids[j]);
            if (d < best) { best = d; m_labels[i] = j; }
        }
    }

    // Initialize Elkan bounds
    // lowerBounds[i][j] = lower bound on d(x_i, c_j)
    QVector<QVector<double>> lowerBounds(n, QVector<double>(m_k, 0.0));
    // upperBounds[i] = upper bound on d(x_i, c_{label[i]})
    QVector<double> upperBounds(n, 0.0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m_k; ++j) {
            lowerBounds[i][j] = qSqrt(distSq(data[i], m_centroids[j]));
        }
        upperBounds[i] = lowerBounds[i][m_labels[i]];
    }

    QVector<double> sCache;
    int iters = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        iters++;
        int changes = elkanIterate(data, lowerBounds, upperBounds, sCache);
        if (changes == 0) break;
    }

    // Compute inertia
    m_inertia = 0.0;
    for (int i = 0; i < n; ++i)
        m_inertia += distSq(data[i], m_centroids[m_labels[i]]);

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = m_k;
    m_stats.iterations = iters;
    m_stats.inertia = m_inertia;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringDone(m_k, iters, m_inertia, elapsed);

    return m_centroids;
}

/* ---- Predict ---- */

QVector<int> KMeans28::predict(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = m_centroids.size();
    QVector<int> result(n, 0);

    for (int i = 0; i < n; ++i) {
        double best = std::numeric_limits<double>::max();
        for (int j = 0; j < k; ++j) {
            double d = distSq(data[i], m_centroids[j]);
            if (d < best) { best = d; result[i] = j; }
        }
    }
    return result;
}

/* ---- Accessors ---- */

QVector<int> KMeans28::labels() const { return m_labels; }
QVector<QVector<double>> KMeans28::centroids() const { return m_centroids; }
double KMeans28::inertia() const { return m_inertia; }

/* ---- Reset ---- */

void KMeans28::resetStatistics()
{
    m_centroids.clear();
    m_labels.clear();
    m_inertia = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
