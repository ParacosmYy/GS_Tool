/**
 * @file SlidingDFT9.cpp
 * @brief SlidingDFT9 实现
 *
 * 实现滑动DFT：调制滑窗与周期分母重正化保证稳定性。
 */

#include "utils/fft258/SlidingDFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SlidingDFT9::SlidingDFT9(QObject *parent)
    : QObject(parent) { initBins(); }
SlidingDFT9::~SlidingDFT9() = default;

/* ---- Configuration ---- */

void SlidingDFT9::setSize(int size)
{
    // Round up to power of 2
    int p = 1;
    while (p < size) p <<= 1;
    m_size = p;
    initBins();
}

void SlidingDFT9::setRenormPeriod(int period) { m_renormPeriod = qMax(m_size, period); }
void SlidingDFT9::setModulationDepth(double depth) { m_modDepth = qBound(0.0, depth, 2.0); }

/* ---- Initialize bins ---- */

void SlidingDFT9::initBins()
{
    m_bins.resize(m_size);
    m_circularBuf.resize(m_size, 0.0);
    m_bufIdx = 0;
    m_sampleCounter = 0;

    for (int k = 0; k < m_size; ++k) {
        double angle = 2.0 * M_PI * k / m_size;
        m_bins[k].coeffReal = qCos(angle);
        m_bins[k].coeffImag = qSin(angle);
        m_bins[k].real = 0.0;
        m_bins[k].imag = 0.0;
    }
}

/* ---- Update single bin ---- */

void SlidingDFT9::updateBin(int k, double newSample, double oldSample)
{
    // Sliding DFT: S_k[n] = coeff * (S_k[n-1] + x[n] - x[n-N])
    BinState& bin = m_bins[k];
    double diff = newSample - oldSample;

    // Apply modulation depth weighting
    diff *= m_modDepth;

    // Update using twiddle factor rotation
    double newReal = bin.real + diff;
    double newImag = bin.imag;

    // Rotate by twiddle factor
    bin.real = newReal * bin.coeffReal + newImag * bin.coeffImag;
    bin.imag = -newReal * bin.coeffImag + newImag * bin.coeffReal;
}

/* ---- Renormalize all bins ---- */

void SlidingDFT9::renormalize()
{
    // Recompute each bin from circular buffer to prevent numerical drift
    for (int k = 0; k < m_size; ++k) {
        double re = 0.0, im = 0.0;
        double angle_step = 2.0 * M_PI * k / m_size;
        for (int n = 0; n < m_size; ++n) {
            int idx = (m_bufIdx + n) % m_size;
            double angle = angle_step * n;
            re += m_circularBuf[idx] * qCos(angle);
            im -= m_circularBuf[idx] * qSin(angle);
        }
        m_bins[k].real = re;
        m_bins[k].imag = im;
    }
    m_stats.renormalizations++;
}

/* ---- Push single sample ---- */

QVector<double> SlidingDFT9::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    // Get old sample from circular buffer
    double oldSample = m_circularBuf[m_bufIdx];
    m_circularBuf[m_bufIdx] = sample;
    m_bufIdx = (m_bufIdx + 1) % m_size;
    m_sampleCounter++;

    // Update all bins
    for (int k = 0; k < m_size; ++k)
        updateBin(k, sample, oldSample);

    // Periodic renormalization for stability
    if (m_sampleCounter % m_renormPeriod == 0)
        renormalize();

    // Compute magnitude spectrum
    QVector<double> mag = magnitude();

    double elapsed = timer.elapsed();
    m_stats.samplesProcessed = m_sampleCounter;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit spectrumUpdated(m_size, m_sampleCounter, elapsed);
    return mag;
}

/* ---- Process block ---- */

QVector<double> SlidingDFT9::processBlock(const QVector<double>& samples)
{
    QVector<double> lastSpectrum;
    for (double s : samples)
        lastSpectrum = pushSample(s);
    return lastSpectrum;
}

/* ---- Get magnitude spectrum ---- */

QVector<double> SlidingDFT9::magnitude() const
{
    QVector<double> mag(m_size / 2);
    for (int k = 0; k < m_size / 2; ++k) {
        double re = m_bins[k].real;
        double im = m_bins[k].imag;
        mag[k] = qSqrt(re * re + im * im) / m_size;
    }
    return mag;
}

/* ---- Get phase spectrum ---- */

QVector<double> SlidingDFT9::phase() const
{
    QVector<double> ph(m_size / 2);
    for (int k = 0; k < m_size / 2; ++k)
        ph[k] = qAtan2(m_bins[k].imag, m_bins[k].real);
    return ph;
}

/* ---- Get frequency axis ---- */

QVector<double> SlidingDFT9::frequencyAxis() const
{
    QVector<double> freq(m_size / 2);
    double binWidth = static_cast<double>(m_sampleRate) / m_size;
    for (int k = 0; k < m_size / 2; ++k)
        freq[k] = k * binWidth;
    return freq;
}

/* ---- Reset ---- */

void SlidingDFT9::resetStatistics()
{
    initBins();
    m_sampleCounter = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
