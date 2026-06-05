/**
 * @file AutoCorrelator.cpp
 * @brief 自相关引擎实现 — 标准/有偏/无偏自相关+周期性检测
 */

#include "utils/autocorr/AutoCorrelator.h"

#include <QtMath>
#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
AutoCorrelator::AutoCorrelator(QObject* parent)
    : QObject(parent)
    , m_method(Method::Standard)
    , m_maxLag(256)
{
}

void AutoCorrelator::setMethod(Method method) { m_method = method; }

/** @brief 设置最大滞后 @param maxLag 最大滞后 */
void AutoCorrelator::setMaxLag(int maxLag)
{
    m_maxLag = qMax(1, maxLag);
}

/** @brief 计算自相关 @param data 数据 @return 结果 */
AutoCorrelator::CorrelationResult AutoCorrelator::compute(
    const QVector<double>& data)
{
    CorrelationResult result;
    if (data.size() < 2) return result;

    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    int maxLag = qMin(m_maxLag, n - 1);

    /* 计算均值 */
    double mean = 0.0;
    for (double v : data) mean += v;
    mean /= n;

    /* 计算方差 */
    double var = 0.0;
    for (double v : data) {
        double d = v - mean;
        var += d * d;
    }

    result.lags.resize(maxLag + 1);
    result.values.resize(maxLag + 1);

    for (int k = 0; k <= maxLag; ++k) {
        result.lags[k] = static_cast<double>(k);

        double sum = 0.0;
        for (int i = 0; i < n - k; ++i) {
            sum += (data[i] - mean) * (data[i + k] - mean);
        }

        switch (m_method) {
        case Method::Standard:
        case Method::FFT:
            result.values[k] = sum / n;
            break;
        case Method::Biased:
            result.values[k] = sum / n;
            break;
        case Method::Unbiased:
            result.values[k] = sum / qMax(1, n - k);
            break;
        }
    }

    /* 归一化(滞后0处为1) */
    if (result.values[0] != 0.0) {
        double norm = result.values[0];
        for (auto& v : result.values) v /= norm;
    }

    /* 找峰值(跳过滞后0) */
    double peak = -2.0;
    int peakK = 0;
    for (int k = 1; k <= maxLag; ++k) {
        if (result.values[k] > peak) {
            peak = result.values[k];
            peakK = k;
        }
    }
    result.peakLag = peakK;
    result.peakValue = peak;

    /* 更新统计 */
    ++m_stats.totalComputations;
    m_stats.totalPointsProcessed += static_cast<quint64>(n);
    if (peak > m_stats.peakCorrelation) m_stats.peakCorrelation = peak;

    emit computationComplete(maxLag + 1, peak);
    return result;
}

/** @brief 检测周期性 @param data 数据 @return 是否周期性 */
bool AutoCorrelator::detectPeriodicity(const QVector<double>& data)
{
    if (data.size() < 4) return false;

    CorrelationResult result = compute(data);

    /* 峰值相关超过阈值认为周期性 */
    if (result.peakValue > 0.5 && result.peakLag > 0) {
        ++m_stats.totalPeriodicDetected;
        return true;
    }
    return false;
}

/** @brief 估计基频 @param data 数据 @param sampleRate 采样率 @return 基频 */
double AutoCorrelator::findFundamentalFrequency(
    const QVector<double>& data, double sampleRate)
{
    if (data.size() < 4 || sampleRate <= 0) return 0.0;

    CorrelationResult result = compute(data);

    if (result.peakLag <= 0) return 0.0;
    return sampleRate / static_cast<double>(result.peakLag);
}

/** @brief 重置统计 */
void AutoCorrelator::resetStatistics()
{
    m_stats = Stats{};
}
