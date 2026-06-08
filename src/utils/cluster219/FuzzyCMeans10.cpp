/**
 * @file FuzzyCMeans10.cpp
 * @brief FuzzyCMeans10 实现
 *
 * 实现模糊C均值聚类：可能性隶属度、噪声簇自适应距离原型。
 */

#include "utils/cluster219/FuzzyCMeans10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FuzzyCMeans10::FuzzyCMeans10(QObject *parent) : QObject(parent) {}
FuzzyCMeans10::~FuzzyCMeans10() = default;

/* ---- Configuration ---- */

void FuzzyCMeans10::setParameters(int clusters, double fuzziness,
                                    double noiseDelta, int maxIter)
{
    m_clusters = qMax(2, clusters);
    m_fuzziness = qMax(1.1, fuzziness);
    m_noiseDelta = qMax(0.01, noiseDelta);
    m_maxIter = qMax(10, maxIter);
}

/* ---- Distance ---- */

double FuzzyCMeans10::euclidean(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Initialize membership randomly ---- */

void FuzzyCMeans10::initMembership(int n)
{
    m_membership.resize(n);
    for (int i = 0; i < n; ++i) {
        m_membership[i].resize(m_clusters + 1); // +1 for noise cluster
        double sum = 0.0;
        for (int c = 0; c <= m_clusters; ++c) {
            m_membership[i][c] = 0.01 + qrand() % 100;
            sum += m_membership[i][c];
        }
        for (int c = 0; c <= m_clusters; ++c)
            m_membership[i][c] /= sum;
    }
}

/* ---- Update centroids ---- */

void FuzzyCMeans10::updateCentroids(const QVector<QVector<double>>& data)
{
    double mp = m_fuzziness;
    m_centroids.resize(m_clusters);

    for (int c = 0; c < m_clusters; ++c) {
        QVector<double> cent(m_dim, 0.0);
        double denom = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            double w = qPow(m_membership[i][c], mp);
            denom += w;
            for (int d = 0; d < m_dim; ++d)
                cent[d] += w * data[i][d];
        }
        if (denom > 1e-12) {
            for (int d = 0; d < m_dim; ++d)
                cent[d] /= denom;
        }
        m_centroids[c] = cent;
    }
}

/* ---- Update membership with possibilistic + noise ---- */

void FuzzyCMeans10::updateMembership(const QVector<QVector<double>>& data)
{
    double mp = m_fuzziness;
    double delta2 = m_noiseDelta * m_noiseDelta;

    for (int i = 0; i < data.size(); ++i) {
        // Compute distances to all cluster centroids
        QVector<double> dists(m_clusters);
        for (int c = 0; c < m_clusters; ++c)
            dists[c] = euclidean(data[i], m_centroids[c]);

        // Standard fuzzy membership update
        for (int c = 0; c < m_clusters; ++c) {
            if (dists[c] < 1e-12) {
                for (int k = 0; k <= m_clusters; ++k)
                    m_membership[i][k] = (k == c) ? 1.0 : 0.0;
                break;
            }
            double sumInv = 0.0;
            for (int k = 0; k < m_clusters; ++k) {
                double ratio = dists[c] / qMax(dists[k], 1e-12);
                sumInv += qPow(ratio, 2.0 / (mp - 1.0));
            }
            m_membership[i][c] = 1.0 / qMax(sumInv, 1e-12);
        }

        // Noise cluster membership: noise weight = 1 - sum(real memberships)
        double sumReal = 0.0;
        for (int c = 0; c < m_clusters; ++c)
            sumReal += m_membership[i][c];

        // Possibilistic relaxation: allow sum != 1
        // Noise weight inversely proportional to delta^2
        double noiseDist = adaptiveNoiseDistance(data);
        m_membership[i][m_clusters] = (noiseDist > 0)
            ? 1.0 / (1.0 + qPow(dists[0] / (noiseDist * delta2), 2.0 / (mp - 1.0)))
            : 0.0;

        // Re-normalize with noise cluster
        double total = sumReal + m_membership[i][m_clusters];
        if (total > 1e-12) {
            for (int c = 0; c <= m_clusters; ++c)
                m_membership[i][c] /= total;
        }
    }
}

