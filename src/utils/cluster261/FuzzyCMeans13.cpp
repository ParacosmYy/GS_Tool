/**
 * @file FuzzyCMeans13.cpp
 * @brief FuzzyCMeans13 实现
 *
 * 实现模糊C均值聚类：Gustafson-Kessel距离协方差矩阵椭球簇自适应形状。
 */

#include "utils/cluster261/FuzzyCMeans13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FuzzyCMeans13::FuzzyCMeans13(QObject *parent)
    : QObject(parent) {}

FuzzyCMeans13::~FuzzyCMeans13() = default;

/* ---- Configuration ---- */

void FuzzyCMeans13::setParameters(int numClusters, double fuzziness, double epsilon,
                                  int maxIterations)
{
    m_C = qMax(2, numClusters);
    m_m = qBound(1.01, fuzziness, 100.0);
    m_eps = qMax(1e-10, epsilon);
    m_maxIter = qMax(1, maxIterations);
}

/* ---- Initialize membership randomly ---- */

void FuzzyCMeans13::initMembership(int n)
{
    m_U.resize(n);
    for (int i = 0; i < n; ++i) {
        m_U[i].resize(m_C);
        double sum = 0.0;
        for (int j = 0; j < m_C; ++j) {
            m_U[i][j] = static_cast<double>(qrand()) / RAND_MAX + 0.01;
            sum += m_U[i][j];
        }
        for (int j = 0; j < m_C; ++j)
            m_U[i][j] /= sum;
    }
}

/* ---- Update cluster centers ---- */

void FuzzyCMeans13::updateCenters()
{
    int n = m_data.size();
    m_centers.resize(m_C);
    double power = m_m;
    for (int j = 0; j < m_C; ++j) {
        m_centers[j].resize(m_dims);
        double denom = 0.0;
        QVector<double> numer(m_dims, 0.0);
        for (int i = 0; i < n; ++i) {
            double w = qPow(m_U[i][j], power);
            denom += w;
            for (int d = 0; d < m_dims; ++d)
                numer[d] += w * m_data[i][d];
        }
        if (denom > 1e-15) {
            for (int d = 0; d < m_dims; ++d)
                m_centers[j][d] = numer[d] / denom;
        }
    }
}

/* ---- Compute covariance matrices ---- */

void FuzzyCMeans13::updateCovariances()
{
    int n = m_data.size();
    m_cov.resize(m_C);
    double power = m_m;
    for (int j = 0; j < m_C; ++j) {
        m_cov[j].resize(m_dims);
        for (int d = 0; d < m_dims; ++d)
            m_cov[j][d].resize(m_dims, 0.0);

        double denom = 0.0;
        for (int i = 0; i < n; ++i) {
            double w = qPow(m_U[i][j], power);
            denom += w;
            for (int d1 = 0; d1 < m_dims; ++d1) {
                double diff1 = m_data[i][d1] - m_centers[j][d1];
                for (int d2 = d1; d2 < m_dims; ++d2) {
                    double diff2 = m_data[i][d2] - m_centers[j][d2];
                    m_cov[j][d1][d2] += w * diff1 * diff2;
                }
            }
        }
        // Regularize and symmetrize
        if (denom > 1e-15) {
            double detFactor = qPow(denom, 1.0 / m_dims);
            for (int d1 = 0; d1 < m_dims; ++d1)
                for (int d2 = d1; d2 < m_dims; ++d2) {
                    m_cov[j][d1][d2] /= denom;
                    m_cov[j][d2][d1] = m_cov[j][d1][d2];
                }
        }
        // Add regularization for numerical stability
        for (int d = 0; d < m_dims; ++d)
            m_cov[j][d][d] += 1e-6;
    }
}

/* ---- Gustafson-Kessel distance ---- */

