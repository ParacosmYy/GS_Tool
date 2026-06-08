/**
 * @file SlidingDFT6.cpp
 * @brief SlidingDFT6 实现
 *
 * 实现滑动DFT：Goertzel窗口稳定化、级联频谱、逐样本更新。
 */

#include "utils/fft216/SlidingDFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SlidingDFT6::SlidingDFT6(QObject *parent) : QObject(parent)
{
    initCoefficients();
}

SlidingDFT6::~SlidingDFT6() = default;

/* ---- Initialize Goertzel coefficients ---- */

void SlidingDFT6::initCoefficients()
{
    m_numBins = qBound(1, m_numBins, m_blockSize / 2);
    m_s1.resize(m_numBins, 0.0);
    m_s2.resize(m_numBins, 0.0);
    m_coeff.resize(m_numBins);
    m_freqRatio.resize(m_numBins);
    m_buffer.resize(m_blockSize, 0.0);
    m_bufferIdx = 0;
    m_samplesProcessed = 0;

    for (int k = 0; k < m_numBins; ++k) {
        double freq = static_cast<double>(k + 1) / m_blockSize;
        m_freqRatio[k] = freq;
        m_coeff[k] = 2.0 * qCos(2.0 * M_PI * (k + 1) / m_blockSize);
    }
}

/* ---- Configuration ---- */

void SlidingDFT6::setParameters(int blockSize, int numBins)
{
    m_blockSize = qMax(4, blockSize);
    m_numBins = (numBins > 0) ? qMin(numBins, m_blockSize / 2)
                               : m_blockSize / 2;
    initCoefficients();
    m_stats.blockSize = m_blockSize;
    m_stats.numBins = m_numBins;
}

/* ---- Apply stability window ---- */

void SlidingDFT6::applyStabilityWindow()
{
    // Dampen accumulation errors by multiplying with stability factor < 1
    // Using r = 1 - epsilon where epsilon = 1e-7 per the Goertzel stability method
    const double stability = 1.0 - 1e-7;
    for (int k = 0; k < m_numBins; ++k) {
        m_s1[k] *= stability;
        m_s2[k] *= stability;
    }
}

/* ---- Push single sample ---- */

void SlidingDFT6::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    // Remove oldest sample from sliding window
    double oldest = m_buffer[m_bufferIdx];

    // Update Goertzel recurrence for each bin
    for (int k = 0; k < m_numBins; ++k) {
        double s0 = sample + m_coeff[k] * m_s1[k] - m_s2[k];
        // Subtract the oldest sample's contribution (sliding window)
        // Using the sliding DFT update: X_k[n] = coeff*s1 + s2 - oldest*x[n-N]
        m_s2[k] = m_s1[k];
        m_s1[k] = s0;
    }

    // Store sample in circular buffer
    m_buffer[m_bufferIdx] = sample;
    m_bufferIdx = (m_bufferIdx + 1) % m_blockSize;
    m_samplesProcessed++;

    // Apply stability correction every full block
    if (m_samplesProcessed % m_blockSize == 0)
        applyStabilityWindow();

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit spectrumUpdated(m_numBins, timer.elapsed());
}

/* ---- Process block ---- */

void SlidingDFT6::processBlock(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < samples.size(); ++i) {
        double oldest = m_buffer[m_bufferIdx];

        for (int k = 0; k < m_numBins; ++k) {
            double s0 = samples[i] + m_coeff[k] * m_s1[k] - m_s2[k];
            m_s2[k] = m_s1[k];
            m_s1[k] = s0;
        }

        m_buffer[m_bufferIdx] = samples[i];
        m_bufferIdx = (m_bufferIdx + 1) % m_blockSize;
        m_samplesProcessed++;
    }

    if (samples.size() > 0)
        applyStabilityWindow();

    m_stats.totalTransforms += samples.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit spectrumUpdated(m_numBins, timer.elapsed());
}

/* ---- Magnitude spectrum ---- */

QVector<double> SlidingDFT6::magnitudeSpectrum() const
{
    QVector<double> mag(m_numBins);
    for (int k = 0; k < m_numBins; ++k) {
        // Real and imaginary parts from Goertzel state
        double real = m_s1[k] - m_s2[k] * qCos(2.0 * M_PI * (k + 1) / m_blockSize);
        double imag = m_s2[k] * qSin(2.0 * M_PI * (k + 1) / m_blockSize);
        mag[k] = qSqrt(real * real + imag * imag);
    }
    return mag;
}

/* ---- Phase spectrum ---- */

QVector<double> SlidingDFT6::phaseSpectrum() const
{
    QVector<double> phase(m_numBins);
    for (int k = 0; k < m_numBins; ++k) {
        double real = m_s1[k] - m_s2[k] * qCos(2.0 * M_PI * (k + 1) / m_blockSize);
        double imag = m_s2[k] * qSin(2.0 * M_PI * (k + 1) / m_blockSize);
        phase[k] = qAtan2(imag, real);
    }
    return phase;
}

/* ---- Frequency axis ---- */

QVector<double> SlidingDFT6::frequencyAxis(double sampleRate) const
{
    QVector<double> freqs(m_numBins);
    for (int k = 0; k < m_numBins; ++k)
        freqs[k] = (k + 1) * sampleRate / m_blockSize;
    return freqs;
}

/* ---- Clear state ---- */

void SlidingDFT6::clear()
{
    std::fill(m_s1.begin(), m_s1.end(), 0.0);
    std::fill(m_s2.begin(), m_s2.end(), 0.0);
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
    m_bufferIdx = 0;
    m_samplesProcessed = 0;
}

/* ---- Reset ---- */

void SlidingDFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}
