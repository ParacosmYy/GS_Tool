/**
 * @file GoodnessOfFit.cpp
 * @brief 拟合优度检验实现
 */

#include "utils/gof/GoodnessOfFit.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

GoodnessOfFit::GoodnessOfFit(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

GoodnessOfFit::TestResult GoodnessOfFit::chiSquareTest(
    const QVector<double>& observed, const QVector<double>& expected,
    double significance)
{
    QElapsedTimer timer;
    timer.start();

    TestResult result;
    result.significance = significance;
    result.testName = tr("卡方拟合检验");

    int n = qMin(observed.size(), expected.size());
    if (n < 2) {
        m_stats.totalTests++;
        return result;
    }

    double chi2 = 0.0;
    int df = 0;
    for (int i = 0; i < n; ++i) {
        if (expected[i] > 0) {
            double diff = observed[i] - expected[i];
            chi2 += diff * diff / expected[i];
            df++;
        }
    }
    df = qMax(df - 1, 1);

    result.statistic = chi2;
    result.pValue = 1.0 - chiSquareCDF(chi2, df);
    result.rejected = (result.pValue < significance);

    if (result.rejected) m_stats.totalRejections++;
    else m_stats.totalAcceptances++;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTests, 1ULL);

    emit testCompleted(result.testName, chi2, result.pValue);
    return result;
}

GoodnessOfFit::TestResult GoodnessOfFit::ksNormalityTest(
    const QVector<double>& data, double significance)
{
    QElapsedTimer timer;
    timer.start();

    TestResult result;
    result.significance = significance;
    result.testName = tr("KS正态性检验");

    if (data.size() < 5) {
        m_stats.totalTests++;
        return result;
    }

    int n = data.size();
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    /* 计算均值和标准差 */
    double mean = 0.0;
    for (double v : sorted) mean += v;
    mean /= n;

    double var = 0.0;
    for (double v : sorted) { double d = v - mean; var += d * d; }
    var /= n;
    double stdDev = qSqrt(var);

    if (stdDev < 1e-15) {
        m_stats.totalTests++;
        return result;
    }

    /* KS统计量 */
    double dMax = 0.0;
    for (int i = 0; i < n; ++i) {
        double z = (sorted[i] - mean) / stdDev;
        double fEmpiricalUp = static_cast<double>(i + 1) / n;
        double fEmpiricalLo = static_cast<double>(i) / n;
        double fNormal = normalCDF(z);
        dMax = qMax(dMax, qAbs(fEmpiricalUp - fNormal));
        dMax = qMax(dMax, qAbs(fEmpiricalLo - fNormal));
    }

    result.statistic = dMax;
    /* KS分布近似p值 */
    double sqrtN = qSqrt(static_cast<double>(n));
    double lambda = (sqrtN + 0.12 + 0.11 / sqrtN) * dMax;
    result.pValue = 2.0 * qExp(-2.0 * lambda * lambda);
    result.pValue = qBound(0.0, result.pValue, 1.0);
    result.rejected = (result.pValue < significance);

    if (result.rejected) m_stats.totalRejections++;
    else m_stats.totalAcceptances++;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTests, 1ULL);

    emit testCompleted(result.testName, dMax, result.pValue);
    return result;
}

GoodnessOfFit::TestResult GoodnessOfFit::jarqueBeraTest(
    const QVector<double>& data, double significance)
{
    QElapsedTimer timer;
    timer.start();

    TestResult result;
    result.significance = significance;
    result.testName = tr("Jarque-Bera正态性检验");

    int n = data.size();
    if (n < 10) {
        m_stats.totalTests++;
        return result;
    }

    double mean = 0.0;
    for (double v : data) mean += v;
    mean /= n;

    double m2 = 0.0, m3 = 0.0, m4 = 0.0;
    for (double v : data) {
        double d = v - mean;
        m2 += d * d;
        m3 += d * d * d;
        m4 += d * d * d * d;
    }
    m2 /= n;
    m3 /= n;
    m4 /= n;

    double skewness = (m2 > 1e-30) ? m3 / qPow(m2, 1.5) : 0.0;
    double kurtosis = (m2 > 1e-30) ? m4 / (m2 * m2) - 3.0 : 0.0;

    double jb = (static_cast<double>(n) / 6.0) *
        (skewness * skewness + kurtosis * kurtosis / 4.0);

    result.statistic = jb;
    /* JB近似服从χ²(2) */
    result.pValue = 1.0 - chiSquareCDF(jb, 2);
    result.rejected = (result.pValue < significance);

    if (result.rejected) m_stats.totalRejections++;
    else m_stats.totalAcceptances++;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTests, 1ULL);

    emit testCompleted(result.testName, jb, result.pValue);
    return result;
}

double GoodnessOfFit::normalCDF(double z) const
{
    /* Abramowitz & Stegun 近似 */
    if (z < -8.0) return 0.0;
    if (z > 8.0) return 1.0;

    double t = 1.0 / (1.0 + 0.2316419 * qAbs(z));
    double d = 0.3989422804014327;
    double p = d * qExp(-z * z / 2.0) *
        (t * (0.319381530 + t * (-0.356563782 + t *
        (1.781477937 + t * (-1.821255978 + t * 1.330274429)))));

    return (z > 0) ? 1.0 - p : p;
}

double GoodnessOfFit::chiSquareCDF(double x, int df) const
{
    if (x <= 0 || df < 1) return 0.0;
    return lowerIncompleteGamma(static_cast<double>(df) / 2.0, x / 2.0) /
           gammaFunc(static_cast<double>(df) / 2.0);
}

double GoodnessOfFit::lowerIncompleteGamma(double s, double x) const
{
    /* 级数展开 */
    double sum = 1.0 / s;
    double term = 1.0 / s;
    for (int n = 1; n < 200; ++n) {
        term *= x / (s + n);
        sum += term;
        if (qAbs(term) < 1e-12 * qAbs(sum)) break;
    }
    return qPow(x, s) * qExp(-x) * sum;
}

double GoodnessOfFit::gammaFunc(double x) const
{
    if (x <= 0) return 1.0;
    if (x < 0.5) {
        return M_PI / (qSin(M_PI * x) * gammaFunc(1.0 - x));
    }
    x -= 1.0;
    double g = 0.99999999999980993 +
        676.5203681218851 / (x + 1) - 1259.1392167224028 / (x + 2) +
        771.32342877765313 / (x + 3) - 176.61502916214059 / (x + 4) +
        12.507343278686905 / (x + 5) - 0.13857109526572012 / (x + 6) +
        9.9843695780195716e-6 / (x + 7) + 1.5056327351493116e-7 / (x + 8);

    double t = x + 7.5;
    return qSqrt(2.0 * M_PI) * qPow(t, x + 0.5) * qExp(-t) * g;
}

void GoodnessOfFit::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
