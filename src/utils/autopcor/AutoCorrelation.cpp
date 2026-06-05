/**
 * @file AutoCorrelation.cpp
 * @brief 自相关分析器实现
 */

#include "utils/autopcor/AutoCorrelation.h"

#include <QElapsedTimer>
#include <QtMath>

AutoCorrelation::AutoCorrelation(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

AutoCorrelation::AcfResult AutoCorrelation::compute(
    const QVector<double>& data, int maxLag)
{
    QElapsedTimer timer;
    timer.start();

    AcfResult result;
    int n = data.size();
    if (n < 2) {
        m_stats.totalComputations++;
        return result;
    }

    if (maxLag <= 0) maxLag = n / 2;
    maxLag = qMin(maxLag, n - 1);

    /* 计算均值 */
    double mean = 0.0;
    for (double v : data) mean += v;
    mean /= n;

    /* 计算方差 */
    double var = 0.0;
    for (double v : data) { double d = v - mean; var += d * d; }

    result.acf.resize(maxLag + 1);
    result.acf[0] = var;

    for (int lag = 1; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i) {
            sum += (data[i] - mean) * (data[i + lag] - mean);
        }
        result.acf[lag] = sum;
    }

    /* 归一化 */
    if (var > 1e-15) {
        for (int lag = 0; lag <= maxLag; ++lag) {
            result.acf[lag] /= var;
        }
    }

    /* 找基频: 第一个显著峰(在lag>0的范围内) */
    double maxVal = -2.0;
    int bestLag = 0;
    for (int lag = 1; lag <= maxLag; ++lag) {
        if (result.acf[lag] > maxVal) {
            maxVal = result.acf[lag];
            bestLag = lag;
        }
    }
    result.fundamentalLag = bestLag;
    result.periodicity = qMax(0.0, maxVal);

    m_stats.totalComputations++;
    m_stats.totalLagsProcessed += maxLag;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalComputations, 1ULL);

    emit computationCompleted(bestLag, result.fundamentalFreq);
    return result;
}

QVector<double> AutoCorrelation::computeNormalized(
    const QVector<double>& data, int maxLag)
{
    AcfResult result = compute(data, maxLag);
    return result.acf;
}

double AutoCorrelation::detectPeriodicity(
    const QVector<double>& data, double sampleRate)
{
    if (data.size() < 4) return 0.0;

    AcfResult acf = compute(data, data.size() / 2);

    /* 在lag > 0范围内找第一个过零点后的最大峰 */
    int n = acf.acf.size();
    double maxPeak = 0.0;
    for (int lag = 1; lag < n; ++lag) {
        if (acf.acf[lag] > maxPeak) {
            maxPeak = acf.acf[lag];
        }
    }

    return qBound(0.0, maxPeak, 1.0);
}

void AutoCorrelation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
