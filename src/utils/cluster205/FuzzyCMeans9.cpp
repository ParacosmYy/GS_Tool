/**
 * @file FuzzyCMeans9.cpp
 * @brief FuzzyCMeans9 实现
 *
 * 实现模糊C均值聚类：Gustafson-Kessel距离、自适应体积约束、模糊分区矩阵。
 */

#include "utils/cluster205/FuzzyCMeans9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FuzzyCMeans9::FuzzyCMeans9(QObject *parent) : QObject(parent) {}
FuzzyCMeans9::~FuzzyCMeans9() = default;

/* ---- Configuration ---- */

void FuzzyCMeans9::setClusterCount(int k) { m_k = qMax(2, k); }
void FuzzyCMeans9::setFuzziness(double m) { m_fuzziness = qMax(1.1, m); }
void FuzzyCMeans9::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void FuzzyCMeans9::setConvergenceThreshold(double eps) { m_eps = qMax(1e-10, eps); }

/* ---- Initialize membership matrix ---- */

void FuzzyCMeans9::initMembership(int n)
{
    m_U.resize(n);
    for (int i = 0; i < n; ++i) {
        m_U[i].resize(m_k, 0.0);
        double sum = 0.0;
        for (int j = 0; j < m_k; ++j) {
            m_U[i][j] = static_cast<double>(qrand()) / RAND_MAX + 0.01;
            sum += m_U[i][j];
        }
        for (int j = 0; j < m_k; ++j)
            m_U[i][j] /= sum;
    }
}

/* ---- Update cluster centers ---- */

void FuzzyCMeans9::updateCenters()
{
    int n = m_data.size();
    int d = m_dim;
    m_centers.resize(m_k);

    for (int j = 0; j < m_k; ++j) {
        m_centers[j].resize(d, 0.0);
        double sumUm = 0.0;
        for (int i = 0; i < n; ++i) {
            double um = qPow(m_U[i][j], m_fuzziness);
            sumUm += um;
            for (int p = 0; p < d; ++p)
                m_centers[j][p] += um * m_data[i][p];
        }
        if (sumUm > 1e-12)
            for (int p = 0; p < d; ++p)
                m_centers[j][p] /= sumUm;
    }
}

/* ---- 2x2 matrix inverse ---- */

QVector<QVector<double>> FuzzyCMeans9::invert2x2(
    const QVector<QVector<double>>& mat)
{
    double a = mat[0][0], b = mat[0][1];
    double c = mat[1][0], dd = mat[1][1];
    double det = a * dd - b * c;
    if (qAbs(det) < 1e-15)
        det = 1e-15;
    QVector<QVector<double>> inv(2, QVector<double>(2));
    inv[0][0] = dd / det;  inv[0][1] = -b / det;
    inv[1][0] = -c / det;  inv[1][1] = a / det;
    return inv;
}

/* ---- Trace ---- */

double FuzzyCMeans9::trace(const QVector<QVector<double>>& mat)
{
    double t = 0.0;
    for (int i = 0; i < qMin(mat.size(), mat.isEmpty() ? 0 : mat[0].size()); ++i)
        t += mat[i][i];
    return t;
}

/* ---- Gustafson-Kessel distance ---- */

double FuzzyCMeans9::gkDistance(const QVector<double>& x,
                                 const QVector<double>& v,
                                 int clusterIdx) const
{
    int d = qMin(x.size(), v.size());
    if (d > 2) d = 2;  // Embedded constraint: use 2D covariance

    // Compute diff vector
    QVector<double> diff(d);
    for (int i = 0; i < d; ++i)
        diff[i] = x[i] - v[i];

    // Use covariance inverse for distance
    if (clusterIdx < 0 || clusterIdx >= m_covariances.size())
        return 1e10;

    const auto& cov = m_covariances[clusterIdx];
    auto inv = invert2x2(cov);

    double dist = 0.0;
    for (int i = 0; i < d; ++i) {
        double row = 0.0;
        for (int j = 0; j < d; ++j)
            row += inv[i][j] * diff[j];
        dist += diff[i] * row;
    }

    // Adaptive volume constraint scaling
    if (clusterIdx < m_volumeConstraints.size())
        dist *= m_volumeConstraints[clusterIdx];

    return qSqrt(qAbs(dist));
}

