/**
 * @file FuzzyCMeans7.cpp
 * @brief FuzzyCMeans7 实现
 *
 * 实现模糊C均值聚类：Gustafson-Kessel距离、协方差矩阵、划分熵。
 */

#include "utils/cluster176/FuzzyCMeans7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FuzzyCMeans7::FuzzyCMeans7(QObject *parent)
    : QObject(parent)
{
}

FuzzyCMeans7::~FuzzyCMeans7() = default;

/* ---- Configuration ---- */

void FuzzyCMeans7::setNumClusters(int c) { m_numClusters = qMax(2, c); }
void FuzzyCMeans7::setFuzziness(double m) { m_fuzziness = qMax(1.1, m); }
void FuzzyCMeans7::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void FuzzyCMeans7::setTolerance(double eps) { m_tolerance = qMax(1e-8, eps); }

/* ---- Initialize random membership matrix ---- */

void FuzzyCMeans7::initMembership()
{
    m_U.resize(m_n);
    double exp = 1.0 / (m_numClusters - 1.0);

    for (int i = 0; i < m_n; ++i) {
        m_U[i].resize(m_numClusters);
        double sum = 0.0;
        for (int c = 0; c < m_numClusters; ++c) {
            m_U[i][c] = static_cast<double>(qrand()) / RAND_MAX + 0.1;
            sum += m_U[i][c];
        }
        for (int c = 0; c < m_numClusters; ++c)
            m_U[i][c] /= sum;
    }
}

/* ---- Update cluster centers from weighted memberships ---- */

void FuzzyCMeans7::updateCenters(const QVector<QVector<double>>& data)
{
    double mp = m_fuzziness;
    m_centers.resize(m_numClusters);

    for (int c = 0; c < m_numClusters; ++c) {
        m_centers[c].resize(m_dims, 0.0);
        double denom = 0.0;
        for (int i = 0; i < m_n; ++i) {
            double w = qPow(m_U[i][c], mp);
            denom += w;
            for (int d = 0; d < m_dims; ++d)
                m_centers[c][d] += w * data[i][d];
        }
        if (denom > 1e-12)
            for (int d = 0; d < m_dims; ++d)
                m_centers[c][d] /= denom;
    }
}

/* ---- Compute fuzzy covariance matrices per cluster ---- */

void FuzzyCMeans7::updateCovariances(const QVector<QVector<double>>& data,
                                       QVector<QVector<QVector<double>>>& cov)
{
    double mp = m_fuzziness;
    cov.resize(m_numClusters);

    for (int c = 0; c < m_numClusters; ++c) {
        cov[c].resize(m_dims);
        for (int d = 0; d < m_dims; ++d)
            cov[c][d].resize(m_dims, 0.0);

        double denom = 0.0;
        for (int i = 0; i < m_n; ++i) {
            double w = qPow(m_U[i][c], mp);
            denom += w;
            for (int r = 0; r < m_dims; ++r) {
                double dr = data[i][r] - m_centers[c][r];
                for (int s = 0; s < m_dims; ++s) {
                    double ds = data[i][s] - m_centers[c][s];
                    cov[c][r][s] += w * dr * ds;
                }
            }
        }
        if (denom > 1e-12) {
            for (int r = 0; r < m_dims; ++r)
                for (int s = 0; s < m_dims; ++s)
                    cov[c][r][s] /= denom;
        }
        /* Regularize: add small diagonal to ensure positive-definiteness */
        for (int r = 0; r < m_dims; ++r)
            cov[c][r][r] += 1e-6;
    }
}

/* ---- Gustafson-Kessel distance: (x-c)^T * det(C)^{1/d} * C^{-1} * (x-c) ---- */

double FuzzyCMeans7::gkDistance(const QVector<double>& x, int cluster,
                                  const QVector<QVector<QVector<double>>>& covInv)
{
    int d = m_dims;
    /* Compute (x - center) vector */
    QVector<double> diff(d);
    for (int i = 0; i < d; ++i)
        diff[i] = x[i] - m_centers[cluster][i];

    /* Quadratic form: diff^T * covInv * diff */
    double dist = 0.0;
    for (int r = 0; r < d; ++r)
        for (int s = 0; s < d; ++s)
            dist += diff[r] * covInv[cluster][r][s] * diff[s];

    return qSqrt(qMax(0.0, dist));
}

