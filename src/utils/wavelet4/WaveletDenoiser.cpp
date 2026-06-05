/**
 * @file WaveletDenoiser.cpp
 * @brief 小波去噪器实现
 */

#include "WaveletDenoiser.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

WaveletDenoiser::WaveletDenoiser(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> WaveletDenoiser::denoise(const QVector<double>& signal,
                                          int level, ThresholdType type)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    /* 自动计算分解层数 */
    if (level <= 0) {
        level = static_cast<int>(std::log2(n));
        level = qBound(1, level, 10);
    }

    QVector<double> data = signal;
    int len = n;

    /* 多层分解 + 阈值处理 */
    QVector<double> detailCoeffs;

    for (int l = 0; l < level && len >= 4; ++l) {
        int half = len / 2;
        QVector<double> temp(len);

        /* Haar小波分解 */
        for (int i = 0; i < half; ++i) {
            temp[i] = (data[2 * i] + data[2 * i + 1]) / std::sqrt(2.0);
            temp[half + i] = (data[2 * i] - data[2 * i + 1]) / std::sqrt(2.0);
        }

        /* 提取细节系数用于阈值估计 */
        QVector<double> detail(half);
        for (int i = 0; i < half; ++i)
            detail[i] = temp[half + i];

        /* 阈值处理 */
        double sigma = estimateNoiseStd(detail);
        double thresh = sureThreshold(detail);
        thresh = qMax(thresh, universalThreshold(sigma, half) * 0.5);

        for (int i = 0; i < half; ++i)
            temp[half + i] = applyThreshold(temp[half + i], thresh, type);

        /* 复制回data */
        for (int i = 0; i < len; ++i)
            data[i] = temp[i];

        len = half;
    }

    /* 重构 */
    len = n;
    for (int l = level - 1; l >= 0; --l) {
        int currentLen = 1;
        for (int k = 0; k < l; ++k) currentLen *= 2;
        int half = currentLen;
        int full = half * 2;
        if (full > n) break;

        QVector<double> temp(full);
        for (int i = 0; i < half; ++i) {
            double approx = data[i];
            double detail = data[half + i];
            double s = std::sqrt(2.0);
            temp[2 * i] = (approx + detail) / s;
            temp[2 * i + 1] = (approx - detail) / s;
        }

        for (int i = 0; i < full; ++i)
            data[i] = temp[i];

        len = full;
    }

    /* 只保留原始长度 */
    data.resize(n);

    m_stats.totalDenoised++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoised;

    emit denoised(n, 0.0);
    return data;
}

double WaveletDenoiser::estimateNoiseStd(
    const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;

    /* MAD估计 */
    QVector<double> absCoeffs;
    absCoeffs.reserve(detailCoeffs.size());
    for (double c : detailCoeffs)
        absCoeffs.append(std::abs(c));
    std::sort(absCoeffs.begin(), absCoeffs.end());

    double median;
    int n = absCoeffs.size();
    if (n % 2 == 0)
        median = (absCoeffs[n / 2 - 1] + absCoeffs[n / 2]) / 2.0;
    else
        median = absCoeffs[n / 2];

    return median / 0.6745;
}

double WaveletDenoiser::universalThreshold(double sigma, int n) const
{
    return sigma * std::sqrt(2.0 * std::log(static_cast<double>(n)));
}

double WaveletDenoiser::sureThreshold(
    const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;

    int n = detailCoeffs.size();
    QVector<double> sorted;
    for (double c : detailCoeffs)
        sorted.append(std::abs(c));
    std::sort(sorted.begin(), sorted.end());

    double bestThreshold = sorted.last();
    double bestRisk = static_cast<double>(n);

    for (int i = 0; i < n; ++i) {
        double t = sorted[i];
        double risk = (n - 2.0 * (i + 1));
        for (int j = 0; j <= i; ++j)
            risk += sorted[j] * sorted[j] / (t * t + 1e-15) * t * t;
        /* 简化SURE公式 */
        double s0 = 0.0;
        for (int j = 0; j < n; ++j) {
            double ct = qMin(std::abs(detailCoeffs[j]), t);
            s0 += ct * ct;
        }
        risk = n - 2.0 * (n - i - 1) + s0;

        if (risk < bestRisk) {
            bestRisk = risk;
            bestThreshold = t;
        }
    }

    return bestThreshold;
}

double WaveletDenoiser::computeSNR(const QVector<double>& signal,
                                    const QVector<double>& noisy) const
{
    int n = qMin(signal.size(), noisy.size());
    if (n == 0) return 0.0;

    double signalPower = 0.0, noisePower = 0.0;
    for (int i = 0; i < n; ++i) {
        signalPower += signal[i] * signal[i];
        double diff = noisy[i] - signal[i];
        noisePower += diff * diff;
    }

    if (noisePower < 1e-15) return 100.0;
    return 10.0 * std::log10(signalPower / noisePower);
}

double WaveletDenoiser::applyThreshold(double coeff, double threshold,
                                         ThresholdType type) const
{
    double absC = std::abs(coeff);
    switch (type) {
    case Hard:
        return (absC >= threshold) ? coeff : 0.0;
    case Soft:
        if (absC <= threshold) return 0.0;
        return (coeff > 0) ? coeff - threshold : coeff + threshold;
    case SemiSoft: {
        double t2 = threshold * 2.0;
        if (absC <= threshold) return 0.0;
        if (absC >= t2) return coeff;
        double factor = (absC - threshold) / (t2 - threshold);
        return coeff * factor;
    }
    }
    return coeff;
}

WaveletDenoiser::Stats WaveletDenoiser::stats() const { return m_stats; }

void WaveletDenoiser::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
