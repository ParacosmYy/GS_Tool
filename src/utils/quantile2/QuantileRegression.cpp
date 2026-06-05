/**
 * @file QuantileRegression.cpp
 * @brief 分位数回归实现
 */

#include "QuantileRegression.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

QuantileRegression::QuantileRegression(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> QuantileRegression::fit(const QVector<QVector<double>>& x,
                                           const QVector<double>& y, double tau)
{
    QElapsedTimer timer;
    timer.start();

    auto result = solveLinearQuantile(x, y, tau);

    m_stats.totalFitted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitted;

    emit fitted(tau, result.size());
    return result;
}

QVector<QVector<double>> QuantileRegression::fitMulti(
    const QVector<QVector<double>>& x,
    const QVector<double>& y,
    const QVector<double>& taus)
{
    QVector<QVector<double>> results;
    for (double tau : taus)
        results.append(fit(x, y, tau));
    return results;
}

double QuantileRegression::predict(const QVector<double>& coefficients,
                                      const QVector<double>& x)
{
    double result = 0.0;
    int dim = qMin(coefficients.size(), x.size());
    for (int i = 0; i < dim; ++i)
        result += coefficients[i] * x[i];
    return result;
}

double QuantileRegression::checkLoss(const QVector<double>& residuals, double tau)
{
    double loss = 0.0;
    for (double r : residuals) {
        if (r >= 0)
            loss += tau * r;
        else
            loss += (tau - 1.0) * r;
    }
    return loss;
}

QVector<double> QuantileRegression::solveLinearQuantile(
    const QVector<QVector<double>>& x,
    const QVector<double>& y,
    double tau)
{
    int n = qMin(x.size(), y.size());
    if (n == 0) return {};

    int p = x[0].size();
    QVector<double> beta(p, 0.0);

    /* 使用迭代重加权最小二乘(IRLS) */
    for (int iter = 0; iter < 200; ++iter) {
        QVector<double> residuals(n);
        for (int i = 0; i < n; ++i)
            residuals[i] = y[i] - predict(beta, x[i]);

        QVector<double> weights(n);
        for (int i = 0; i < n; ++i) {
            double r = residuals[i];
            double eps = 1e-6;
            if (r > eps)
                weights[i] = tau / r;
            else if (r < -eps)
                weights[i] = (tau - 1.0) / r;
            else
                weights[i] = tau / eps;
        }

        /* 加权最小二乘: (X^T W X) beta = X^T W y */
        QVector<QVector<double>> XtWX(p, QVector<double>(p, 0.0));
        QVector<double> XtWy(p, 0.0);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < p; ++j) {
                XtWy[j] += weights[i] * x[i][j] * y[i];
                for (int k = 0; k < p; ++k)
                    XtWX[j][k] += weights[i] * x[i][j] * x[i][k];
            }
        }

        /* 高斯消元求解 */
        QVector<double> newBeta = XtWy;
        QVector<QVector<double>> A = XtWX;

        for (int col = 0; col < p; ++col) {
            int maxRow = col;
            for (int row = col + 1; row < p; ++row) {
                if (std::abs(A[row][col]) > std::abs(A[maxRow][col]))
                    maxRow = row;
            }
            std::swap(A[col], A[maxRow]);
            std::swap(newBeta[col], newBeta[maxRow]);

            if (std::abs(A[col][col]) < 1e-15) continue;

            for (int row = col + 1; row < p; ++row) {
                double factor = A[row][col] / A[col][col];
                for (int k = col; k < p; ++k)
                    A[row][k] -= factor * A[col][k];
                newBeta[row] -= factor * newBeta[col];
            }
        }

        /* 回代 */
        for (int i = p - 1; i >= 0; --i) {
            if (std::abs(A[i][i]) < 1e-15) continue;
            for (int j = i + 1; j < p; ++j)
                newBeta[i] -= A[i][j] * newBeta[j];
            newBeta[i] /= A[i][i];
        }

        /* 检查收敛 */
        double maxChange = 0.0;
        for (int i = 0; i < p; ++i)
            maxChange = qMax(maxChange, std::abs(newBeta[i] - beta[i]));

        beta = newBeta;
        if (maxChange < 1e-8) break;
    }

    return beta;
}

QuantileRegression::Stats QuantileRegression::stats() const { return m_stats; }

void QuantileRegression::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
