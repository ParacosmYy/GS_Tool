/**
 * @file SlidingDFT.cpp
 * @brief 滑动DFT实现 — 逐样本更新 + 指数窗 + 幅度跟踪
 */

#include "utils/fft15/SlidingDFT.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/* ========== 构造/配置 ========== */

SlidingDFT::SlidingDFT(int fftSize, double sampleRate, QObject* parent)
    : QObject(parent), m_fftSize(fftSize), m_sampleRate(sampleRate),
      m_alpha(0.999), m_prevSample(0.0), m_timeSum(0.0)
{
    m_fftSize = qMax(4, m_fftSize);
    /* 确保是2的幂 */
    int power = 1;
    while (power < m_fftSize) power <<= 1;
    m_fftSize = power;

    precomputeTwiddles();
    reinitBins();
}

void SlidingDFT::setFFTSize(int size)
{
    int power = 1;
    while (power < qMax(4, size)) power <<= 1;
    m_fftSize = power;
    precomputeTwiddles();
    reinitBins();
}

void SlidingDFT::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

void SlidingDFT::setAlpha(double alpha)
{
    m_alpha = qBound(0.0, alpha, 1.0);
}

/* ========== 预计算 ========== */

void SlidingDFT::precomputeTwiddles()
{
    int halfN = m_fftSize / 2 + 1;
    m_twiddleReal.resize(halfN);
    m_twiddleImag.resize(halfN);

    for (int k = 0; k < halfN; ++k) {
        double angle = -2.0 * M_PI * k / m_fftSize;
        m_twiddleReal[k] = qCos(angle);
        m_twiddleImag[k] = qSin(angle);
    }
}

void SlidingDFT::reinitBins()
{
    int halfN = m_fftSize / 2 + 1;
    m_bins.resize(halfN);
    for (auto& b : m_bins) {
        b.real = 0.0;
        b.imag = 0.0;
        b.magnitude = 0.0;
        b.phase = 0.0;
    }
    m_prevSample = 0.0;
}

/* ========== 逐样本更新 ========== */

void SlidingDFT::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    /* 滑动DFT递推公式:
     * X_k[n] = alpha * (X_k[n-1] * W_N^k + (x[n] - x[n-N]))
     * 由于我们不存储环形缓冲区, 使用指数窗近似:
     * X_k[n] = alpha * X_k[n-1] * W_N^k + x[n]
     * 这给出一个带指数衰减的滑动DFT
     */
    int halfN = m_bins.size();
    for (int k = 0; k < halfN; ++k) {
        /* 旋转: X * W_N^k */
        double newReal = m_bins[k].real * m_twiddleReal[k]
                       - m_bins[k].imag * m_twiddleImag[k];
        double newImag = m_bins[k].real * m_twiddleImag[k]
                       + m_bins[k].imag * m_twiddleReal[k];

        /* 加入新样本 */
        newReal = m_alpha * newReal + sample;
        newImag = m_alpha * newImag;

        m_bins[k].real = newReal;
        m_bins[k].imag = newImag;
        m_bins[k].magnitude = qSqrt(newReal * newReal + newImag * newImag);
        m_bins[k].phase = qAtan2(newImag, newReal);
    }

    m_prevSample = sample;

    /* 统计更新 */
    ++m_stats.totalUpdates;
    ++m_stats.totalSamplesProcessed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalUpdates > 0) ? m_timeSum / m_stats.totalUpdates : 0.0;
}

void SlidingDFT::pushSamples(const QVector<double>& samples)
{
    for (double s : samples)
        pushSample(s);
    emit samplesProcessed(m_stats.totalSamplesProcessed);
}

/* ========== 查询 ========== */

SlidingDFT::BinState SlidingDFT::binAt(int binIndex) const
{
    if (binIndex < 0 || binIndex >= m_bins.size()) return BinState{};
    return m_bins[binIndex];
}

QVector<double> SlidingDFT::magnitudeSpectrum() const
{
    QVector<double> mag(m_bins.size());
    for (int k = 0; k < m_bins.size(); ++k)
        mag[k] = m_bins[k].magnitude;
    return mag;
}

QVector<QPair<int, double>> SlidingDFT::detectPeaks(double threshold) const
{
    QVector<QPair<int, double>> peaks;
    if (m_bins.size() < 3) return peaks;

    /* 找最大幅度 */
    double maxMag = 0.0;
    for (const auto& b : m_bins)
        maxMag = qMax(maxMag, b.magnitude);

    if (maxMag < 1e-10) return peaks;

    double threshMag = maxMag * threshold;

    /* 局部最大值检测(跳过DC和Nyquist) */
    for (int k = 1; k < m_bins.size() - 1; ++k) {
        if (m_bins[k].magnitude > threshMag &&
            m_bins[k].magnitude > m_bins[k - 1].magnitude &&
            m_bins[k].magnitude > m_bins[k + 1].magnitude)
        {
            peaks.append({k, m_bins[k].magnitude});
        }
    }

    /* 统计 */
    m_stats.totalPeakDetections += peaks.size();
    return peaks;
}

/* ========== 频率转换 ========== */

double SlidingDFT::frequencyResolution() const
{
    return m_sampleRate / m_fftSize;
}

double SlidingDFT::binToFrequency(int bin) const
{
    return bin * m_sampleRate / m_fftSize;
}

int SlidingDFT::frequencyToBin(double freq) const
{
    int bin = qRound(freq * m_fftSize / m_sampleRate);
    return qBound(0, bin, m_bins.size() - 1);
}

/* ========== 重置 ========== */

void SlidingDFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reinitBins();
}
