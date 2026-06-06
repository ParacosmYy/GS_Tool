/**
 * @file FuzzyCMeans6.cpp
 * @brief FuzzyCMeans6 实现
 *
 * 实现模糊C均值聚类：标准FCM迭代、Gustafson-Kessel协方差扩展、聚类有效性指标。
 */

#include "utils/cluster165/FuzzyCMeans6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

FuzzyCMeans6::FuzzyCMeans6(QObject* parent)
    : QObject(parent)
{
}

FuzzyCMeans6::~FuzzyCMeans6() = default;

void FuzzyCMeans6::setFuzziness(double m)
{
    m_fuzziness = qMax(1.1, m);
}

void FuzzyCMeans6::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

void FuzzyCMeans6::setTolerance(double eps)
{
    m_tolerance = qMax(1e-12, eps);
}

void FuzzyCMeans6::initMembership(int n, int c)
{
    m_U.resize(c);
    for (int i = 0; i < c; ++i) {
        m_U[i].resize(n);
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            m_U[i][j] = static_cast<double>(qrand()) / RAND_MAX + 0.01;
            sum += m_U[i][j];
        }
        /* Normalize each column so sum over clusters = 1 */
        for (int j = 0; j < n; ++j)
            m_U[i][j] /= sum;
    }
}

void FuzzyCMeans6::updateCentroids(const QVector<QVector<double>>& data)
{
    int c = m_U.size();
    int n = data.size();
    if (n == 0) return;
    int dim = data[0].size();

    m_centers.resize(c);
    double power = m_fuzziness;
    for (int i = 0; i < c; ++i) {
        m_centers[i].assign(dim, 0.0);
        double denom = 0.0;
        for (int j = 0; j < n; ++j) {
            double w = qPow(m_U[i][j], power);
            denom += w;
            for (int d = 0; d < dim; ++d)
                m_centers[i][d] += w * data[j][d];
        }
        if (denom > 1e-15)
            for (int d = 0; d < dim; ++d)
                m_centers[i][d] /= denom;
    }
}

double FuzzyCMeans6::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

void FuzzyCMeans6::updateMembership(const QVector<QVector<double>>& data)
{
    int c = m_centers.size();
    int n = data.size();
    double power = 2.0 / (m_fuzziness - 1.0);

    for (int j = 0; j < n; ++j) {
        double sumInv = 0.0;
        QVector<double> dists(c);
        for (int i = 0; i < c; ++i) {
            dists[i] = qMax(1e-15, euclidean(data[j], m_centers[i]));
            sumInv += 1.0 / qPow(dists[i], power);
        }
        for (int i = 0; i < c; ++i)
            m_U[i][j] = 1.0 / (qPow(dists[i], power) * sumInv);
    }
}

void FuzzyCMeans6::updateMembershipGK(const QVector<QVector<double>>& data)
{
    int c = m_centers.size();
    int n = data.size();
    int dim = (data.isEmpty() ? 0 : data[0].size());
    double power = 2.0 / (m_fuzziness - 1.0);

    /* Compute fuzzy covariance matrices and GK distances */
    for (int j = 0; j < n; ++j) {
        QVector<double> dists(c);
        for (int i = 0; i < c; ++i) {
            /* Approximate GK distance using diagonal covariance */
            double det = 1e-6;
            QVector<double> covDiag(dim, 0.0);
            double uSum = 0.0;
            for (int k = 0; k < n; ++k) {
                double w = qPow(m_U[i][k], m_fuzziness);
                uSum += w;
                for (int d = 0; d < dim; ++d) {
                    double diff = data[k][d] - m_centers[i][d];
                    covDiag[d] += w * diff * diff;
                }
            }
            if (uSum > 1e-15) {
                for (int d = 0; d < dim; ++d) {
                    covDiag[d] /= uSum;
                    det *= qMax(covDiag[d], 1e-10);
                }
            }
            /* Mahalanobis-like distance */
            double mahalDist = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = data[j][d] - m_centers[i][d];
                mahalDist += diff * diff / qMax(covDiag[d], 1e-10);
            }
            /* Normalize by det^(1/dim) */
            double detNorm = qPow(qMax(det, 1e-30), 1.0 / qMax(dim, 1));
            dists[i] = qMax(1e-15, qSqrt(mahalDist * detNorm));
        }

        double sumInv = 0.0;
        for (int i = 0; i < c; ++i)
            sumInv += 1.0 / qPow(dists[i], power);
        for (int i = 0; i < c; ++i)
            m_U[i][j] = 1.0 / (qPow(dists[i], power) * sumInv);
    }
}

