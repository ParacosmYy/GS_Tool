/**
 * @file PolynomialRegression.cpp
 * @brief 多项式回归实现
 */

#include "utils/polyreg/PolynomialRegression.h"

#include <QElapsedTimer>
#include <QtMath>

PolynomialRegression::PolynomialRegression(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

PolynomialRegression::FitResult PolynomialRegression::fit(
    const QVector<double>& x, const QVector<double>& y, int degree)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;
    result.degree = degree;
    int n = qMin(x.size(), y.size());
    if (n <= degree || degree < 0) {
        m_stats.totalFits++;
        return result;
    }

    int m = degree + 1;

    /* 构建正规方程 (X^T X) c = X^T y */
    QVector<QVector<double>> XtX(m, QVector<double>(m, 0.0));
    QVector<double> Xty(m, 0.0);

    for (int i = 0; i < n; ++i) {
        double xpowers[32];
        xpowers[0] = 1.0;
        for (int j = 1; j < m; ++j) xpowers[j] = xpowers[j - 1] * x[i];

        for (int j = 0; j < m; ++j) {
            Xty[j] += xpowers[j] * y[i];
            for (int k = 0; k < m; ++k) {
                XtX[j][k] += xpowers[j] * xpowers[k];
            }
        }
    }

    result.coefficients = solveSystem(XtX, Xty);

    /* 计算R²和RMSE */
    double ssTot = 0.0, ssRes = 0.0, yMean = 0.0;
    for (int i = 0; i < n; ++i) yMean += y[i];
    yMean /= n;

    for (int i = 0; i < n; ++i) {
        double pred = predict(result.coefficients, x[i]);
        double diff = y[i] - pred;
        ssRes += diff * diff;
        ssTot += (y[i] - yMean) * (y[i] - yMean);
    }

    result.rSquared = (ssTot > 1e-15) ? 1.0 - ssRes / ssTot : 0.0;
    result.rmse = qSqrt(ssRes / n);

    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalFits, 1ULL);

    emit fitCompleted(degree, result.rSquared);
    return result;
}

double PolynomialRegression::predict(
    const QVector<double>& coefficients, double xValue) const
{
    double result = 0.0;
    double xpow = 1.0;
    for (int i = 0; i < coefficients.size(); ++i) {
        result += coefficients[i] * xpow;
        xpow *= xValue;
    }
    return result;
}

QVector<double> PolynomialRegression::predictBatch(
    const QVector<double>& coefficients,
    const QVector<double>& xValues) const
{
    QVector<double> result;
    result.reserve(xValues.size());
    for (double x : xValues) {
        result.append(predict(coefficients, x));
    }
    return result;
}

PolynomialRegression::FitResult PolynomialRegression::autoFit(
    const QVector<double>& x, const QVector<double>& y, int maxDegree)
{
    FitResult bestResult;
    double bestAIC = 1e30;

    for (int d = 1; d <= qMin(maxDegree, x.size() - 1); ++d) {
        FitResult res = fit(x, y, d);
        if (res.coefficients.isEmpty()) continue;

        /* AIC = n*ln(RSS/n) + 2*(d+2) */
        double rss = 0.0;
        int n = qMin(x.size(), y.size());
        for (int i = 0; i < n; ++i) {
            double pred = predict(res.coefficients, x[i]);
            double diff = y[i] - pred;
            rss += diff * diff;
        }
        double aic = n * qLn(qMax(rss / n, 1e-30)) + 2.0 * (d + 2);

        if (aic < bestAIC) {
            bestAIC = aic;
            bestResult = res;
        }
    }

    return bestResult;
}

QVector<double> PolynomialRegression::solveSystem(
    QVector<QVector<double>>& A, QVector<double>& b) const
{
    int n = A.size();
    if (n == 0) return {};

    /* 增广矩阵 */
    for (int i = 0; i < n; ++i) A[i].append(b[i]);

    /* 前向消元(部分主元) */
    for (int col = 0; col < n; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(A[row][col]) > qAbs(A[maxRow][col]))
                maxRow = row;
        }
        std::swap(A[col], A[maxRow]);

        if (qAbs(A[col][col]) < 1e-15) continue;

        for (int row = col + 1; row < n; ++row) {
            double factor = A[row][col] / A[col][col];
            for (int j = col; j <= n; ++j) {
                A[row][j] -= factor * A[col][j];
            }
        }
    }

    /* 回代 */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        if (qAbs(A[i][i]) < 1e-15) continue;
        x[i] = A[i][n];
        for (int j = i + 1; j < n; ++j) {
            x[i] -= A[i][j] * x[j];
        }
        x[i] /= A[i][i];
    }

    /* 移除增广列 */
    for (int i = 0; i < n; ++i) A[i].removeLast();

    return x;
}

void PolynomialRegression::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
