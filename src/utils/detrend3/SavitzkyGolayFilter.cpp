/**
 * @file SavitzkyGolayFilter.cpp
 * @brief Savitzky-Golay平滑微分滤波器实现
 */

#include "SavitzkyGolayFilter.h"
#include <QElapsedTimer>
#include <cmath>

SavitzkyGolayFilter::SavitzkyGolayFilter(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> SavitzkyGolayFilter::smooth(const QVector<double>& data,
                                              int windowSize, int polyOrder)
{
    QElapsedTimer timer;
    timer.start();

    if (windowSize % 2 == 0) windowSize++;
    QVector<double> coeffs = computeCoeffs(windowSize, polyOrder, 0);
    QVector<double> result = applyConvolution(data, coeffs);

    m_stats.totalFiltered++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalFiltered + m_stats.totalDerivatives);

    emit filtered(data.size(), windowSize);
    return result;
}

QVector<double> SavitzkyGolayFilter::derivative(const QVector<double>& data,
                                                  int windowSize,
                                                  int polyOrder, double dx)
{
    QElapsedTimer timer;
    timer.start();

    if (windowSize % 2 == 0) windowSize++;
    QVector<double> coeffs = computeCoeffs(windowSize, polyOrder, 1, dx);
    QVector<double> result = applyConvolution(data, coeffs);

    m_stats.totalDerivatives++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalFiltered + m_stats.totalDerivatives);

    return result;
}

QVector<double> SavitzkyGolayFilter::secondDerivative(
    const QVector<double>& data, int windowSize, int polyOrder, double dx)
{
    QElapsedTimer timer;
    timer.start();

    if (windowSize % 2 == 0) windowSize++;
    QVector<double> coeffs = computeCoeffs(windowSize, polyOrder, 2, dx);
    QVector<double> result = applyConvolution(data, coeffs);

    m_stats.totalDerivatives++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalFiltered + m_stats.totalDerivatives);

    return result;
}

QVector<double> SavitzkyGolayFilter::computeCoeffs(int windowSize,
                                                     int polyOrder,
                                                     int deriv, double dx)
{
    int half = windowSize / 2;
    int m = polyOrder + 1;

    /* 构建设计矩阵 */
    QVector<QVector<double>> A(m, QVector<double>(windowSize, 0.0));
    for (int i = -half; i <= half; ++i) {
        double x = static_cast<double>(i);
        double xpow = 1.0;
        for (int j = 0; j < m; ++j) {
            A[j][i + half] = xpow;
            xpow *= x;
        }
    }

    /* A^T * A */
    QVector<QVector<double>> ATA(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            for (int k = 0; k < windowSize; ++k)
                ATA[i][j] += A[i][k] * A[j][k];
        }
    }

    /* 高斯消元求逆 */
    QVector<QVector<double>> aug(m, QVector<double>(2 * m, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) aug[i][j] = ATA[i][j];
        aug[i][m + i] = 1.0;
    }

    for (int col = 0; col < m; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < m; ++row)
            if (std::abs(aug[row][col]) > std::abs(aug[maxRow][col]))
                maxRow = row;
        std::swap(aug[col], aug[maxRow]);
        double pivot = aug[col][col];
        if (std::abs(pivot) < 1e-15) continue;
        for (int j = 0; j < 2 * m; ++j) aug[col][j] /= pivot;
        for (int row = 0; row < m; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * m; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }

    /* 计算卷积系数 */
    QVector<double> coeffs(windowSize, 0.0);
    double factorial = 1.0;
    for (int d = 0; d < deriv; ++d) factorial *= (d + 1);

    for (int i = 0; i < windowSize; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m; ++j)
            sum += aug[deriv][j + m] * A[j][i];
        coeffs[i] = sum / (factorial * std::pow(dx, deriv));
    }

    return coeffs;
}

QVector<double> SavitzkyGolayFilter::applyConvolution(
    const QVector<double>& data, const QVector<double>& coeffs)
{
    int n = data.size();
    int half = coeffs.size() / 2;
    QVector<double> result(n);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < static_cast<int>(coeffs.size()); ++j) {
            int idx = i - half + j;
            if (idx < 0) idx = -idx;
            if (idx >= n) idx = 2 * (n - 1) - idx;
            sum += coeffs[j] * data[idx];
        }
        result[i] = sum;
    }

    return result;
}

SavitzkyGolayFilter::Stats SavitzkyGolayFilter::stats() const
{
    return m_stats;
}

void SavitzkyGolayFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
