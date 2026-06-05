/**
 * @file SignalDecomposer.cpp
 * @brief 信号分解引擎实现 — 趋势+季节+残差分解
 */

#include "utils/decomposer/SignalDecomposer.h"
#include <QtMath>
#include <algorithm>

SignalDecomposer::SignalDecomposer(QObject* parent)
    : QObject(parent), m_period(12), m_trendSum(0.0), m_seasonalSum(0.0) {}

void SignalDecomposer::setPeriod(int period) { m_period = qMax(2, period); }

SignalDecomposer::DecompositionResult SignalDecomposer::decompose(
    const QVector<double>& data)
{
    DecompositionResult result;
    if (data.size() < m_period * 2) {
        result.trend = result.seasonal = result.residual = data;
        return result;
    }

    int n = data.size();
    /* 步骤1: 提取趋势(移动平均) */
    result.trend = movingAverage(data, m_period);

    /* 步骤2: 去趋势 */
    QVector<double> detrended(n);
    for (int i = 0; i < n; ++i) {
        detrended[i] = data[i] - result.trend[i];
    }

    /* 步骤3: 提取季节分量(周期平均) */
    result.seasonal.resize(n);
    QVector<double> seasonalAvg(m_period, 0.0);
    QVector<int> counts(m_period, 0);
    for (int i = 0; i < n; ++i) {
        int bin = i % m_period;
        seasonalAvg[bin] += detrended[i];
        ++counts[bin];
    }
    double seasonalSum = 0.0;
    for (int j = 0; j < m_period; ++j) {
        if (counts[j] > 0) seasonalAvg[j] /= counts[j];
        seasonalSum += seasonalAvg[j];
    }
    /* 零均值化季节分量 */
    double seasonalMean = seasonalSum / m_period;
    for (int j = 0; j < m_period; ++j) {
        seasonalAvg[j] -= seasonalMean;
    }
    for (int i = 0; i < n; ++i) {
        result.seasonal[i] = seasonalAvg[i % m_period];
    }

    /* 步骤4: 残差 */
    result.residual.resize(n);
    for (int i = 0; i < n; ++i) {
        result.residual[i] = data[i] - result.trend[i] - result.seasonal[i];
    }

    /* 更新统计 */
    ++m_stats.totalDecompositions;
    double trendVar = 0.0, seasonalVar = 0.0, totalVar = 0.0;
    double mean = 0.0;
    for (double v : data) mean += v;
    mean /= n;
    for (int i = 0; i < n; ++i) {
        double d = data[i] - mean;
        totalVar += d * d;
        double td = result.trend[i] - mean;
        trendVar += td * td;
        double sd = result.seasonal[i];
        seasonalVar += sd * sd;
    }
    if (totalVar > 0) {
        m_trendSum += trendVar / totalVar;
        m_seasonalSum += seasonalVar / totalVar;
    }
    m_stats.averageTrendStrength = m_trendSum
        / static_cast<double>(m_stats.totalDecompositions);
    m_stats.averageSeasonalStrength = m_seasonalSum
        / static_cast<double>(m_stats.totalDecompositions);
    if (m_stats.peakPeriod == 0) m_stats.peakPeriod = m_period;

    emit decompositionComplete(n);
    return result;
}

int SignalDecomposer::detectPeriod(const QVector<double>& data) const
{
    if (data.size() < 10) return 2;
    int bestPeriod = 2;
    double bestCorr = 0.0;
    int maxP = qMin(data.size() / 3, 200);
    double mean = 0.0;
    for (double v : data) mean += v;
    mean /= data.size();
    double var = 0.0;
    for (double v : data) { double d = v - mean; var += d * d; }
    if (var < 1e-10) return 2;

    for (int p = 2; p <= maxP; ++p) {
        double cov = 0.0;
        for (int i = 0; i < data.size() - p; ++i) {
            cov += (data[i] - mean) * (data[i + p] - mean);
        }
        double r = cov / var;
        if (r > bestCorr) { bestCorr = r; bestPeriod = p; }
    }
    return bestPeriod;
}

void SignalDecomposer::resetStatistics()
{
    m_stats = Stats{};
    m_trendSum = m_seasonalSum = 0.0;
}

QVector<double> SignalDecomposer::movingAverage(
    const QVector<double>& data, int window) const
{
    int n = data.size();
    QVector<double> result(n, 0.0);
    double sum = 0.0;
    int half = window / 2;
    for (int i = 0; i < n; ++i) {
        int start = qMax(0, i - half);
        int end = qMin(n - 1, i + half);
        sum = 0.0;
        for (int j = start; j <= end; ++j) sum += data[j];
        result[i] = sum / (end - start + 1);
    }
    return result;
}
