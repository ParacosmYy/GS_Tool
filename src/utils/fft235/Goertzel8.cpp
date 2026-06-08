/**
 * @file Goertzel8.cpp
 * @brief Goertzel8 实现
 *
 * 实现Goertzel算法：滑动窗口与可配置重叠实时音调检测。
 */

#include "utils/fft235/Goertzel8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Goertzel8::Goertzel8(QObject *parent) : QObject(parent) {}
Goertzel8::~Goertzel8() = default;

/* ---- Configuration ---- */

bool Goertzel8::configure(int blockSize, int sampleRate)
{
    if (blockSize < 8 || sampleRate < 1000) return false;

    m_blockSize = blockSize;
    m_sampleRate = sampleRate;
    m_ringBuffer.resize(blockSize * 2);  // Extra space for overlap
    m_ringBuffer.fill(0.0);
    m_writePos = 0;
    m_samplesInBuffer = 0;

    m_stats.blockSize = blockSize;
    m_stats.sampleRate = sampleRate;
    precomputeCoefficients();
    return true;
}

/* ---- Setters ---- */

void Goertzel8::setTargets(const QVector<double>& frequencies)
{
    m_targets = frequencies;
    m_stats.numTargets = m_targets.size();
    precomputeCoefficients();
}

void Goertzel8::setOverlap(double ratio)
{
    m_overlapRatio = qBound(0.0, ratio, 0.95);
}

void Goertzel8::setThreshold(double thresholdDb)
{
    m_thresholdDb = thresholdDb;
}

/* ---- Precompute coefficients ---- */

void Goertzel8::precomputeCoefficients()
{
    int N = m_blockSize;
    m_coeff.resize(m_targets.size());
    m_cosK.resize(m_targets.size());
    m_sinK.resize(m_targets.size());

    for (int i = 0; i < m_targets.size(); ++i) {
        // k = target_freq * N / sample_rate
        double k = m_targets[i] * N / m_sampleRate;
        double w = 2.0 * M_PI * k / N;
        m_coeff[i] = 2.0 * qCos(w);
        m_cosK[i] = qCos(w);
        m_sinK[i] = qSin(w);
    }
}

/* ---- Single-frequency Goertzel ---- */

Goertzel8::Detection Goertzel8::goertzelSingle(const QVector<double>& samples, int idx) const
{
    int N = qMin(samples.size(), m_blockSize);
    double coeff = m_coeff[idx];
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    // Goertzel recursion
    for (int n = 0; n < N; ++n) {
        s0 = samples[n] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    Detection det;
    det.frequency = m_targets[idx];
    // Magnitude from final state
    double re = s1 - s2 * m_cosK[idx];
    double im = s2 * m_sinK[idx];
    det.magnitude = qSqrt(re * re + im * im) / N;
    det.magnitudeDb = (det.magnitude > 0.0) ? 20.0 * qLn(det.magnitude) / M_LN10 : -120.0;
    det.phase = qAtan2(im, re);
    det.detected = (det.magnitudeDb >= m_thresholdDb);
    return det;
}

/* ---- Check buffer ---- */

bool Goertzel8::hasFullBlock() const { return m_samplesInBuffer >= m_blockSize; }

/* ---- Extract block ---- */

QVector<double> Goertzel8::extractBlock() const
{
    QVector<double> block(m_blockSize);
    int start = (m_writePos - m_blockSize + m_ringBuffer.size()) % m_ringBuffer.size();
    for (int i = 0; i < m_blockSize; ++i) {
        block[i] = m_ringBuffer[(start + i) % m_ringBuffer.size()];
    }
    return block;
}

/* ---- Process block ---- */

QVector<Goertzel8::Detection> Goertzel8::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Detection> results(m_targets.size());
    int N = qMin(samples.size(), m_blockSize);

    for (int i = 0; i < m_targets.size(); ++i)
        results[i] = goertzelSingle(samples, i);

    // Count detections
    int numDetected = 0;
    for (const auto& d : results) {
        if (d.detected) {
            numDetected++;
            emit toneDetected(d.frequency, d.magnitudeDb);
        }
    }

    m_blockNum++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit detectionCompleted(m_blockNum, numDetected);
    return results;
}

/* ---- Push samples (streaming) ---- */

QVector<Goertzel8::Detection> Goertzel8::pushSamples(const QVector<double>& samples)
{
    // Write to ring buffer
    for (int i = 0; i < samples.size(); ++i) {
        m_ringBuffer[m_writePos] = samples[i];
        m_writePos = (m_writePos + 1) % m_ringBuffer.size();
        m_samplesInBuffer = qMin(m_samplesInBuffer + 1, m_ringBuffer.size());
    }

    QVector<Detection> results;
    int hopSize = qMax(1, static_cast<int>(m_blockSize * (1.0 - m_overlapRatio)));

    // Process as many full blocks as available
    while (m_samplesInBuffer >= m_blockSize) {
        QVector<double> block = extractBlock();
        results = process(block);
        m_stats.overlapsProcessed++;

        // Advance by hop
        m_samplesInBuffer -= hopSize;
        // Simulate read pointer advance by adjusting write pos concept
        // For simplicity, just track remaining samples
        break;  // Process one block per push for real-time
    }
    return results;
}

/* ---- Accessors ---- */

QVector<double> Goertzel8::targets() const { return m_targets; }

/* ---- Reset ---- */

void Goertzel8::resetStatistics()
{
    m_ringBuffer.fill(0.0);
    m_writePos = 0;
    m_samplesInBuffer = 0;
    m_blockNum = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
