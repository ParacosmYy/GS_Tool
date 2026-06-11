/**
 * @file FuzzyCMeans15.cpp
 * @brief FuzzyCMeans15 实现
 *
 * 实现模糊C均值聚类：可能性隶属度与簇有效性指数引导的自适应指数调整。
 */

#include "utils/cluster289/FuzzyCMeans15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FuzzyCMeans15::FuzzyCMeans15(QObject *parent)
    : QObject(parent) {}

FuzzyCMeans15::~FuzzyCMeans15() = default;

/* ---- Configuration ---- */

void FuzzyCMeans15::setNumClusters(int k) { m_k = qBound(2, k, 500); }
void FuzzyCMeans15::setFuzziness(double m) { m_m = qBound(1.1, m, 10.0); }
void FuzzyCMeans15::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 10000); }
void FuzzyCMeans15::setConvergenceThreshold(double eps) { m_eps = qBound(1e-10, eps, 1.0); }
void FuzzyCMeans15::setAutoTuneM(bool enabled) { m_autoTuneM = enabled; }

/* ---- Squared Euclidean distance ---- */

double FuzzyCMeans15::squaredDist(const QVector<double>& a,
                                    const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}

/* ---- Initialize membership matrix randomly ---- */

QVector<QVector<double>> FuzzyCMeans15::initMembership(int n) const
{
    QVector<QVector<double>> U(n);
    for (int i = 0; i < n; ++i) {
        U[i].resize(m_k);
        double sum = 0.0;
        for (int j = 0; j < m_k; ++j) {
            U[i][j] = static_cast<double>(qrand()) / RAND_MAX + 0.01;
            sum += U[i][j];
        }
        for (int j = 0; j < m_k; ++j)
            U[i][j] /= sum;
    }
    return U;
}

/* ---- Update centroids ---- */

QVector<QVector<double>> FuzzyCMeans15::updateCentroids(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& U) const
{
    int d = m_dims;
    QVector<QVector<double>> centroids(m_k, QVector<double>(d, 0.0));

    for (int j = 0; j < m_k; ++j) {
        double numSum = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            double w = qPow(U[i][j], m_m);
            numSum += w;
            for (int dd = 0; dd < d; ++dd)
                centroids[j][dd] += w * data[i][dd];
        }
        if (numSum > 1e-15)
            for (int dd = 0; dd < d; ++dd)
                centroids[j][dd] /= numSum;
    }
    return centroids;
}

/* ---- Update membership matrix ---- */

QVector<QVector<double>> FuzzyCMeans15::updateMembership(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& centroids) const
{
    int n = data.size();
    double exp = 1.0 / (m_m - 1.0);
    QVector<QVector<double>> U(n, QVector<double>(m_k, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m_k; ++j) {
            double dij = squaredDist(data[i], centroids[j]);
            if (dij < 1e-15) dij = 1e-15;
            double sum = 0.0;
            for (int jj = 0; jj < m_k; ++jj) {
                double dijj = squaredDist(data[i], centroids[jj]);
                if (dijj < 1e-15) dijj = 1e-15;
                sum += qPow(dij / dijj, exp);
            }
            U[i][j] = 1.0 / sum;
        }
    }
    return U;
}

/* ---- Compute Jm objective ---- */

double FuzzyCMeans15::computeObjective(const QVector<QVector<double>>& data,
                                         const QVector<QVector<double>>& U,
                                         const QVector<QVector<double>>& centroids) const
{
    double Jm = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        for (int j = 0; j < m_k; ++j) {
            double w = qPow(U[i][j], m_m);
            Jm += w * squaredDist(data[i], centroids[j]);
        }
    }
    return Jm;
}

/* ---- Auto-tune fuzziness exponent ---- */

