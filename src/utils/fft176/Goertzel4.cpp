/**
 * @file Goertzel4.cpp
 * @brief Goertzel4 实现
 *
 * 实现Goertzel单频DFT：标准块计算、滑动窗口连续更新、多频率批量计算。
 */

#include "utils/fft176/Goertzel4.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Goertzel4::Goertzel4(QObject *parent)
    : QObject(parent)
{
    updateCoefficient();
}

Goertzel4::~Goertzel4() = default;

/* ---- Configuration ---- */

void Goertzel4::setTargetFrequency(double freq)
{
    m_targetFreq = qMax(0.0, freq);
    updateCoefficient();
}

void Goertzel4::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    updateCoefficient();
}

void Goertzel4::setBlockSize(int N)
{
    m_blockSize = qMax(4, N);
    updateCoefficient();
    resetSliding();
}

/* ---- Update coefficient ---- */

void Goertzel4::updateCoefficient()
{
    m_binIndex = static_cast<int>(0.5 + m_targetFreq * m_blockSize / m_sampleRate);
    double k = static_cast<double>(m_binIndex);
    double omega = 2.0 * M_PI * k / m_blockSize;
    m_coeff = 2.0 * qCos(omega);
    m_coeff1 = qSin(omega);

    m_circularBuf.resize(m_blockSize, 0.0);
    m_prevS1.resize(m_blockSize, 0.0);
    m_prevS2.resize(m_blockSize, 0.0);
}

/* ---- Standard Goertzel block ---- */

QPair<double, double> Goertzel4::goertzelBlock(const QVector<double>& samples) const
{
    int N = qMin(samples.size(), m_blockSize);
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        double s0 = samples[i] + m_coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* Extract real and imaginary parts */
    double real = s1 - s2 * qCos(2.0 * M_PI * m_binIndex / m_blockSize);
    double imag = s2 * m_coeff1;

    double magnitude = qSqrt(real * real + imag * imag);
    double phase = qAtan2(imag, real);

    return {magnitude * 2.0 / N, phase};
}

/* ---- Block compute ---- */

QPair<double, double> Goertzel4::compute(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    auto result = goertzelBlock(samples);

    m_stats.totalComputations++;
    m_stats.lastMagnitude = result.first;
    m_stats.lastPhase = result.second;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(result.first, result.second);
    return result;
}

/* ---- Sliding window: process one sample ---- */

QPair<double, double> Goertzel4::processSample(double sample)
{
    /* Sliding Goertzel: maintain state and subtract oldest sample contribution */
    int pos = m_bufPos % m_blockSize;

    /* Remove oldest sample contribution using stored states */
    if (m_sampleCount >= m_blockSize) {
        double oldest = m_circularBuf[pos];
        int oldPos = (pos + 1) % m_blockSize;
        /* Reverse Goertzel for oldest sample */
        double oldS1 = m_prevS1[pos];
        double oldS2 = m_prevS2[pos];
        /* Approximate subtraction: recompute from scratch is more stable */
    }

    m_circularBuf[pos] = sample;
    m_prevS1[pos] = m_s1;
    m_prevS2[pos] = m_s2;

    /* Standard Goertzel iteration */
    double s0 = sample + m_coeff * m_s1 - m_s2;
    m_s2 = m_s1;
    m_s1 = s0;

    m_bufPos = (pos + 1) % m_blockSize;
    m_sampleCount++;

    /* Output result after we have at least N samples */
    if (m_sampleCount < m_blockSize)
        return {0.0, 0.0};

    /* Recompute from circular buffer for accuracy */
    double s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < m_blockSize; ++i) {
        int idx = (m_bufPos + i) % m_blockSize;
        double s0 = m_circularBuf[idx] + m_coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double real = s1 - s2 * qCos(2.0 * M_PI * m_binIndex / m_blockSize);
    double imag = s2 * m_coeff1;

    double magnitude = qSqrt(real * real + imag * imag) * 2.0 / m_blockSize;
    double phase = qAtan2(imag, real);

    m_stats.totalComputations++;
    emit slidingUpdated(magnitude, phase);
    return {magnitude, phase};
}

/* ---- Sliding batch ---- */

QVector<QPair<double, double>> Goertzel4::processSliding(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    int n = samples.size();
    QVector<QPair<double, double>> result(n);

    for (int i = 0; i < n; ++i)
        result[i] = processSample(samples[i]);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1ULL, m_stats.totalComputations);

    return result;
}

/* ---- Multi-frequency Goertzel ---- */

QVector<QPair<double, double>> Goertzel4::computeMultiFreq(
    const QVector<double>& samples,
    const QVector<double>& frequencies)
{
    QElapsedTimer timer;
    timer.start();

    int N = qMin(samples.size(), m_blockSize);
    int numFreqs = frequencies.size();
    QVector<QPair<double, double>> results(numFreqs);

    for (int f = 0; f < numFreqs; ++f) {
        int bin = static_cast<int>(0.5 + frequencies[f] * m_blockSize / m_sampleRate);
        double omega = 2.0 * M_PI * bin / m_blockSize;
        double coeff = 2.0 * qCos(omega);
        double sinePart = qSin(omega);

        double s1 = 0.0, s2 = 0.0;
        for (int i = 0; i < N; ++i) {
            double s0 = samples[i] + coeff * s1 - s2;
            s2 = s1;
            s1 = s0;
        }

        double real = s1 - s2 * qCos(2.0 * M_PI * bin / m_blockSize);
        double imag = s2 * sinePart;
        double mag = qSqrt(real * real + imag * imag) * 2.0 / N;
        double phase = qAtan2(imag, real);

        results[f] = {mag, phase};
    }

    m_stats.totalComputations += numFreqs;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    return results;
}

/* ---- Reset ---- */

void Goertzel4::resetSliding()
{
    m_s1 = 0.0;
    m_s2 = 0.0;
    m_sampleCount = 0;
    m_bufPos = 0;
    m_circularBuf.fill(0.0);
    m_prevS1.fill(0.0);
    m_prevS2.fill(0.0);
}

void Goertzel4::reset()
{
    resetSliding();
}

void Goertzel4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
