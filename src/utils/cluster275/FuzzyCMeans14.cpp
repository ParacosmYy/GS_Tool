/**
 * @file FuzzyCMeans14.cpp
 * @brief FuzzyCMeans14 实现
 *
 * 实现模糊C均值聚类：熵正则化与可能性隶属度的噪声鲁棒模糊划分。
 */

#include "utils/cluster275/FuzzyCMeans14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FuzzyCMeans14::FuzzyCMeans14(QObject *parent)
    : QObject(parent) {}

FuzzyCMeans14::~FuzzyCMeans14() = default;

/* ---- Configuration ---- */

void FuzzyCMeans14::setFuzziness(double m) { m_fuzziness = qBound(1.01, m, 10.0); }
void FuzzyCMeans14::setEntropyWeight(double alpha) { m_entropyWeight = qBound(0.0, alpha, 10.0); }
void FuzzyCMeans14::setPossibilisticScale(double eta) { m_possScale = qBound(0.01, eta, 1e6); }
void FuzzyCMeans14::setTolerance(double tol) { m_tolerance = qBound(1e-10, tol, 1.0); }
void FuzzyCMeans14::setMaxIterations(int iter) { m_maxIter = qBound(1, iter, 10000); }

/* ---- Distance helper ---- */

double FuzzyCMeans14::distSq(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

/* ---- Initialize membership matrix randomly ---- */

QVector<QVector<double>> FuzzyCMeans14::initMembership(int n, int k) const
{
    QVector<QVector<double>> U(n);
    for (int i = 0; i < n; ++i) {
        U[i].resize(k);
        double sum = 0.0;
        for (int j = 0; j < k; ++j) {
            U[i][j] = static_cast<double>(qrand()) / RAND_MAX + 0.01;
            sum += U[i][j];
        }
        for (int j = 0; j < k; ++j)
            U[i][j] /= sum;
    }
    return U;
}

/* ---- Update centroids ---- */

QVector<QVector<double>> FuzzyCMeans14::updateCentroids(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& U) const
{
    int n = data.size();
    int k = U.isEmpty() ? 0 : U[0].size();
    int dim = data.isEmpty() ? 0 : data[0].size();

    QVector<QVector<double>> centroids(k, QVector<double>(dim, 0.0));
    QVector<double> denom(k, 0.0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            double w = qPow(U[i][j], m_fuzziness);
            denom[j] += w;
            for (int d = 0; d < dim; ++d)
                centroids[j][d] += w * data[i][d];
        }
    }
    for (int j = 0; j < k; ++j) {
        if (denom[j] > 1e-15) {
            for (int d = 0; d < dim; ++d)
                centroids[j][d] /= denom[j];
        }
    }
    return centroids;
}

/* ---- Update membership with entropy regularization + possibilistic ---- */

QVector<QVector<double>> FuzzyCMeans14::updateMembership(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& centroids) const
{
    int n = data.size();
    int k = centroids.size();
    double m = m_fuzziness;
    double exp = 1.0 / (m - 1.0);

    QVector<QVector<double>> U(n, QVector<double>(k, 0.0));

    for (int i = 0; i < n; ++i) {
        // Compute inverse distance powers
        QVector<double> invDPow(k);
        double sumInv = 0.0;
        bool zeroDist = false;
        int zeroIdx = -1;

        for (int j = 0; j < k; ++j) {
            double dSq = distSq(data[i], centroids[j]);
            if (dSq < 1e-15) { zeroDist = true; zeroIdx = j; break; }
            invDPow[j] = qPow(dSq, -exp);
            sumInv += invDPow[j];
        }

        if (zeroDist) {
            for (int j = 0; j < k; ++j) U[i][j] = (j == zeroIdx) ? 1.0 : 0.0;
        } else {
            // Standard fuzzy membership
            for (int j = 0; j < k; ++j)
                U[i][j] = invDPow[j] / sumInv;
        }

        // Entropy regularization: soften membership
        if (m_entropyWeight > 0.0) {
            double sumE = 0.0;
            for (int j = 0; j < k; ++j) {
                U[i][j] += m_entropyWeight * U[i][j] * (1.0 - U[i][j]);
                sumE += U[i][j];
            }
            if (sumE > 1e-15) {
                for (int j = 0; j < k; ++j) U[i][j] /= sumE;
            }
        }

        // Possibilistic correction: allow memberships < 1/k for outliers
        for (int j = 0; j < k; ++j) {
            double dSq = distSq(data[i], centroids[j]);
            double possTerm = 1.0 / (1.0 + qPow(dSq / m_possScale, exp));
            U[i][j] = 0.7 * U[i][j] + 0.3 * possTerm;
        }

        // Re-normalize
        double sum = 0.0;
        for (int j = 0; j < k; ++j) sum += U[i][j];
        if (sum > 1e-15)
            for (int j = 0; j < k; ++j) U[i][j] /= sum;
    }
    return U;
}