double FuzzyCMeans15::autoTuneExponent(const QVector<QVector<double>>& data,
                                         int k) const
{
    // Search for best m in [1.1, 5.0] by maximizing partition coefficient
    double bestM = 2.0;
    double bestPC = -1.0;
    int n = data.size();

    for (double m = 1.2; m <= 5.0; m += 0.3) {
        // Quick evaluation with 5 iterations
        double savedM = m_m;
        m_m = m;
        QVector<QVector<double>> U = initMembership(n);
        auto C = updateCentroids(data, U);
        for (int iter = 0; iter < 5; ++iter) {
            U = updateMembership(data, C);
            C = updateCentroids(data, U);
        }
        m_m = savedM;

        // Partition coefficient: sum(U[i][j]^2) / n
        double pc = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < k; ++j)
                pc += U[i][j] * U[i][j];
        pc /= n;

        if (pc > bestPC) { bestPC = pc; bestM = m; }
    }
    return bestM;
}

/* ---- Compute validity indices ---- */

FuzzyCMeans15::ValidityIndex FuzzyCMeans15::evaluateValidity(
    const QVector<QVector<double>>& data,
    const ClusterResult& result) const
{
    ValidityIndex vi;
    int n = data.size();
    int k = result.numClusters;
    if (n == 0 || k == 0) return vi;

    // Partition Coefficient
    double pcSum = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            pcSum += result.membership[i][j] * result.membership[i][j];
    vi.partitionCoefficient = pcSum / n;

    // Xie-Beni index: Jm / (n * min_dist^2 between centroids)
    double minInterDist = 1e300;
    for (int i = 0; i < k; ++i)
        for (int j = i + 1; j < k; ++j) {
            double d = squaredDist(result.centroids[i], result.centroids[j]);
            if (d < minInterDist) minInterDist = d;
        }
    if (minInterDist < 1e-15) minInterDist = 1e-15;
    vi.xieBeni = result.objectiveValue / (n * minInterDist);
    vi.optimalM = m_m;

    return vi;
}

/* ---- Possibilistic membership ---- */

QVector<QVector<double>> FuzzyCMeans15::computePossibilisticMembership(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& centroids) const
{
    int n = data.size();
    int k = centroids.size();
    // Estimate bandwidth parameters (eta_j) from fuzzy result
    QVector<double> eta(k, 0.0);
    QVector<int> counts(k, 0);
    for (int i = 0; i < n; ++i) {
        double bestD = 1e300;
        int bestJ = 0;
        for (int j = 0; j < k; ++j) {
            double d = squaredDist(data[i], centroids[j]);
            if (d < bestD) { bestD = d; bestJ = j; }
        }
        eta[bestJ] += bestD;
        counts[bestJ]++;
    }
    for (int j = 0; j < k; ++j)
        eta[j] = (counts[j] > 0) ? eta[j] / counts[j] : 1.0;

    // Compute typicality (possibilistic membership)
    QVector<QVector<double>> T(n, QVector<double>(k, 0.0));
    double exp = 1.0 / (m_m - 1.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            double d = squaredDist(data[i], centroids[j]);
            if (eta[j] < 1e-15) eta[j] = 1e-15;
            T[i][j] = 1.0 / (1.0 + qPow(d / eta[j], exp));
        }
    }
    return T;
}

/* ---- Main fit ---- */

FuzzyCMeans15::ClusterResult FuzzyCMeans15::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0) return result;
    m_dims = data[0].size();

    // Auto-tune fuzziness exponent
    if (m_autoTuneM)
        m_m = autoTuneExponent(data, m_k);

    // Initialize membership
    QVector<QVector<double>> U = initMembership(n);
    QVector<QVector<double>> C = updateCentroids(data, U);

    double prevObj = 1e300;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        U = updateMembership(data, C);
        C = updateCentroids(data, U);

        double obj = computeObjective(data, U, C);
        if (qAbs(prevObj - obj) < m_eps) break;
        prevObj = obj;
    }

    // Build result
    result.numClusters = m_k;
    result.membership = U;
    result.centroids = C;
    result.objectiveValue = prevObj;
    result.labels.resize(n);
    for (int i = 0; i < n; ++i) {
        double bestU = 0.0;
        int bestJ = 0;
        for (int j = 0; j < m_k; ++j) {
            if (U[i][j] > bestU) { bestU = U[i][j]; bestJ = j; }
        }
        result.labels[i] = bestJ;
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = m_k;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitDone(n, m_k, result.objectiveValue, elapsed);
    return result;
}

/* ---- Reset ---- */

void FuzzyCMeans15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_dims = 0;
}
