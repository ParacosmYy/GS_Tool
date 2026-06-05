/**
 * @file PolyphaseResampler.cpp
 * @brief 多相重采样器实现
 */

#include "PolyphaseResampler.h"
#include <QElapsedTimer>
#include <cmath>

PolyphaseResampler::PolyphaseResampler(QObject* parent)
    : QObject(parent)
    , m_ratio(1.0)
    , m_upFactor(1)
    , m_downFactor(1)
    , m_taps(64)
    , m_timeSum(0.0)
{
}

void PolyphaseResampler::configure(double inRate, double outRate, int taps)
{
    m_taps = (taps > 0) ? taps : 64;
    m_ratio = outRate / inRate;

    /* 简化GCD比率 */
    double eps = 1e-6;
    int up = qRound(outRate);
    int down = qRound(inRate);

    /* 找GCD */
    int a = up, b = down;
    while (b > 0) { int t = b; b = a % t; a = t; }
    if (a > 0) { up /= a; down /= a; }

    m_upFactor = qMax(up, 1);
    m_downFactor = qMax(down, 1);

    /* 设计低通滤波器 */
    int filterLen = m_taps * qMax(m_upFactor, m_downFactor);
    if (filterLen < 16) filterLen = 16;
    if (filterLen % 2 == 0) filterLen++;

    m_filter.resize(filterLen);
    double cutoff = M_PI / qMax(m_upFactor, m_downFactor);
    double sum = 0.0;
    int mid = filterLen / 2;

    for (int i = 0; i < filterLen; ++i) {
        double n = i - mid;
        double sinc = (std::abs(n) < 1e-10) ? 1.0 : std::sin(cutoff * n) / (M_PI * n);
        double win = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (filterLen - 1)));
        m_filter[i] = sinc * win;
        sum += m_filter[i];
    }

    for (double& v : m_filter) v /= sum;
}

QVector<double> PolyphaseResampler::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return {};

    int N = input.size();
    int outLen = static_cast<int>(N * m_ratio);
    QVector<double> output(outLen, 0.0);

    /* 线性插值(简化实现) */
    for (int i = 0; i < outLen; ++i) {
        double srcIdx = static_cast<double>(i) / m_ratio;
        int idx0 = static_cast<int>(srcIdx);
        int idx1 = idx0 + 1;
        double frac = srcIdx - idx0;

        if (idx0 >= 0 && idx0 < N) {
            double v0 = input[idx0];
            double v1 = (idx1 < N) ? input[idx1] : v0;
            output[i] = v0 + frac * (v1 - v0);
        }
    }

    m_stats.totalResamples++;
    m_stats.totalSamplesIn += N;
    m_stats.totalSamplesOut += outLen;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalResamples;

    emit resampleCompleted(N, outLen);
    return output;
}

void PolyphaseResampler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