/* ---- Update membership using GK distances ---- */

void FuzzyCMeans7::updateMembership(const QVector<QVector<double>>& data,
                                      const QVector<QVector<QVector<double>>>& covInv)
{
    double exp = 2.0 / (m_fuzziness - 1.0);

    for (int i = 0; i < m_n; ++i) {
        double sumInv = 0.0;
        QVector<double> dists(m_numClusters);

        for (int c = 0; c < m_numClusters; ++c) {
            dists[c] = gkDistance(data[i], c, covInv);
            if (dists[c] < 1e-12) dists[c] = 1e-12;
            sumInv += 1.0 / qPow(dists[c], exp);
        }

        for (int c = 0; c < m_numClusters; ++c)
            m_U[i][c] = qPow(dists[c], -exp) / (sumInv * qPow(dists[c], 0.0) + 1e-30);
        /* Normalize */
        double s = 0.0;
        for (int c = 0; c < m_numClusters; ++c) s += m_U[i][c];
        if (s > 0)
            for (int c = 0; c < m_numClusters; ++c) m_U[i][c] /= s;
    }
}

/* ---- Partition entropy: PE = -sum(U_ij * log(U_ij)) / n ---- */

double FuzzyCMeans7::computeEntropy() const
{
    double pe = 0.0;
    for (int i = 0; i < m_n; ++i)
        for (int c = 0; c < m_numClusters; ++c)
            if (m_U[i][c] > 1e-12)
                pe -= m_U[i][c] * qLn(m_U[i][c]);
    return pe / m_n;
}

/* ---- Main fit ---- */

QVector<QVector<double>> FuzzyCMeans7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return {};

    m_dims = data[0].size();
    initMembership();

    /* Pre-compute covariance inverses */
    QVector<QVector<QVector<double>>> covInv;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        updateCenters(data);
        updateCovariances(data, covInv);

        /* Invert covariances via Gauss-Jordan (small dims) */
        covInv.resize(m_numClusters);
        for (int c = 0; c < m_numClusters; ++c) {
            int d = m_dims;
            /* Augmented matrix [cov | I] */
            QVector<QVector<double>> aug(d);
            for (int r = 0; r < d; ++r) {
                aug[r].resize(2 * d, 0.0);
                for (int s = 0; s < d; ++s) aug[r][s] = covInv[c][r][s];
                aug[r][d + r] = 1.0;
            }
            for (int col = 0; col < d; ++col) {
                double piv = aug[col][col];
                if (qAbs(piv) < 1e-12) piv = 1e-6;
                for (int s = 0; s < 2 * d; ++s) aug[col][s] /= piv;
                for (int r = 0; r < d; ++r) {
                    if (r == col) continue;
                    double f = aug[r][col];
                    for (int s = 0; s < 2 * d; ++s) aug[r][s] -= f * aug[col][s];
                }
            }
            covInv[c].resize(d);
            for (int r = 0; r < d; ++r) {
                covInv[c][r].resize(d);
                for (int s = 0; s < d; ++s) covInv[c][r][s] = aug[r][d + s];
            }
        }

        /* Save old memberships for convergence check */
        auto oldU = m_U;
        updateMembership(data, covInv);

        /* Check max change */
        double maxChange = 0.0;
        for (int i = 0; i < m_n; ++i)
            for (int c = 0; c < m_numClusters; ++c)
                maxChange = qMax(maxChange, qAbs(m_U[i][c] - oldU[i][c]));

        m_stats.iterations = iter + 1;
        if (maxChange < m_tolerance) break;
    }

    m_labels.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        int best = 0;
        for (int c = 1; c < m_numClusters; ++c)
            if (m_U[i][c] > m_U[i][best]) best = c;
        m_labels[i] = best;
    }

    double pe = computeEntropy();
    m_stats.totalRuns++;
    m_stats.numClusters = m_numClusters;
    m_stats.numSamples = m_n;
    m_stats.partitionEntropy = pe;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(m_numClusters, pe);
    return m_U;
}

QVector<int> FuzzyCMeans7::labels() const { return m_labels; }
QVector<QVector<double>> FuzzyCMeans7::centers() const { return m_centers; }
double FuzzyCMeans7::partitionEntropy() const { return m_stats.partitionEntropy; }

void FuzzyCMeans7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