/* ---- Update membership matrix ---- */

void FuzzyCMeans9::updateMembership()
{
    int n = m_data.size();
    double exp = 1.0 / (m_fuzziness - 1.0);

    for (int i = 0; i < n; ++i) {
        double sumInv = 0.0;
        QVector<double> dists(m_k);
        for (int j = 0; j < m_k; ++j) {
            dists[j] = gkDistance(m_data[i], m_centers[j], j);
            if (dists[j] < 1e-12) dists[j] = 1e-12;
            sumInv += qPow(1.0 / dists[j], exp);
        }
        for (int j = 0; j < m_k; ++j)
            m_U[i][j] = qPow(1.0 / dists[j], exp) / sumInv;
    }
}

/* ---- Update covariances ---- */

void FuzzyCMeans9::updateCovariances()
{
    int n = m_data.size();
    int d = qMin(m_dim, 2);
    m_covariances.resize(m_k);

    for (int j = 0; j < m_k; ++j) {
        m_covariances[j].resize(d, QVector<double>(d, 0.0));
        double sumUm = 0.0;
        for (int i = 0; i < n; ++i) {
            double um = qPow(m_U[i][j], m_fuzziness);
            sumUm += um;
            for (int p = 0; p < d; ++p) {
                double dp = m_data[i][p] - m_centers[j][p];
                for (int q = 0; q < d; ++q) {
                    double dq = m_data[i][q] - m_centers[j][q];
                    m_covariances[j][p][q] += um * dp * dq;
                }
            }
        }
        if (sumUm > 1e-12) {
            double det = qAbs(m_covariances[j][0][0] * m_covariances[j][1][1]
                              - m_covariances[j][0][1] * m_covariances[j][1][0]);
            double volTarget = 1.0;
            if (det > 1e-15) {
                double scale = qPow(volTarget / det, 1.0 / d);
                for (int p = 0; p < d; ++p)
                    for (int q = 0; q < d; ++q)
                        m_covariances[j][p][q] = m_covariances[j][p][q] / sumUm * scale;
            }
        }
    }
}

/* ---- Compute volume constraints ---- */

QVector<double> FuzzyCMeans9::computeVolumeConstraints() const
{
    QVector<double> constraints(m_k, 1.0);
    int n = m_data.size();
    for (int j = 0; j < m_k; ++j) {
        double totalWeight = 0.0;
        for (int i = 0; i < n; ++i)
            totalWeight += m_U[i][j];
        if (totalWeight > 1e-12)
            constraints[j] = n / (m_k * totalWeight);
    }
    return constraints;
}

/* ---- Fit ---- */

void FuzzyCMeans9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n == 0) return;
    m_dim = data[0].size();
    m_volumeConstraints = QVector<double>(m_k, 1.0);

    initMembership(n);

    int iter = 0;
    double prevCost = std::numeric_limits<double>::max();

    for (iter = 0; iter < m_maxIter; ++iter) {
        updateCenters();
        updateCovariances();
        m_volumeConstraints = computeVolumeConstraints();
        updateMembership();

        // Compute objective function
        double cost = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m_k; ++j)
                cost += qPow(m_U[i][j], m_fuzziness)
                        * gkDistance(m_data[i], m_centers[j], j);

        if (qAbs(prevCost - cost) < m_eps) break;
        prevCost = cost;
    }

    m_stats.numPoints = n;
    m_stats.numClusters = m_k;
    m_stats.iterations = iter;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(m_k, iter, timer.elapsed());
}

/* ---- Get membership matrix ---- */

QVector<QVector<double>> FuzzyCMeans9::getMembershipMatrix() const { return m_U; }

/* ---- Get centers ---- */

QVector<QVector<double>> FuzzyCMeans9::getCenters() const { return m_centers; }

/* ---- Get hard labels ---- */

QVector<int> FuzzyCMeans9::getLabels() const
{
    int n = m_data.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double maxU = 0.0;
        for (int j = 0; j < m_k; ++j) {
            if (m_U[i][j] > maxU) { maxU = m_U[i][j]; labels[i] = j; }
        }
    }
    return labels;
}

/* ---- Reset ---- */

void FuzzyCMeans9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_data.clear();
    m_U.clear();
    m_centers.clear();
    m_covariances.clear();
    m_volumeConstraints.clear();
}