/* ---- Adaptive noise distance ---- */

double FuzzyCMeans10::adaptiveNoiseDistance(
    const QVector<QVector<double>>& data) const
{
    if (m_centroids.isEmpty() || data.isEmpty()) return m_noiseDelta;

    double avgDist = 0.0;
    int count = 0;
    for (int i = 0; i < qMin(data.size(), 100); ++i) {
        double minD = std::numeric_limits<double>::max();
        for (int c = 0; c < m_clusters; ++c)
            minD = qMin(minD, euclidean(data[i], m_centroids[c]));
        avgDist += minD;
        count++;
    }
    return (count > 0) ? avgDist / count : m_noiseDelta;
}

/* ---- Objective function ---- */

double FuzzyCMeans10::computeObjective(
    const QVector<QVector<double>>& data) const
{
    double obj = 0.0;
    double mp = m_fuzziness;
    for (int i = 0; i < data.size(); ++i) {
        for (int c = 0; c < m_clusters; ++c) {
            double d = euclidean(data[i], m_centroids[c]);
            obj += qPow(m_membership[i][c], mp) * d * d;
        }
        // Noise cluster penalty
        double d2 = m_noiseDelta * m_noiseDelta;
        obj += qPow(m_membership[i][m_clusters], mp) * d2;
    }
    return obj;
}

/* ---- Fit ---- */

FuzzyCMeans10::ClusterResult FuzzyCMeans10::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return ClusterResult();

    int n = data.size();
    m_dim = data[0].size();
    m_stats.numPoints = n;
    m_stats.numClusters = m_clusters;
    m_stats.dim = m_dim;

    initMembership(n);

    double prevObj = std::numeric_limits<double>::max();
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        updateCentroids(data);
        updateMembership(data);
        double obj = computeObjective(data);
        if (qAbs(prevObj - obj) < 1e-6) break;
        prevObj = obj;
    }

    ClusterResult result;
    result.centroids = m_centroids;
    result.membership = m_membership;
    result.noiseWeight.resize(n);
    for (int i = 0; i < n; ++i)
        result.noiseWeight[i] = m_membership[i][m_clusters];
    result.objective = prevObj;
    result.iterations = iter;

    m_stats.totalIterations += iter;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_clusters, iter, timer.elapsed());
    return result;
}

/* ---- Predict ---- */

QVector<int> FuzzyCMeans10::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size(), -1);
    for (int i = 0; i < data.size(); ++i) {
        double bestD = std::numeric_limits<double>::max();
        for (int c = 0; c < m_centroids.size(); ++c) {
            double d = euclidean(data[i], m_centroids[c]);
            if (d < bestD) { bestD = d; labels[i] = c; }
        }
    }
    return labels;
}

/* ---- Possibilistic membership ---- */

QVector<QVector<double>> FuzzyCMeans10::computePossibilistic(
    const QVector<QVector<double>>& data) const
{
    QVector<QVector<double>> pm(data.size());
    double mp = m_fuzziness;
    for (int i = 0; i < data.size(); ++i) {
        pm[i].resize(m_clusters);
        for (int c = 0; c < m_clusters; ++c) {
            double d = euclidean(data[i], m_centroids[c]);
            double eta = adaptiveNoiseDistance(data);
            pm[i][c] = 1.0 / (1.0 + qPow(d / qMax(eta, 1e-12), 2.0 / (mp - 1.0)));
        }
    }
    return pm;
}

/* ---- Centroids ---- */

QVector<QVector<double>> FuzzyCMeans10::centroids() const
{
    return m_centroids;
}

/* ---- Reset ---- */

void FuzzyCMeans10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
    m_membership.clear();
    m_dim = 0;
}
