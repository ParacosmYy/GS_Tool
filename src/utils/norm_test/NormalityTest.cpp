/**
 * @file NormalityTest.cpp
 * @brief 正态性检验工具实现
 */

#include "NormalityTest.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

NormalityTest::NormalityTest(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

NormalityTest::TestResult NormalityTest::shapiroWilk(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();
    TestResult result;
    result.testName = "Shapiro-Wilk";
    result.sampleSize = data.size();

    if (data.size() < 3) {
        result.timeMs = 0;
        return result;
    }

    QVector<double> sorted = sortData(data);
    int n = sorted.size();
    double mean = computeMean(sorted);
    double s2 = 0.0;
    for (int i = 0; i < n; ++i)
        s2 += (sorted[i] - mean) * (sorted[i] - mean);

    if (s2 < 1e-15) {
        result.statistic = 1.0;
        result.pValue = 1.0;
        result.isNormal = true;
        return result;
    }

    /* 简化Shapiro-Wilk系数近似 */
    double b = 0.0;
    int m = n / 2;
    for (int i = 0; i < m; ++i) {
        double a_i = (static_cast<double>(n) - 2 * i - 1) / n;
        b += a_i * (sorted[n - 1 - i] - sorted[i]);
    }

    double W = (b * b) / s2;
    W = qBound(0.0, W, 1.0);

    /* p值近似: Royston方法简化 */
    double ln_n = std::log(static_cast<double>(n));
    double mu = -1.2725 + 1.0521 * ln_n;
    double sigma = 1.0308 - 0.26758 * ln_n;
    double z = (std::log(1.0 - W) - mu) / sigma;
    result.pValue = 1.0 - normalCDF(z);

    result.statistic = W;
    result.isNormal = (result.pValue > 0.05);

    m_stats.totalTests++;
    if (result.isNormal) m_stats.totalNormalDetected++;
    else m_stats.totalNonNormalDetected++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(result.testName, result.isNormal);
    return result;
}

NormalityTest::TestResult NormalityTest::jarqueBera(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();
    TestResult result;
    result.testName = "Jarque-Bera";
    result.sampleSize = data.size();

    if (data.size() < 3) return result;

    int n = data.size();
    double mean = computeMean(data);
    double var = computeVariance(data, mean);
    if (var < 1e-15) { result.statistic = 0.0; result.pValue = 1.0; result.isNormal = true; return result; }

    double stdDev = std::sqrt(var);
    double skew = computeSkewness(data, mean, stdDev);
    double kurt = computeKurtosis(data, mean, stdDev);

    double JB = (static_cast<double>(n) / 6.0) *
                (skew * skew + 0.25 * (kurt - 3.0) * (kurt - 3.0));

    result.statistic = JB;
    result.pValue = 1.0 - chi2CDF(JB, 2);
    result.isNormal = (result.pValue > 0.05);

    m_stats.totalTests++;
    if (result.isNormal) m_stats.totalNormalDetected++;
    else m_stats.totalNonNormalDetected++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(result.testName, result.isNormal);
    return result;
}

NormalityTest::TestResult NormalityTest::andersonDarling(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();
    TestResult result;
    result.testName = "Anderson-Darling";
    result.sampleSize = data.size();

    if (data.size() < 5) return result;

    int n = data.size();
    QVector<double> sorted = sortData(data);
    double mean = computeMean(sorted);
    double stdDev = std::sqrt(computeVariance(sorted, mean));
    if (stdDev < 1e-15) { result.statistic = 0.0; result.pValue = 1.0; result.isNormal = true; return result; }

    double S = 0.0;
    for (int i = 0; i < n; ++i) {
        double zi = (sorted[i] - mean) / stdDev;
        double cdf_i = normalCDF(zi);
        double cdf_ni = normalCDF((sorted[n - 1 - i] - mean) / stdDev);
        cdf_i = qBound(1e-10, cdf_i, 1.0 - 1e-10);
        cdf_ni = qBound(1e-10, cdf_ni, 1.0 - 1e-10);
        S += (2.0 * (i + 1) - 1.0) * (std::log(cdf_i) + std::log(1.0 - cdf_ni));
    }

    double A2 = -n - S / n;
    /* 修正系数 */
    A2 *= (1.0 + 0.75 / n + 2.25 / (n * n));

    result.statistic = A2;

    /* p值近似 */
    if (A2 >= 0.6) result.pValue = std::exp(1.2937 - 5.709 * A2 + 0.0186 * A2 * A2);
    else if (A2 >= 0.34) result.pValue = std::exp(0.9177 - 4.279 * A2 - 1.38 * A2 * A2);
    else result.pValue = 1.0 - std::exp(-8.318 + 42.796 * A2 - 59.938 * A2 * A2);

    result.pValue = qBound(0.0, result.pValue, 1.0);
    result.isNormal = (result.pValue > 0.05);

    m_stats.totalTests++;
    if (result.isNormal) m_stats.totalNormalDetected++;
    else m_stats.totalNonNormalDetected++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(result.testName, result.isNormal);
    return result;
}

NormalityTest::TestResult NormalityTest::dagostinoK2(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();
    TestResult result;
    result.testName = "D'Agostino K2";
    result.sampleSize = data.size();

    if (data.size() < 20) return result;

    int n = data.size();
    double mean = computeMean(data);
    double stdDev = std::sqrt(computeVariance(data, mean));
    if (stdDev < 1e-15) { result.statistic = 0.0; result.pValue = 1.0; result.isNormal = true; return result; }

    double skew = computeSkewness(data, mean, stdDev);
    double kurt = computeKurtosis(data, mean, stdDev);

    /* 偏度检验 */
    double Y = skew * std::sqrt(static_cast<double>(n) / 6.0);
    double beta2 = 3.0 * (n * n + 27 * n - 70) * (n + 1) * (n + 3) /
                   ((n - 2) * (n + 5) * (n + 7) * (n + 9));
    double W2 = -1.0 + std::sqrt(2.0 * (beta2 - 1.0));
    double delta = 1.0 / std::sqrt(std::log(std::sqrt(W2)));
    double alpha = std::sqrt(2.0 / (W2 - 1.0));
    double z1 = delta * std::log(Y / alpha + std::sqrt(Y * Y / (alpha * alpha) + 1.0));

    /* 峰度检验 */
    double kurt_excess = kurt - 3.0;
    double z2 = kurt_excess * std::sqrt(static_cast<double>(n) / 24.0);

    double K2 = z1 * z1 + z2 * z2;

    result.statistic = K2;
    result.pValue = 1.0 - chi2CDF(K2, 2);
    result.isNormal = (result.pValue > 0.05);

    m_stats.totalTests++;
    if (result.isNormal) m_stats.totalNormalDetected++;
    else m_stats.totalNonNormalDetected++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(result.testName, result.isNormal);
    return result;
}

QVector<NormalityTest::TestResult> NormalityTest::comprehensiveTest(
    const QVector<double>& data)
{
    QVector<TestResult> results;
    results.append(shapiroWilk(data));
    results.append(jarqueBera(data));
    if (data.size() >= 5) results.append(andersonDarling(data));
    if (data.size() >= 20) results.append(dagostinoK2(data));
    return results;
}

double NormalityTest::computeMean(const QVector<double>& data) const
{
    double sum = 0.0;
    for (double v : data) sum += v;
    return sum / data.size();
}

double NormalityTest::computeVariance(const QVector<double>& data,
                                       double mean) const
{
    double sum = 0.0;
    for (double v : data) sum += (v - mean) * (v - mean);
    return sum / data.size();
}

double NormalityTest::computeSkewness(const QVector<double>& data,
                                       double mean, double stdDev) const
{
    double m3 = 0.0;
    for (double v : data) {
        double d = (v - mean) / stdDev;
        m3 += d * d * d;
    }
    return m3 / data.size();
}

double NormalityTest::computeKurtosis(const QVector<double>& data,
                                       double mean, double stdDev) const
{
    double m4 = 0.0;
    for (double v : data) {
        double d = (v - mean) / stdDev;
        m4 += d * d * d * d;
    }
    return m4 / data.size();
}

double NormalityTest::normalCDF(double x) const
{
    /* Abramowitz & Stegun近似 */
    double a1 = 0.254829592, a2 = -0.284496736, a3 = 1.421413741;
    double a4 = -1.453152027, a5 = 1.061405429;
    double p = 0.3275911;
    int sign = (x < 0) ? -1 : 1;
    x = std::abs(x) / std::sqrt(2.0);
    double t = 1.0 / (1.0 + p * x);
    double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t *
               std::exp(-x * x);
    return 0.5 * (1.0 + sign * y);
}

double NormalityTest::chi2CDF(double x, int df) const
{
    if (x <= 0) return 0.0;
    /* 不完全Gamma函数近似 */
    double k = df / 2.0;
    double z = x / 2.0;
    double sum = 0.0, term = 1.0;
    for (int i = 0; i < 200; ++i) {
        sum += term;
        term *= z / (k + i + 1);
        if (term < 1e-15) break;
    }
    double result = 1.0 - sum * std::exp(-z) * std::pow(z, k) / std::tgamma(k);
    return qBound(0.0, result, 1.0);
}

QVector<double> NormalityTest::sortData(const QVector<double>& data) const
{
    QVector<double> s = data;
    std::sort(s.begin(), s.end());
    return s;
}

NormalityTest::Stats NormalityTest::stats() const { return m_stats; }

void NormalityTest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
