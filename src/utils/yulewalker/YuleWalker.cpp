/**
 * @file YuleWalker.cpp
 * @brief Yule-Walker方程求解实现 — Levinson-Durbin递归
 */

#include "utils/yulewalker/YuleWalker.h"

#include <QElapsedTimer>
#include <cmath>

YuleWalker::YuleWalker(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<double> YuleWalker::estimate(const QVector<double>& signal,
                                      int order)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    QVector<double> coeffs;

    if (n <= order || order <= 0) {
        m_stats.totalEstimates++;
        return coeffs;
    }

    /* 计算自相关 */
    QVector<double> r(order + 1, 0.0);
    for (int k = 0; k <= order; ++k) {
        for (int i = 0; i < n - k; ++i)
            r[k] += signal[i] * signal[i + k];
        r[k] /= n;
    }

    /* Levinson-Durbin递归 */
    coeffs.resize(order);
    double sigma2 = r[0];

    if (sigma2 < 1e-15) {
        m_stats.totalEstimates++;
        return coeffs;
    }

    QVector<double> a(order + 1, 0.0);
    a[0] = 1.0;

    for (int m = 1; m <= order; ++m) {
        /* 计算反射系数 */
        double km = 0.0;
        for (int j = 0; j < m; ++j)
            km += a[j] * r[m - j];
        km = -km / sigma2;

        /* 更新系数 */
        QVector<double> aNew = a;
        for (int j = 1; j < m; ++j)
            aNew[j] = a[j] + km * a[m - j];
        aNew[m] = km;
        a = aNew;

        /* 更新残差方差 */
        sigma2 *= (1.0 - km * km);
        if (sigma2 < 1e-15) sigma2 = 1e-15;
    }

    for (int i = 0; i < order; ++i)
        coeffs[i] = a[i + 1];

    m_stats.totalEstimates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEstimates + m_stats.totalPredictions);

    emit estimateCompleted(order, sigma2);
    return coeffs;
}

QPair<int, QVector<double>> YuleWalker::autoFit(
    const QVector<double>& signal, int maxOrder)
{
    int n = signal.size();
    int bestOrder = 1;
    double bestAic = 1e300;
    QVector<double> bestCoeffs;

    for (int p = 1; p <= qMin(maxOrder, n / 2); ++p) {
        QVector<double> coeffs = estimate(signal, p);

        /* 计算残差方差 */
        double sigma2 = 0.0;
        for (int i = p; i < n; ++i) {
            double pred = 0.0;
            for (int j = 0; j < p; ++j)
                pred += coeffs[j] * signal[i - 1 - j];
            double err = signal[i] - pred;
            sigma2 += err * err;
        }
        sigma2 /= (n - p);
        if (sigma2 < 1e-15) sigma2 = 1e-15;

        double currentAic = aic(n, p, sigma2);
        if (currentAic < bestAic) {
            bestAic = currentAic;
            bestOrder = p;
            bestCoeffs = coeffs;
        }
    }

    return {bestOrder, bestCoeffs};
}

QVector<double> YuleWalker::predict(const QVector<double>& signal,
                                     const QVector<double>& coeffs,
                                     int steps)
{
    QElapsedTimer timer;
    timer.start();

    int p = coeffs.size();
    QVector<double> result(steps, 0.0);
    QVector<double> extended = signal;

    for (int s = 0; s < steps; ++s) {
        double val = 0.0;
        int start = extended.size() - 1;
        for (int j = 0; j < p && start - j >= 0; ++j)
            val += coeffs[j] * extended[start - j];
        result[s] = val;
        extended.append(val);
    }

    m_stats.totalPredictions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEstimates + m_stats.totalPredictions);

    emit predictionCompleted(steps);
    return result;
}

double YuleWalker::aic(int n, int order, double sigma2) const
{
    return n * std::log(sigma2) + 2.0 * order;
}

void YuleWalker::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
