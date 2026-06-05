/**
 * @file LombScargle.cpp
 * @brief Lomb-Scargle周期图实现
 */

#include "utils/lomb2/LombScargle.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

LombScargle::LombScargle(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<double> LombScargle::compute(
    const QVector<double>& times,
    const QVector<double>& values,
    const QVector<double>& freqs)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(times.size(), values.size());
    QVector<double> power(freqs.size(), 0.0);

    if (n < 2) {
        m_stats.totalComputations++;
        emit periodogramComputed(0);
        return power;
    }

    /* 计算均值 */
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += values[i];
    mean /= n;

    /* 计算方差 */
    double var = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = values[i] - mean;
        var += d * d;
    }
    var /= n;

    for (int f = 0; f < freqs.size(); ++f) {
        double omega = 2.0 * M_PI * freqs[f];

        /* 计算tau(时间偏移) */
        double sinSum = 0.0, cosSum = 0.0;
        for (int i = 0; i < n; ++i) {
            sinSum += std::sin(2.0 * omega * times[i]);
            cosSum += std::cos(2.0 * omega * times[i]);
        }
        double tau = std::atan2(sinSum, cosSum) / (2.0 * omega);

        /* 计算Lomb-Scargle功率 */
        double sumCos = 0.0, sumSin = 0.0;
        double cos2 = 0.0, sin2 = 0.0;

        for (int i = 0; i < n; ++i) {
            double phase = omega * (times[i] - tau);
            double c = std::cos(phase);
            double s = std::sin(phase);
            sumCos += (values[i] - mean) * c;
            sumSin += (values[i] - mean) * s;
            cos2 += c * c;
            sin2 += s * s;
        }

        power[f] = (var > 1e-15)
            ? ((sumCos * sumCos / cos2 + sumSin * sumSin / sin2) / (2.0 * var))
            : 0.0;
    }

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit periodogramComputed(freqs.size());
    return power;
}

QVector<double> LombScargle::autoFrequency(
    const QVector<double>& times,
    double minFreq, double maxFreq, int numFreqs)
{
    if (times.size() < 2 || numFreqs < 1) return {};

    if (maxFreq <= 0) {
        double minDt = 1e300;
        for (int i = 1; i < times.size(); ++i) {
            double dt = times[i] - times[i - 1];
            if (dt > 0 && dt < minDt) minDt = dt;
        }
        maxFreq = (minDt < 1e300) ? 0.5 / minDt : 1.0;
    }

    if (minFreq <= 0) {
        double span = times.last() - times.first();
        minFreq = (span > 0) ? 1.0 / span : 0.01;
    }

    QVector<double> freqs(numFreqs);
    double step = (maxFreq - minFreq) / qMax(1, numFreqs - 1);
    for (int i = 0; i < numFreqs; ++i)
        freqs[i] = minFreq + i * step;

    return freqs;
}

QVector<QPair<double, double>> LombScargle::findPeaks(
    const QVector<double>& power,
    const QVector<double>& freqs,
    double threshold)
{
    int n = qMin(power.size(), freqs.size());
    if (n < 3) return {};

    /* 找最大值 */
    double maxPow = *std::max_element(power.begin(), power.end());
    double threshVal = maxPow * threshold;

    QVector<QPair<double, double>> peaks;
    for (int i = 1; i < n - 1; ++i) {
        if (power[i] > power[i - 1] && power[i] > power[i + 1] &&
            power[i] >= threshVal) {
            peaks.append({freqs[i], power[i]});
        }
    }

    /* 按功率降序排列 */
    std::sort(peaks.begin(), peaks.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    return peaks;
}

void LombScargle::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
