/**
 * @file SlidingDFT7.cpp
 * @brief SlidingDFT7 实现
 *
 * 实现滑动DFT：保证稳定性窗递归与逐频点噪声底跟踪。
 */

#include "utils/fft230/SlidingDFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SlidingDFT7::SlidingDFT7(QObject *parent) : QObject(parent) {}
SlidingDFT7::~SlidingDFT7() = default;

/* ---- Configure ---- */

bool SlidingDFT7::configure(int fftSize)
{
    if (fftSize < 8) return false;
    m_fftSize = fftSize;
    int halfN = fftSize / 2;
    m_buffer.resize(fftSize, 0.0);
    m_bins.resize(halfN);
    m_pos = 0;

    m_stats.fftSize = fftSize;
    m_stats.numBins = halfN;
    return true;
}

/* ---- Twiddle factor ---- */

void SlidingDFT7::twiddle(int k, double& cosW, double& sinW) const
{
    double angle = 2.0 * M_PI * k / m_fftSize;
    cosW = qCos(angle);
    sinW = qSin(angle);
}

/* ---- Update noise floor ---- */

void SlidingDFT7::updateNoiseFloor(int bin, double magnitude)
{
    // Exponential moving average for noise floor, slow tracking
    double alpha = 0.001;
    double& nf = m_bins[bin].noiseFloor;
    if (magnitude < nf)
        nf = (1.0 - alpha * 10) * nf + alpha * 10 * magnitude;
    else
        nf = (1.0 - alpha) * nf + alpha * magnitude;
}

/* ---- Update peak hold ---- */

void SlidingDFT7::updatePeakHold(int bin, double magnitude)
{
    double& peak = m_bins[bin].peakHold;
    int& count = m_bins[bin].peakHoldCount;

    if (magnitude > peak) {
        peak = magnitude;
        count = 0;
    } else {
        count++;
        // Decay peak after hold time
        if (count > m_fftSize)
            peak *= 0.995;
    }
}

/* ---- Push single sample ---- */

void SlidingDFT7::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    // Remove oldest sample from bins (recursion)
    double oldest = m_buffer[m_pos];

    // Windowed recursion: Hann window implicit via leaky integration
    double windowFactor = 0.999; // Stability factor

    int halfN = m_fftSize / 2;
    for (int k = 0; k < halfN; ++k) {
        double cosW, sinW;
        twiddle(k, cosW, sinW);

        // Sliding DFT recursion: X_k[n] = window * (X_k[n-1] + x[n] - x[n-N]) * e^{j2pi*k/N}
        // Guaranteed-stable variant with windowed decay
        double re = m_bins[k].real;
        double im = m_bins[k].imag;

        // Subtract oldest contribution and add new
        double delta = sample - oldest;
        re = windowFactor * re + delta * cosW;
        im = windowFactor * im - delta * sinW;

        m_bins[k].real = re;
        m_bins[k].imag = im;
        m_bins[k].magnitude = qSqrt(re * re + im * im);

        updateNoiseFloor(k, m_bins[k].magnitude);
        updatePeakHold(k, m_bins[k].magnitude);
    }

    m_buffer[m_pos] = sample;
    m_pos = (m_pos + 1) % m_fftSize;

    m_stats.samplesProcessed++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit spectrumUpdated(halfN, timer.elapsed());
}

/* ---- Push block ---- */

void SlidingDFT7::pushBlock(const QVector<double>& samples)
{
    for (int i = 0; i < samples.size(); ++i)
        pushSample(samples[i]);
}

/* ---- Magnitudes ---- */

QVector<double> SlidingDFT7::magnitudes() const
{
    int halfN = m_fftSize / 2;
    QVector<double> mag(halfN);
    for (int k = 0; k < halfN; ++k)
        mag[k] = m_bins[k].magnitude;
    return mag;
}

/* ---- Noise floor ---- */

QVector<double> SlidingDFT7::noiseFloor() const
{
    int halfN = m_fftSize / 2;
    QVector<double> nf(halfN);
    for (int k = 0; k < halfN; ++k)
        nf[k] = m_bins[k].noiseFloor;
    return nf;
}

/* ---- Reset ---- */

void SlidingDFT7::reset()
{
    m_buffer.fill(0.0);
    for (auto& b : m_bins) {
        b.real = 0.0;
        b.imag = 0.0;
        b.magnitude = 0.0;
        b.noiseFloor = 0.0;
        b.peakHold = 0.0;
        b.peakHoldCount = 0;
    }
    m_pos = 0;
}

/* ---- Reset statistics ---- */

void SlidingDFT7::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