double FuzzyCMeans13::gkDistance(const QVector<double>& point, int cluster) const
{
    // Compute diff vector
    QVector<double> diff(m_dims);
    for (int d = 0; d < m_dims; ++d)
        diff[d] = point[d] - m_centers[cluster][d];

    // Solve linear system A^-1 * diff using Gaussian elimination
    // Build augmented matrix [cov | diff]
    QVector<QVector<double>> aug(m_dims);
    for (int i = 0; i < m_dims; ++i) {
        aug[i] = m_cov[cluster][i];
        aug[i].append(diff[i]);
    }
    // Forward elimination
    for (int col = 0; col < m_dims; ++col) {
        double piv = qAbs(aug[col][col]);
        int pivRow = col;
        for (int row = col + 1; row < m_dims; ++row) {
            if (qAbs(aug[row][col]) > piv) { piv = qAbs(aug[row][col]); pivRow = row; }
        }
        if (piv < 1e-15) return std::numeric_limits<double>::max();
        if (pivRow != col) aug.swapItems(col, pivRow);
        for (int row = col + 1; row < m_dims; ++row) {
            double factor = aug[row][col] / aug[col][col];
            for (int j = col; j <= m_dims; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }
    // Back substitution
    QVector<double> solved(m_dims, 0.0);
    for (int i = m_dims - 1; i >= 0; --i) {
        solved[i] = aug[i][m_dims];
        for (int j = i + 1; j < m_dims; ++j)
            solved[i] -= aug[i][j] * solved[j];
        solved[i] /= aug[i][i];
    }
    // d^2 = diff^T * solved
    double dist = 0.0;
    for (int d = 0; d < m_dims; ++d)
        dist += diff[d] * solved[d];
    return qSqrt(qMax(0.0, dist));
}

/* ---- Update membership matrix ---- */

double FuzzyCMeans13::updateMembership()
{
    int n = m_data.size();
    double maxChange = 0.0;
    double power = 2.0 / (m_m - 1.0);

    for (int i = 0; i < n; ++i) {
        QVector<double> newU(m_C);
        for (int j = 0; j < m_C; ++j) {
            double dJ = gkDistance(m_data[i], j);
            if (dJ < 1e-15) {
                // Point coincides with cluster center
                for (int k = 0; k < m_C; ++k) newU[k] = (k == j) ? 1.0 : 0.0;
                break;
            }
            double sum = 0.0;
            for (int k = 0; k < m_C; ++k) {
                double dK = gkDistance(m_data[i], k);
                sum += qPow(dJ / qMax(dK, 1e-15), power);
            }
            newU[j] = 1.0 / qMax(sum, 1e-15);
        }
        for (int j = 0; j < m_C; ++j) {
            double change = qAbs(newU[j] - m_U[i][j]);
            if (change > maxChange) maxChange = change;
            m_U[i][j] = newU[j];
        }
    }
    return maxChange;
}

/* ---- Compute objective function ---- */

double FuzzyCMeans13::computeObjective() const
{
    double obj = 0.0;
    int n = m_data.size();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m_C; ++j)
            obj += qPow(m_U[i][j], m_m) *
                   qPow(gkDistance(m_data[i], j), 2.0);
    return obj;
}

/* ---- Fit (main loop) ---- */

bool FuzzyCMeans13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_C) return false;
    m_data = data;
    int n = data.size();
    m_dims = data[0].size();

    initMembership(n);
    updateCenters();
    updateCovariances();

    int iter = 0;
    double maxChange = 0.0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        updateCenters();
        updateCovariances();
        maxChange = updateMembership();
        if (maxChange < m_eps) break;
    }

    double objective = computeObjective();
    double elapsed = timer.elapsed();

    m_stats.numPoints = n;
    m_stats.numDimensions = m_dims;
    m_stats.numClusters = m_C;
    m_stats.numIterations = iter;
    m_stats.finalObjective = objective;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringUpdated(m_C, iter, objective, elapsed);
    return maxChange < m_eps;
}

/* ---- Accessors ---- */

QVector<QVector<double>> FuzzyCMeans13::centers() const { return m_centers; }
QVector<QVector<double>> FuzzyCMeans13::membershipMatrix() const { return m_U; }

QVector<int> FuzzyCMeans13::labels() const
{
    QVector<int> result(m_data.size());
    for (int i = 0; i < m_data.size(); ++i) {
        int best = 0;
        double bestU = m_U[i][0];
        for (int j = 1; j < m_C; ++j) {
            if (m_U[i][j] > bestU) { bestU = m_U[i][j]; best = j; }
        }
        result[i] = best;
    }
    return result;
}

QVector<QVector<QVector<double>>> FuzzyCMeans13::covariances() const { return m_cov; }

/* ---- Reset ---- */

void FuzzyCMeans13::resetStatistics()
{
    m_data.clear();
    m_U.clear();
    m_centers.clear();
    m_cov.clear();
    m_dims = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
