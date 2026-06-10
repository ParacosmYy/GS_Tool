/**
 * @file SlidingDFT10.cpp
 * @brief SlidingDFT10 实现
 *
 * 实现滑动DFT：Goertzel递归更新与循环缓冲区O(1)逐样本频谱bin计算。
 */

#include "utils/fft272/SlidingDFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SlidingDFT10::SlidingDFT10(QObject *parent)
    : QObject(parent) {}

SlidingDFT10::~SlidingDFT10() = default;

/* ---- Configuration ---- */

void SlidingDFT10::setBlockSize(int n)
{
    m_blockSize = qBound(16, n, 65536);
    m_circularBuf.resize(m_blockSize, 0.0);
    m_writePos = 0;

    // Reinitialize bin coefficients for new block size
    for (auto& bin : m_bins)
        initBin(bin, bin.k, m_blockSize);
}

void SlidingDFT10::setBins(const QVector<int>& bins)
{
    m_bins.clear();
    m_bins.reserve(bins.size());
    for (int k : bins) {
        BinState state;
        initBin(state, k, m_blockSize);
        m_bins.append(state);
    }
}

/* ---- Initialize Goertzel coefficients for a target bin ---- */

void SlidingDFT10::initBin(BinState& state, int k, int n)
{
    state.k = k;
    double omega = 2.0 * M_PI * k / n;
    state.coeff1 = 2.0 * qCos(omega);
    state.coeff2 = -1.0;
    state.s0 = 0.0;
    state.s1 = 0.0;
    state.s2 = 0.0;
    state.output = {0.0, 0.0};
}

/* ---- Update all bins with one new sample and one old sample ---- */

void SlidingDFT10::updateBins(double newSample, double oldSample)
{
    // Sliding DFT principle:
    // X_k(n) = X_k(n-1) + x(n) - x(n-N)
    // Scaled by twiddle: X_k = (s0 - s1 * W^k) where W = e^(-j2pi*k/N)
    // Using Goertzel recursive form with circular buffer subtraction

    for (auto& bin : m_bins) {
        double diff = newSample - oldSample;

        // Goertzel recursive step with correction
        double s0_new = diff + bin.coeff1 * bin.s0 + bin.coeff2 * bin.s1;
        bin.s2 = bin.s1;
        bin.s1 = bin.s0;
        bin.s0 = s0_new;

        // Extract complex output from Goertzel state
        double omega = 2.0 * M_PI * bin.k / m_blockSize;
        double cosW = qCos(omega);
        double sinW = qSin(omega);

        // X[k] = s0 - s1 * W^k, where W^k = cos(omega) - j*sin(omega)
        bin.output.re = bin.s0 - bin.s1 * cosW;
        bin.output.im = bin.s1 * sinW;
    }
}

/* ---- Push single sample (streaming mode) ---- */

bool SlidingDFT10::pushSample(double sample)
{
    double oldSample = m_circularBuf[m_writePos];
    m_circularBuf[m_writePos] = sample;
    m_writePos = (m_writePos + 1) % m_blockSize;

    updateBins(sample, oldSample);

    // Block is complete when write position wraps around
    return (m_writePos == 0);
}

/* ---- Process entire block at once ---- */

QVector<SlidingDFT10::Complex> SlidingDFT10::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0 || m_bins.isEmpty()) return {};

    // Reset Goertzel state
    for (auto& bin : m_bins) {
        bin.s0 = 0.0;
        bin.s1 = 0.0;
        bin.s2 = 0.0;
    }

    m_circularBuf.resize(m_blockSize, 0.0);

    // Process each sample
    for (int i = 0; i < n; ++i) {
        int pos = i % m_blockSize;
        double oldSample = m_circularBuf[pos];
        m_circularBuf[pos] = input[i];
        updateBins(input[i], oldSample);
    }
    m_writePos = n % m_blockSize;

    // Collect outputs
    QVector<Complex> result;
    result.reserve(m_bins.size());
    for (const auto& bin : m_bins)
        result.append(bin.output);

    double elapsed = timer.elapsed();
    m_stats.blockSize = n;
    m_stats.numBins = m_bins.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit blockReady(n, m_bins.size(), elapsed);

    return result;
}

/* ---- Magnitudes ---- */

QVector<double> SlidingDFT10::magnitudes() const
{
    QVector<double> mags;
    mags.reserve(m_bins.size());
    for (const auto& bin : m_bins) {
        double mag = qSqrt(bin.output.re * bin.output.re + bin.output.im * bin.output.im);
        mags.append(mag);
    }
    return mags;
}

/* ---- Phases ---- */

QVector<double> SlidingDFT10::phases() const
{
    QVector<double> phs;
    phs.reserve(m_bins.size());
    for (const auto& bin : m_bins)
        phs.append(qAtan2(bin.output.im, bin.output.re));
    return phs;
}

/* ---- Reset ---- */

void SlidingDFT10::resetStatistics()
{
    for (auto& bin : m_bins) {
        bin.s0 = 0.0;
        bin.s1 = 0.0;
        bin.s2 = 0.0;
        bin.output = {0.0, 0.0};
    }
    m_circularBuf.clear();
    m_writePos = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
