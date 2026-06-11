/**
 * @file KMeans32.cpp
 * @brief KMeans32 实现
 *
 * 实现K均值聚类：小批量随机梯度与余弦相似度实现球面高维空间聚类分配。
 */

#include "utils/cluster297/KMeans32.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

KMeans32::KMeans32(QObject *parent)
    : QObject(parent) {}

KMeans32::~KMeans32() = default;

/* ---- Configuration ---- */

void KMeans32::setNumClusters(int k) { m_k = qBound(1, k, 1024); }
void KMeans32::setMaxIterations(int maxIter) { m_maxIter = qBound(1, maxIter, 10000); }
void KMeans32::setBatchSize(int size) { m_batchSize = qBound(8, size, 65536); }
void KMeans32::setConvergenceTolerance(double tol) { m_tol = qBound(1e-12, tol, 1.0); }

/* ---- Cosine similarity ---- */

double KMeans32::cosineSimilarity(const QVector<double>& a, const QVector<double>& b) const
{
    int d = qMin(a.size(), b.size());
    double dot = 0.0, na = 0.0, nb = 0.0;
    for (int i = 0; i < d; ++i) {
        dot += a[i] * b[i];
        na += a[i] * a[i];
        nb += b[i] * b[i];
    }
    double denom = qSqrt(qMax(1e-300, na)) * qSqrt(qMax(1e-300, nb));
    return dot / denom;
}

/* ---- L2 normalize ---- */

void KMeans32::normalize(QVector<double>& v) const
{
    double norm = 0.0;
    for (double x : v) norm += x * x;
    norm = qSqrt(qMax(1e-300, norm));
    for (auto& x : v) x /= norm;
}

/* ---- Initialize centroids via k-means++ ---- */

void KMeans32::initializeCentroids(const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_centroids.resize(m_k);

    // Pick first centroid deterministically (index 0)
    m_centroids[0].coordinates = data[0];
    normalize(m_centroids[0].coordinates);
    m_centroids[0].assignmentCount = 0;
    m_centroids[0].inertia = 0.0;

    QVector<double> minDist(n, 1e300);

    for (int c = 1; c < m_k; ++c) {
        // Update minimum distances to existing centroids
        for (int i = 0; i < n; ++i) {
            double sim = cosineSimilarity(data[i], m_centroids[c - 1].coordinates);
            double dist = 1.0 - sim;
            minDist[i] = qMin(minDist[i], dist * dist);
        }

        // Weighted deterministic selection
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) totalDist += minDist[i];
        double threshold = totalDist * ((c * 7919 + 13) % 997) / 997.0;
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }

        m_centroids[c].coordinates = data[chosen];
        normalize(m_centroids[c].coordinates);
        m_centroids[c].assignmentCount = 0;
        m_centroids[c].inertia = 0.0;
    }
}

/* ---- Select mini-batch indices ---- */

QVector<int> KMeans32::selectBatch(int totalSize, int batchSize, int iteration) const
{
    int effective = qMin(batchSize, totalSize);
    QVector<int> indices;
    indices.reserve(effective);
    // Deterministic stride-based batch selection
    int start = (iteration * effective * 7 + 3) % totalSize;
    for (int i = 0; i < effective; ++i)
        indices.append((start + i) % totalSize);
    return indices;
}

/* ---- Find nearest centroid by cosine similarity ---- */

int KMeans32::nearestCentroid(const QVector<double>& point) const
{
    int best = 0;
    double bestSim = cosineSimilarity(point, m_centroids[0].coordinates);
    for (int c = 1; c < m_k; ++c) {
        double sim = cosineSimilarity(point, m_centroids[c].coordinates);
        if (sim > bestSim) { bestSim = sim; best = c; }
    }
    return best;
}

/* ---- Compute total inertia ---- */

double KMeans32::computeInertia(const QVector<QVector<double>>& data,
                                 const QVector<int>& assignments) const
{
    double inertia = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        int c = assignments[i];
        double sim = cosineSimilarity(data[i], m_centroids[c].coordinates);
        inertia += 1.0 - sim;
    }
    return inertia;
}

/* ---- Main fit with mini-batch SGD ---- */

KMeans32::ClusterResult KMeans32::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n < m_k) return result;

    m_dims = data[0].size();
    // Normalize all data points for spherical clustering
    QVector<QVector<double>> normed(n);
    for (int i = 0; i < n; ++i) {
        normed[i] = data[i];
        normalize(normed[i]);
    }

    initializeCentroids(normed);

    result.assignments.resize(n, 0);
    double prevInertia = 1e300;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Select mini-batch
        auto batch = selectBatch(n, m_batchSize, iter);

        // Assign batch points and update centroids (SGD step)
        for (int idx : batch) {
            int c = nearestCentroid(normed[idx]);
            result.assignments[idx] = c;
            m_centroids[c].assignmentCount++;

            // Streaming centroid update
            double lr = 1.0 / m_centroids[c].assignmentCount;
            int d = m_dims;
            for (int j = 0; j < d; ++j)
                m_centroids[c].coordinates[j] += lr * (normed[idx][j] - m_centroids[c].coordinates[j]);

            // Re-normalize centroid to unit sphere
            normalize(m_centroids[c].coordinates);
        }

        // Full assignment pass every 10 iterations for convergence check
        if (iter % 10 == 0 || iter == m_maxIter - 1) {
            for (int i = 0; i < n; ++i)
                result.assignments[i] = nearestCentroid(normed[i]);

            double curInertia = computeInertia(normed, result.assignments);
            if (qAbs(prevInertia - curInertia) < m_tol && iter > 0) {
                result.converged = true;
                result.iterations = iter + 1;
                result.totalInertia = curInertia;
                break;
            }
            prevInertia = curInertia;
            result.totalInertia = curInertia;
            result.iterations = iter + 1;
        }
    }

    result.centroids = m_centroids;
    m_stats.totalFits++;
    m_stats.numClusters = m_k;
    m_stats.dimensions = m_dims;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitDone(m_k, result.iterations, result.totalInertia, elapsed);
    return result;
}

/* ---- Predict ---- */

QVector<int> KMeans32::predict(const QVector<QVector<double>>& points) const
{
    QVector<int> result;
    result.reserve(points.size());
    for (const auto& p : points) {
        QVector<double> normed = p;
        const_cast<KMeans32*>(this)->normalize(normed);
        result.append(nearestCentroid(normed));
    }
    return result;
}

/* ---- Reset ---- */

void KMeans32::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
}
