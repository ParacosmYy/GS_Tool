/**
 * @file PolynomialFit.cpp
 * @brief 多项式拟合实现
 */

#include "PolynomialFit.h"
#include <QElapsedTimer>
#include <cmath>

QVector<double> PolynomialFit::fit(const QVector<double>& x,
                                     const QVector<double>& y,
                                     int degree) const
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(x.size(), y.size());
    int m = degree + 1;

    QVector<double> result;
    if (n < m) {
        m_stats.totalFits++;
        m_timeSum += timer.elapsed();
        return result;
    }

    /* 构建法方程 (X^T * X) * a = X^T * y */
    QVector<QVector<double>> mat(m, QVector<double>(m + 1, 0.0));

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += std::pow(x[k], i + j);
            }
            mat[i][j] = sum;
        }
        double sum = 0.0;
        for (int k = 0; k < n; ++k)
            sum += y[k] * std::pow(x[k], i);
        mat[i][m] = sum;
    }

    /* Gauss-Jordan消元 */
    for (int col = 0; col < m; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < m; ++row) {
            if (std::abs(mat[row][col]) > std::abs(mat[maxRow][col]))
                maxRow = row;
        }
        if (maxRow != col) {
            for (int j = 0; j <= m; ++j)
                std::swap(mat[col][j], mat[maxRow][j]);
        }

        if (std::abs(mat[col][col]) < 1e-15) continue;

        for (int row = col + 1; row < m; ++row) {
            double factor = mat[row][col] / mat[col][col];
            for (int j = col; j <= m; ++j)
                mat[row][j] -= factor * mat[col][j];
        }
    }

    /* 回代 */
    result.resize(m);
    for (int i = m - 1; i >= 0; --i) {
        double sum = mat[i][m];
        for (int j = i + 1; j < m; ++j)
            sum -= mat[i][j] * result[j];
        result[i] = (std::abs(mat[i][i]) > 1e-15) ? sum / mat[i][i] : 0.0;
    }

    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(degree, rSquared(x, y, result));
    return result;
}

double PolynomialFit::evaluate(const QVector<double>& coefficients, double x) const
{
    double result = 0.0;
    double xp = 1.0;
    for (double c : coefficients) {
        result += c * xp;
        xp *= x;
    }
    return result;
}

QVector<double> PolynomialFit::evaluateBatch(const QVector<double>& coefficients,
                                               const QVector<double>& xs) const
{
    QVector<double> result;
    result.reserve(xs.size());
    for (double x : xs)
        result.append(evaluate(coefficients, x));
    return result;
}

double PolynomialFit::rSquared(const QVector<double>& x, const QVector<double>& y,
                                const QVector<double>& coefficients) const
{
    int n = qMin(x.size(), y.size());
    if (n == 0) return 0.0;

    double yMean = 0.0;
    for (int i = 0; i < n; ++i) yMean += y[i];
    yMean /= n;

    double ssTot = 0.0, ssRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double pred = evaluate(coefficients, x[i]);
        ssTot += (y[i] - yMean) * (y[i] - yMean);
        ssRes += (y[i] - pred) * (y[i] - pred);
    }

    return (ssTot > 1e-15) ? 1.0 - ssRes / ssTot : 1.0;
}

QVector<double> PolynomialFit::autoFit(const QVector<double>& x,
                                         const QVector<double>& y,
                                         int maxDegree) const
{
    double bestAIC = 1e18;
    QVector<double> bestCoeffs;

    for (int d = 1; d <= maxDegree; ++d) {
        auto coeffs = fit(x, y, d);
        if (coeffs.isEmpty()) continue;

        int n = qMin(x.size(), y.size());
        double ssRes = 0.0;
        for (int i = 0; i < n; ++i) {
            double pred = evaluate(coeffs, x[i]);
            ssRes += (y[i] - pred) * (y[i] - pred);
        }

        double aic = n * std::log(std::max(ssRes / n, 1e-15)) + 2 * (d + 1);
        if (aic < bestAIC) {
            bestAIC = aic;
            bestCoeffs = coeffs;
        }
    }

    return bestCoeffs;
}

PolynomialFit::Stats PolynomialFit::stats() const { return m_stats; }

void PolynomialFit::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
