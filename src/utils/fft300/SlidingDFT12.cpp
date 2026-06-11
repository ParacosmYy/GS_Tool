/**
 * @file SlidingDFT12.cpp
 * @brief SlidingDFT12 实现
 *
 * 实现滑动离散傅里叶变换：Goertzel递归更新与频谱泄漏窗补偿实现连续实时频率监测。
 */

#include "utils/fft300/SlidingDFT12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SlidingDFT12::SlidingDFT12(QObject *parent)
    : QObject(parent) {}

SlidingDFT12::~SlidingDFT12() = default;

/* ---- Configuration ---- */

void SlidingDFT12::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); }

void SlidingDFT12::setTransformSize(int N) {
    m_N = qBound(16, N, 1 << 18);
    precomputeWindow();
    initBins();
}

/* ---- Goertzel coefficient for bin k ---- */

double SlidingDFT12::goertzelCoeff(int k) const
{
    return 2.0 * qCos(2.0 * M_PI * k / m_N);
}

/* ---- Window compensation (Hann) ---- */

double SlidingDFT12::windowCompensation(int idx) const
{
    if (idx < 0 || idx >= m_window.size()) return 1.0;
    return m_window[idx];
}

/* ---- Precompute Hann window compensation ---- */

void SlidingDFT12::precomputeWindow()
{
    m_window.resize(m_N);
    for (int i = 0; i < m_N; ++i) {
        // Hann window value; compensation = 1/window for deconvolution
        double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_N));
        m_window[i] = (hann > 1e-10) ? hann : 1e-10;
    }
}

/* ---- Initialize bins ---- */

void SlidingDFT12::initBins()
{
    // Bins are configured via setMonitoredBins; just reset states
    for (auto& b : m_bins) {
        b.s0 = 0.0;
        b.s1 = 0.0;
        b.s2 = 0.0;
    }
}

/* ---- Configure monitored frequency bins ---- */

void SlidingDFT12::setMonitoredBins(const QVector<double>& frequenciesHz)
{
    m_bins.clear();
    m_bins.reserve(frequenciesHz.size());

    for (double f : frequenciesHz) {
        BinState bin;
        int k = qBound(0, static_cast<int>(qRound(f * m_N / m_sampleRate)), m_N / 2);
        bin.coeff1 = goertzelCoeff(k);
        bin.s0 = 0.0;
        bin.s1 = 0.0;
        bin.s2 = 0.0;
        bin.magnitude = 0.0;
        bin.phase = 0.0;
        m_bins.append(bin);
    }
    m_circularBuf.resize(m_N, 0.0);
    m_writePos = 0;
    m_initialized = false;
}

/* ---- Initialize with a block of samples ---- */

void SlidingDFT12::initialize(const QVector<double>& samples)
{
    int n = qMin(samples.size(), m_N);

    m_circularBuf.fill(0.0);
    for (int i = 0; i < n; ++i)
        m_circularBuf[i] = samples[i];
    m_writePos = n % m_N;

    // Compute initial Goertzel states for each bin
    for (auto& bin : m_bins) {
        bin.s0 = 0.0;
        bin.s1 = 0.0;
        bin.s2 = 0.0;

        for (int i = 0; i < m_N; ++i) {
            double x = m_circularBuf[i] * windowCompensation(i);
            bin.s0 = x + bin.coeff1 * bin.s1 - bin.s2;
            bin.s2 = bin.s1;
            bin.s1 = bin.s0;
        }
        // Extract magnitude and phase
        double re = bin.s1 - bin.s2 * qCos(M_PI * 2.0 * (&bin - m_bins.data()) / m_N);
        double im = bin.s2 * qSin(M_PI * 2.0 * (&bin - m_bins.data()) / m_N);
        bin.magnitude = qSqrt(re * re + im * im) / m_N;
        bin.phase = qAtan2(im, re);
    }

    m_initialized = true;
}

/* ---- Push a single sample (sliding window update) ---- */

void SlidingDFT12::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    // Oldest sample leaving the window
    double oldest = m_circularBuf[m_writePos];

    // Insert new sample
    double windowed_new = sample * windowCompensation(m_writePos);
    m_circularBuf[m_writePos] = sample;
    int pos = m_writePos;
    m_writePos = (m_writePos + 1) % m_N;

    if (!m_initialized) {
        // Accumulate until buffer is full
        bool full = true;
        for (int i = 0; i < m_N; ++i) {
            if (m_circularBuf[i] == 0.0 && i >= m_writePos) { full = false; break; }
        }
        if (full) m_initialized = true;
        return;
    }

    // Sliding Goertzel update: subtract oldest contribution, add new
    double oldest_windowed = oldest * windowCompensation(pos);

    for (int b = 0; b < m_bins.size(); ++b) {
        auto& bin = m_bins[b];

        // Recursive sliding update:
        // new_s0 = x_new - x_old + coeff1 * s1 - s2
        // This is the key sliding DFT trick
        double delta = windowed_new - oldest_windowed;
        bin.s0 = delta + bin.coeff1 * bin.s1 - bin.s2;
        bin.s2 = bin.s1;
        bin.s1 = bin.s0;

        // Extract complex output for this bin
        int k = b; // simplified bin index
        double re = bin.s1 - bin.s2 * qCos(M_PI * k / m_N);
        double im = bin.s2 * qSin(M_PI * k / m_N);
        bin.magnitude = qSqrt(re * re + im * im) / m_N;
        bin.phase = qAtan2(im, re);
    }

    m_stats.totalUpdates++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;
    m_stats.numBins = m_bins.size();
    m_stats.transformSize = m_N;
}

/* ---- Get current spectrum snapshot ---- */

SlidingDFT12::SpectrumSnapshot SlidingDFT12::snapshot() const
{
    SpectrumSnapshot snap;
    int n = m_bins.size();
    snap.magnitudes.resize(n);
    snap.phases.resize(n);
    snap.frequencies.resize(n);

    for (int i = 0; i < n; ++i) {
        snap.magnitudes[i] = m_bins[i].magnitude;
        snap.phases[i] = m_bins[i].phase;
        // Convert bin index back to frequency
        int k = i;
        snap.frequencies[i] = static_cast<double>(k) * m_sampleRate / m_N;
    }
    snap.timestamp = static_cast<double>(m_stats.totalUpdates) / m_sampleRate;
    return snap;
}

/* ---- Reset ---- */

void SlidingDFT12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_circularBuf.fill(0.0);
    m_writePos = 0;
    m_initialized = false;
    for (auto& b : m_bins) {
        b.s0 = b.s1 = b.s2 = 0.0;
        b.magnitude = 0.0;
        b.phase = 0.0;
    }
}
