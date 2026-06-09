/**
 * @file WaveletDenoiser14.cpp
 * @brief WaveletDenoiser14 实现
 *
 * 实现小波去噪器：硬软混合阈值尺度间相关性信号主导重建。
 */

#include "utils/signal262/WaveletDenoiser14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser14::WaveletDenoiser14(QObject *parent)
    : QObject(parent) {}
WaveletDenoiser14::~WaveletDenoiser14() = default;

/* ---- Configuration ---- */

void WaveletDenoiser14::setLevels(int levels) { m_levels = qMax(1, levels); }
void WaveletDenoiser14::setThresholdMode(ThresholdMode mode) { m_mode = mode; }
void WaveletDenoiser14::setThresholdScale(double scale) { m_thresholdScale = qMax(0.01, scale); }

/* ---- Haar wavelet forward (1 level) ---- */

void WaveletDenoiser14::haarForward(const QVector<double>& input,
                                      QVector<double>& approx,
                                      QVector<double>& detail) const
{
    int n = input.size() / 2;
    approx.resize(n);
    detail.resize(n);
    for (int i = 0; i < n; ++i) {
        approx[i] = (input[2 * i] + input[2 * i + 1]) / qSqrt(2.0);
        detail[i] = (input[2 * i] - input[2 * i + 1]) / qSqrt(2.0);
    }
}

/* ---- Haar wavelet inverse (1 level) ---- */

QVector<double> WaveletDenoiser14::haarInverse(const QVector<double>& approx,
                                                  const QVector<double>& detail) const
{
    int n = approx.size();
    QVector<double> output(2 * n);
    for (int i = 0; i < n; ++i) {
        output[2 * i] = (approx[i] + detail[i]) / qSqrt(2.0);
        output[2 * i + 1] = (approx[i] - detail[i]) / qSqrt(2.0);
    }
    return output;
}

/* ---- Universal threshold (VisuShrink) ---- */

double WaveletDenoiser14::universalThreshold(double sigma, int n) const
{
    return sigma * qSqrt(2.0 * qLn(qMax(n, 1))) * m_thresholdScale;
}

/* ---- Apply hybrid threshold ---- */

void WaveletDenoiser14::applyThreshold(QVector<double>& coeffs, double threshold) const
{
    for (int i = 0; i < coeffs.size(); ++i) {
        double c = coeffs[i];
        double absC = qAbs(c);
        switch (m_mode) {
        case Soft:
            if (absC <= threshold) {
                coeffs[i] = 0.0;
            } else {
                coeffs[i] = (c > 0 ? 1.0 : -1.0) * (absC - threshold);
            }
            break;
        case Hard:
            if (absC <= threshold) coeffs[i] = 0.0;
            break;
        case Hybrid: {
            // Hybrid: soft for small coefficients, hard for large
            double boundary = 2.0 * threshold;
            if (absC <= threshold) {
                coeffs[i] = 0.0;
            } else if (absC <= boundary) {
                // Transition zone: interpolated between soft and hard
                double t = (absC - threshold) / threshold;
                double softVal = absC - threshold;
                coeffs[i] = (c > 0 ? 1.0 : -1.0) * (t * absC + (1.0 - t) * softVal);
            }
            // else: keep (hard)
            break;
        }
        }
    }
}

/* ---- Inter-scale correlation ---- */

QVector<double> WaveletDenoiser14::interScaleCorrelation(const QVector<double>& parent,
                                                            const QVector<double>& child) const
{
    // Upsample parent to child size via replication and compute pointwise product
    QVector<double> corr(child.size(), 0.0);
    int ratio = child.size() / qMax(parent.size(), 1);
    if (ratio < 1) ratio = 1;
    for (int i = 0; i < child.size(); ++i) {
        int pIdx = qMin(i / ratio, parent.size() - 1);
        if (pIdx >= 0 && pIdx < parent.size())
            corr[i] = qAbs(parent[pIdx] * child[i]);
    }
    return corr;
}

/* ---- Estimate noise (MAD of finest detail) ---- */

double WaveletDenoiser14::estimateNoise(const QVector<double>& signal) const
{
    if (signal.size() < 4) return 0.0;
    QVector<double> approx, detail;
    haarForward(signal, approx, detail);
    // Median absolute deviation
    QVector<double> absDetail(detail.size());
    for (int i = 0; i < detail.size(); ++i) absDetail[i] = qAbs(detail[i]);
    std::sort(absDetail.begin(), absDetail.end());
    double median = absDetail[absDetail.size() / 2];
    return median / 0.6745;  // MAD to standard deviation conversion
}

/* ---- Denoise ---- */

QVector<double> WaveletDenoiser14::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    // Pad to power of 2
    int padded = 1;
    while (padded < n) padded <<= 1;
    QVector<double> data = signal;
    data.resize(padded, signal.last());

    // Forward wavelet decomposition
    QVector<QVector<double>> details(m_levels);
    QVector<double> current = data;
    for (int lev = 0; lev < m_levels; ++lev) {
        QVector<double> approx;
        haarForward(current, approx, details[lev]);
        current = approx;
    }

    // Estimate noise level from finest detail coefficients
    double sigma = estimateNoise(signal);

    // Process each level: threshold with inter-scale correlation
    for (int lev = m_levels - 1; lev >= 0; --lev) {
        double threshold = universalThreshold(sigma, details[lev].size());

        // Inter-scale correlation enhancement
        if (lev < m_levels - 1) {
            QVector<double> corr = interScaleCorrelation(details[lev + 1], details[lev]);
            // Adjust threshold based on correlation
            double corrSum = 0.0;
            for (double c : corr) corrSum += c;
            double avgCorr = corrSum / qMax(corr.size(), 1);
            if (avgCorr > sigma * sigma) {
                // High correlation: signal-dominated, reduce threshold
                threshold *= 0.5;
            }
        }

        applyThreshold(details[lev], threshold);
    }

    // Inverse wavelet reconstruction
    for (int lev = m_levels - 1; lev >= 0; --lev) {
        current = haarInverse(current, details[lev]);
    }

    // Trim to original length
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) result[i] = current[i];

    double elapsed = timer.elapsed();
    double outputNoise = estimateNoise(result);
    m_stats.signalLength = n;
    m_stats.decomposeLevels = m_levels;
    m_stats.inputSNR = (sigma > 0) ? 20.0 * qLn(norm(signal) / sigma) / M_LN10 : 0.0;
    m_stats.outputSNR = (outputNoise > 0) ? 20.0 * qLn(norm(result) / outputNoise) / M_LN10 : 0.0;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit denoisingCompleted(m_levels, sigma, outputNoise, elapsed);
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