double FuzzyCMeans6::membershipChange() const
{
    if (m_U.isEmpty()) return 0.0;
    int c = m_U.size();
    int n = m_U[0].size();
    double maxChange = 0.0;
    /* Compare with previous state not stored; return norm estimate */
    for (int i = 0; i < c; ++i)
        for (int j = 0; j < n; ++j)
            maxChange += m_U[i][j] * m_U[i][j];
    return maxChange;
}

QVector<int> FuzzyCMeans6::fit(const QVector<QVector<double>>& data, int c)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || c <= 0) return QVector<int>();
    c = qMin(c, n);
    m_data = data;

    initMembership(n, c);

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> prevU = m_U;
        updateCentroids(data);
        updateMembership(data);
        /* Check convergence */
        double change = 0.0;
        for (int i = 0; i < c; ++i)
            for (int j = 0; j < n; ++j)
                change += qAbs(m_U[i][j] - prevU[i][j]);
        if (change < m_tolerance * n) break;
    }

    /* Assign hard labels */
    QVector<int> labels(n);
    for (int j = 0; j < n; ++j) {
        int best = 0;
        double bestU = m_U[0][j];
        for (int i = 1; i < c; ++i) {
            if (m_U[i][j] > bestU) { bestU = m_U[i][j]; best = i; }
        }
        labels[j] = best;
    }

    m_stats.totalRuns++;
    m_stats.lastClusterCount = c;
    m_stats.lastIterations = iter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0) ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(c, iter);
    return labels;
}

QVector<int> FuzzyCMeans6::fitGustafsonKessel(const QVector<QVector<double>>& data, int c)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || c <= 0) return QVector<int>();
    c = qMin(c, n);
    m_data = data;

    initMembership(n, c);

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> prevU = m_U;
        updateCentroids(data);
        updateMembershipGK(data);
        double change = 0.0;
        for (int i = 0; i < c; ++i)
            for (int j = 0; j < n; ++j)
                change += qAbs(m_U[i][j] - prevU[i][j]);
        if (change < m_tolerance * n) break;
    }

    QVector<int> labels(n);
    for (int j = 0; j < n; ++j) {
        int best = 0;
        double bestU = m_U[0][j];
        for (int i = 1; i < c; ++i) {
            if (m_U[i][j] > bestU) { bestU = m_U[i][j]; best = i; }
        }
        labels[j] = best;
    }

    m_stats.totalRuns++;
    m_stats.lastClusterCount = c;
    m_stats.lastIterations = iter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0) ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(c, iter);
    return labels;
}

QVector<QVector<double>> FuzzyCMeans6::membershipMatrix() const { return m_U; }
QVector<QVector<double>> FuzzyCMeans6::centroids() const { return m_centers; }

FuzzyCMeans6::ValidityIndices FuzzyCMeans6::computeValidity() const
{
    ValidityIndices vi;
    if (m_U.isEmpty() || m_data.isEmpty()) return vi;
    int c = m_U.size();
    int n = m_U[0].size();

    /* Partition Coefficient: sum(u_ij^2) / n */
    double pc = 0.0;
    for (int i = 0; i < c; ++i)
        for (int j = 0; j < n; ++j)
            pc += m_U[i][j] * m_U[i][j];
    vi.partitionCoeff = pc / n;

    /* Partition Entropy: -sum(u_ij * ln(u_ij)) / n */
    double pe = 0.0;
    for (int i = 0; i < c; ++i)
        for (int j = 0; j < n; ++j) {
            double u = m_U[i][j];
            if (u > 1e-15) pe -= u * qLn(u);
        }
    vi.partitionEntropy = pe / n;

    /* Xie-Beni Index */
    double numerator = 0.0;
    double minCenterDist = std::numeric_limits<double>::max();
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < c; ++i) {
            double d = euclidean(m_data[j], m_centers[i]);
            numerator += qPow(m_U[i][j], m_fuzziness) * d * d;
        }
    for (int i = 0; i < c; ++i)
        for (int k = i + 1; k < c; ++k)
            minCenterDist = qMin(minCenterDist,
                qPow(euclidean(m_centers[i], m_centers[k]), 2));
    vi.xieBeni = (minCenterDist > 1e-15) ? numerator / (n * minCenterDist) : 0.0;

    return vi;
}

void FuzzyCMeans6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
