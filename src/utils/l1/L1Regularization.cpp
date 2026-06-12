/**
 * @file L1Regularization.cpp
 * @brief L1正则化实现
 */

#include "L1Regularization.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

L1Regularization::L1Regularization(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> L1Regularization::lasso(const QVector<QVector<double>>& X,
                                           const QVector<double>& y,
                                           double lambda, int maxIter,
                                           double tolerance)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(X.size(), y.size());
    int p = (n > 0) ? X[0].size() : 0;
    if (n == 0 || p == 0) return {};

    QVector<double> beta(p, 0.0);

    /* 坐标下降法 */
    for (int iter = 0; iter < maxIter; ++iter) {
        double maxChange = 0.0;

        for (int j = 0; j < p; ++j) {
            /* 计算偏残差 */
            double rho = 0.0;
            double normSq = 0.0;

            for (int i = 0; i < n; ++i) {
                double pred = 0.0;
                for (int k = 0; k < p; ++k)
                    if (k != j) pred += beta[k] * X[i][k];
                double residual = y[i] - pred;
                rho += X[i][j] * residual;
                normSq += X[i][j] * X[i][j];
            }

            double oldBeta = beta[j];
            if (normSq < 1e-15) {
                beta[j] = 0.0;
            } else {
                beta[j] = softThreshold(rho / normSq, lambda / (2.0 * normSq));
            }

            maxChange = qMax(maxChange, std::abs(beta[j] - oldBeta));
        }

        if (maxChange < tolerance) break;
    }

    m_stats.totalFitted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitted;

    emit fitted(p, lambda);
    return beta;
}

QVector<double> L1Regularization::elasticNet(const QVector<QVector<double>>& X,
                                               const QVector<double>& y,
                                               double lambda, double alpha)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(X.size(), y.size());
    int p = (n > 0) ? X[0].size() : 0;
    if (n == 0 || p == 0) return {};

    QVector<double> beta(p, 0.0);
    double l1 = lambda * alpha;
    double l2 = lambda * (1.0 - alpha);

    for (int iter = 0; iter < 1000; ++iter) {
        double maxChange = 0.0;
        for (int j = 0; j < p; ++j) {
            double rho = 0.0, normSq = 0.0;
            for (int i = 0; i < n; ++i) {
                double pred = 0.0;
                for (int k = 0; k < p; ++k)
                    if (k != j) pred += beta[k] * X[i][k];
                rho += X[i][j] * (y[i] - pred);
                normSq += X[i][j] * X[i][j];
            }

            double old = beta[j];
            if (normSq < 1e-15) { beta[j] = 0.0; }
            else {
                double denom = normSq + l2;
                beta[j] = softThreshold(rho / denom, l1 / (2.0 * denom));
            }
            maxChange = qMax(maxChange, std::abs(beta[j] - old));
        }
        if (maxChange < 1e-6) break;
    }

    m_stats.totalFitted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitted;

    return beta;
}

double L1Regularization::softThreshold(double x, double threshold)
{
    if (x > threshold) return x - threshold;
    if (x < -threshold) return x + threshold;
    return 0.0;
}

QVector<QVector<double>> L1Regularization::regularizationPath(
    const QVector<QVector<double>>& X,
    const QVector<double>& y,
    const QVector<double>& lambdas)
{
    QVector<QVector<double>> path;
    for (double lam : lambdas)
        path.append(lasso(X, y, lam));
    return path;
}

double L1Regularization::predict(const QVector<double>& coefficients,
                                    const QVector<double>& x)
{
    double result = 0.0;
    int dim = qMin(coefficients.size(), x.size());
    for (int i = 0; i < dim; ++i)
        result += coefficients[i] * x[i];
    return result;
}

L1Regularization::Stats L1Regularization::stats() const { return m_stats; }

void L1Regularization::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
