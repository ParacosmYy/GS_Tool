/**
 * @file GivensRotation.cpp
 * @brief Givens旋转实现
 */

#include "GivensRotation.h"
#include <QElapsedTimer>
#include <cmath>

GivensRotation::GivensRotation(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QPair<double, double> GivensRotation::compute(double a, double b)
{
    if (std::abs(b) < 1e-15) return {1.0, 0.0};
    if (std::abs(a) < 1e-15) return {0.0, (b > 0) ? 1.0 : -1.0};

    double r = std::hypot(a, b);
    return {a / r, -b / r};
}

void GivensRotation::applyLeft(QVector<QVector<double>>& matrix,
                                 int i, int j, int col, double c, double s)
{
    int n = matrix[i].size();
    for (int k = col; k < n; ++k) {
        double ri = matrix[i][k], rj = matrix[j][k];
        matrix[i][k] = c * ri - s * rj;
        matrix[j][k] = s * ri + c * rj;
    }
}

QPair<QVector<QVector<double>>, QVector<QVector<double>>> GivensRotation::qrDecompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int m = matrix.size();
    int n = (m > 0) ? matrix[0].size() : 0;
    if (m == 0 || n == 0) return {{}, {}};

    QVector<QVector<double>> R = matrix;
    QVector<QVector<double>> Q(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) Q[i][i] = 1.0;

    for (int j = 0; j < n; ++j) {
        for (int i = m - 1; i > j; --i) {
            if (std::abs(R[i][j]) < 1e-15) continue;

            auto [c, s] = compute(R[i - 1][j], R[i][j]);

            /* 应用到R */
            for (int k = j; k < n; ++k) {
                double ri = R[i - 1][k], rj = R[i][k];
                R[i - 1][k] = c * ri - s * rj;
                R[i][k] = s * ri + c * rj;
            }

            /* 应用到Q^T */
            for (int k = 0; k < m; ++k) {
                double qi = Q[i - 1][k], qj = Q[i][k];
                Q[i - 1][k] = c * qi - s * qj;
                Q[i][k] = s * qi + c * qj;
            }

            m_stats.totalRotations++;
        }
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit decompositionCompleted(m, n);
    return {Q, R};
}

QVector<double> GivensRotation::leastSquares(
    const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    int n = (m > 0) ? A[0].size() : 0;
    if (m == 0 || n == 0) return {};

    /* 增广矩阵 [A|b] */
    QVector<QVector<double>> aug(m, QVector<double>(n + 1));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = A[i][j];
        aug[i][n] = b[i];
    }

    /* Givens QR零化下三角 */
    for (int j = 0; j < n; ++j) {
        for (int i = m - 1; i > j; --i) {
            if (std::abs(aug[i][j]) < 1e-15) continue;
            auto [c, s] = compute(aug[i - 1][j], aug[i][j]);
            for (int k = j; k <= n; ++k) {
                double ri = aug[i - 1][k], rj = aug[i][k];
                aug[i - 1][k] = c * ri - s * rj;
                aug[i][k] = s * ri + c * rj;
            }
        }
    }

    /* 回代 */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = aug[i][n];
        for (int j = i + 1; j < n; ++j)
            sum -= aug[i][j] * x[j];
        if (std::abs(aug[i][i]) > 1e-15)
            x[i] = sum / aug[i][i];
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    return x;
}

GivensRotation::Stats GivensRotation::stats() const { return m_stats; }

void GivensRotation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
