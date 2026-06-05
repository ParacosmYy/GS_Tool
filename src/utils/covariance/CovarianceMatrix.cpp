/**
 * @file CovarianceMatrix.cpp
 * @brief 协方差矩阵计算器实现
 */

#include "utils/covariance/CovarianceMatrix.h"

#include <QtMath>
#include <QElapsedTimer>

CovarianceMatrix::CovarianceMatrix(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<QVector<double>> CovarianceMatrix::compute(
    const QVector<QVector<double>>& data, bool sample)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    int d = data[0].size();
    if (d == 0) return {};
    /* 验证所有行维度一致 */
    for (int k = 1; k < n; ++k) {
        if (data[k].size() != d) return {};
    }
    QVector<QVector<double>> cov(d, QVector<double>(d, 0.0));

    QVector<double> mean = computeMean(data);

    for (int i = 0; i < d; ++i) {
        for (int j = i; j < d; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                if (data[k].size() > j && data[k].size() > i) {
                    sum += (data[k][i] - mean[i]) * (data[k][j] - mean[j]);
                }
            }
            double divisor = sample ? (n - 1) : n;
            cov[i][j] = (divisor > 0) ? sum / divisor : 0.0;
            cov[j][i] = cov[i][j];
        }
    }

    m_stats.totalComputations++;
    m_stats.totalDimensionsProcessed += d;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(d);
    return cov;
}

QVector<QVector<double>> CovarianceMatrix::correlationMatrix(
    const QVector<QVector<double>>& covMatrix) const
{
    int d = covMatrix.size();
    QVector<QVector<double>> corr(d, QVector<double>(d, 0.0));

    for (int i = 0; i < d; ++i) corr[i][i] = 1.0;

    for (int i = 0; i < d; ++i) {
        for (int j = i + 1; j < d; ++j) {
            double denom = qSqrt(covMatrix[i][i] * covMatrix[j][j]);
            corr[i][j] = (denom > 1e-15) ? covMatrix[i][j] / denom : 0.0;
            corr[j][i] = corr[i][j];
        }
    }
    return corr;
}

double CovarianceMatrix::mahalanobisDistance(const QVector<double>& x,
                                              const QVector<double>& mean,
                                              const QVector<QVector<double>>& covInv) const
{
    int d = x.size();
    if (d != mean.size() || static_cast<int>(covInv.size()) < d) return 0.0;

    QVector<double> diff(d);
    for (int i = 0; i < d; ++i) diff[i] = x[i] - mean[i];

    double dist = 0.0;
    for (int i = 0; i < d; ++i) {
        double sum = 0.0;
        for (int j = 0; j < d; ++j) {
            sum += diff[j] * covInv[i][j];
        }
        dist += sum * diff[i];
    }
    return qSqrt(qMax(dist, 0.0));
}

QVector<double> CovarianceMatrix::computeMean(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n == 0) return {};

    int d = data[0].size();
    QVector<double> mean(d, 0.0);

    for (const auto& row : data) {
        for (int j = 0; j < qMin(d, row.size()); ++j) {
            mean[j] += row[j];
        }
    }
    for (int j = 0; j < d; ++j) mean[j] /= n;
    return mean;
}

void CovarianceMatrix::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