/* ---- Compute objective function ---- */

double FuzzyCMeans14::computeObjective(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& centroids,
    const QVector<QVector<double>>& U) const
{
    int n = data.size();
    int k = centroids.size();
    double obj = 0.0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            double w = qPow(U[i][j], m_fuzziness);
            obj += w * distSq(data[i], centroids[j]);
        }
        // Entropy term: -alpha * sum(u * log(u))
        if (m_entropyWeight > 0.0) {
            for (int j = 0; j < k; ++j) {
                if (U[i][j] > 1e-15)
                    obj -= m_entropyWeight * U[i][j] * qLn(U[i][j]);
            }
        }
    }
    return obj;
}

/* ---- Fit: main clustering loop ---- */

FuzzyCMeans14::ClusterResult FuzzyCMeans14::fit(
    const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0 || k <= 0) return result;
    k = qBound(1, k, n);

    auto U = initMembership(n, k);
    QVector<QVector<double>> centroids;

    double prevObj = 1e18;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        centroids = updateCentroids(data, U);
        U = updateMembership(data, centroids);

        double obj = computeObjective(data, centroids, U);
        emit iterationUpdate(iter, obj);

        if (qAbs(obj - prevObj) < m_tolerance) {
            result.converged = true;
            result.iterations = iter + 1;
            result.objectiveValue = obj;
            break;
        }
        prevObj = obj;
        result.iterations = iter + 1;
        result.objectiveValue = obj;
    }

    result.centroids = centroids;
    result.membership = U;

    // Hard labels from max membership
    result.labels.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        double maxU = U[i][0];
        for (int j = 1; j < k; ++j) {
            if (U[i][j] > maxU) { maxU = U[i][j]; best = j; }
        }
        result.labels[i] = best;
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = k;
    m_stats.numIterations += result.iterations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringDone(k, result.iterations, result.objectiveValue, elapsed);

    return result;
}

/* ---- Predict membership for new points ---- */

QVector<QVector<double>> FuzzyCMeans14::predict(
    const QVector<QVector<double>>& points,
    const QVector<QVector<double>>& centroids) const
{
    int n = points.size();
    int k = centroids.size();
    double exp = 1.0 / (m_fuzziness - 1.0);

    QVector<QVector<double>> U(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i) {
        double sumInv = 0.0;
        QVector<double> invDPow(k);
        for (int j = 0; j < k; ++j) {
            double dSq = distSq(points[i], centroids[j]);
            invDPow[j] = (dSq > 1e-15) ? qPow(dSq, -exp) : 1e15;
            sumInv += invDPow[j];
        }
        for (int j = 0; j < k; ++j)
            U[i][j] = invDPow[j] / sumInv;
    }
    return U;
}

/* ---- Partition coefficient ---- */

double FuzzyCMeans14::partitionCoefficient(const QVector<QVector<double>>& membership) const
{
    double pc = 0.0;
    int n = membership.size();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < membership[i].size(); ++j)
            pc += membership[i][j] * membership[i][j];
    return (n > 0) ? pc / n : 0.0;
}

/* ---- Xie-Beni index ---- */

double FuzzyCMeans14::xieBeniIndex(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& centroids,
    const QVector<QVector<double>>& membership) const
{
    int n = data.size();
    int k = centroids.size();
    if (n == 0 || k == 0) return 0.0;

    double num = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            num += qPow(membership[i][j], m_fuzziness) * distSq(data[i], centroids[j]);

    // Minimum inter-centroid distance
    double minSep = 1e18;
    for (int i = 0; i < k; ++i)
        for (int j = i + 1; j < k; ++j)
            minSep = qMin(minSep, distSq(centroids[i], centroids[j]));

    return (minSep > 1e-15) ? num / (n * minSep) : 1e18;
}

/* ---- Reset ---- */

void FuzzyCMeans14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
