/**
 * @file SlidingDFT8.cpp
 * @brief SlidingDFT8 实现
 *
 * 实现滑动DFT：递归Goertzel更新与梳状滤波器极点校正保证稳定性。
 */

#include "utils/fft244/SlidingDFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SlidingDFT8::SlidingDFT8(QObject *parent) : QObject(parent) {}
SlidingDFT8::~SlidingDFT8() = default;

/* ---- Configuration ---- */

void SlidingDFT8::setBlockSize(int n)
{
    m_blockSize = qMax(4, n);
    m_circularBuf.resize(m_blockSize, 0.0);
    m_bufPos = 0;
    initBins();
}

void SlidingDFT8::setBins(const QVector<int>& bins)
{
    m_bins.clear();
    for (int k : bins) {
        BinState bs;
        bs.k = qBound(1, k, m_blockSize / 2);
        bs.coeff = 2.0 * qCos(2.0 * M_PI * bs.k / m_blockSize);
        bs.s0 = 0.0; bs.s1 = 0.0; bs.s2 = 0.0; bs.xOld = 0.0;
        m_bins.append(bs);
    }
    m_circularBuf.resize(m_blockSize, 0.0);
}

void SlidingDFT8::setCorrectionInterval(int samples)
{
    m_correctionInterval = qMax(1, samples);
}

/* ---- Initialize bin coefficients ---- */

void SlidingDFT8::initBins()
{
    for (auto& b : m_bins) {
        b.coeff = 2.0 * qCos(2.0 * M_PI * b.k / m_blockSize);
    }
}

/* ---- Comb filter pole correction ---- */

void SlidingDFT8::applyCorrection()
{
    // Stability correction: compute the exact DFT output for the current
    // window and reset the Goertzel state to prevent pole drift.
    // The recursive sliding DFT has a pole on the unit circle; without
    // correction, finite-precision arithmetic causes gradual instability.
    for (auto& b : m_bins) {
        // Recompute from current circular buffer via direct DFT at bin k
        double re = 0.0, im = 0.0;
        for (int n = 0; n < m_blockSize; ++n) {
            int idx = (m_bufPos + n) % m_blockSize;
            double angle = -2.0 * M_PI * b.k * n / m_blockSize;
            re += m_circularBuf[idx] * qCos(angle);
            im += m_circularBuf[idx] * qSin(angle);
        }
        // Reset Goertzel state from the correct DFT output
        // X[k] = s1 - e^{-j2πk/N} * s2 (Goertzel output relation)
        // For stability, we re-derive s0, s1 from re + j*im
        b.s1 = re;
        b.s2 = 0.0;
        b.s0 = b.coeff * b.s1 - b.s2;
    }
    m_stats.numCorrections++;
}

/* ---- Push single sample ---- */

void SlidingDFT8::pushSample(double sample)
{
    // Get oldest sample from circular buffer
    double xOld = m_circularBuf[m_bufPos];
    m_circularBuf[m_bufPos] = sample;
    m_bufPos = (m_bufPos + 1) % m_blockSize;

    // Update each bin via sliding Goertzel: S_new = coeff * S_prev - S_prev2 + x_new - x_old
    for (auto& b : m_bins) {
        double newS0 = sample - xOld + b.coeff * b.s1 - b.s2;
        b.s2 = b.s1;
        b.s1 = newS0;
        b.xOld = xOld;
    }

    m_sampleCount++;

    // Periodic stability correction
    if (m_sampleCount % m_correctionInterval == 0) {
        applyCorrection();
    }
}

/* ---- Process block ---- */

QVector<double> SlidingDFT8::processBlock(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    for (double s : samples)
        pushSample(s);

    QVector<double> mags = magnitudes();

    m_stats.blockSize = m_blockSize;
    m_stats.numBins = m_bins.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit blockProcessed(samples.size(), m_bins.size(), timer.elapsed());
    return mags;
}

/* ---- Bin magnitude ---- */

double SlidingDFT8::magnitude(int bin) const
{
    for (const auto& b : m_bins) {
        if (b.k == bin) {
            // |X[k]|² = s1² + s2² - 2*s1*s2*cos(2πk/N)
            double re = b.s1 - b.s2 * qCos(2.0 * M_PI * b.k / m_blockSize);
            double im = b.s2 * qSin(2.0 * M_PI * b.k / m_blockSize);
            return qSqrt(re * re + im * im) / m_blockSize;
        }
    }
    return 0.0;
}

/* ---- Bin phase ---- */

double SlidingDFT8::phase(int bin) const
{
    for (const auto& b : m_bins) {
        if (b.k == bin) {
            double re = b.s1 - b.s2 * qCos(2.0 * M_PI * b.k / m_blockSize);
            double im = b.s2 * qSin(2.0 * M_PI * b.k / m_blockSize);
            return qAtan2(im, re);
        }
    }
    return 0.0;
}

/* ---- All magnitudes ---- */

QVector<double> SlidingDFT8::magnitudes() const
{
    QVector<double> mags;
    mags.reserve(m_bins.size());
    for (const auto& b : m_bins) {
        double re = b.s1 - b.s2 * qCos(2.0 * M_PI * b.k / m_blockSize);
        double im = b.s2 * qSin(2.0 * M_PI * b.k / m_blockSize);
        mags.append(qSqrt(re * re + im * im) / m_blockSize);
    }
    return mags;
}

/* ---- Reset ---- */

void SlidingDFT8::reset()
{
    m_circularBuf.fill(0.0);
    m_bufPos = 0;
    m_sampleCount = 0;
    for (auto& b : m_bins) {
        b.s0 = 0.0; b.s1 = 0.0; b.s2 = 0.0; b.xOld = 0.0;
    }
}

/* ---- Reset statistics ---- */

void SlidingDFT8::resetStatistics()
{
    reset();
    m_stats = Stats{}; m_timeSum = 0.0;
}
