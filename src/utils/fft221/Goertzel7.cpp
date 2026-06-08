/**
 * @file Goertzel7.cpp
 * @brief Goertzel7 实现
 *
 * 实现Goertzel算法：单频点DFT、滑动窗口多音检测、能量比验证。
 */

#include "utils/fft221/Goertzel7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Goertzel7::Goertzel7(QObject *parent) : QObject(parent) {}
Goertzel7::~Goertzel7() = default;

/* ---- Configuration ---- */

void Goertzel7::setParameters(int blockSize, int sampleRate,
                                const QVector<double>& targetFreqs)
{
    m_blockSize = qMax(8, blockSize);
    m_sampleRate = qMax(1, sampleRate);
    m_targetFreqs = targetFreqs;
    m_stats.blockSize = m_blockSize;
    m_stats.sampleRate = m_sampleRate;
    m_stats.numTones = m_targetFreqs.size();

    // Initialize sliding window
    m_windowBuf.resize(m_blockSize, 0.0);
    m_windowPos = 0;
    m_windowFull = false;

    resetSlidingStates();
}

void Goertzel7::resetSlidingStates()
{
    m_gStates.resize(m_targetFreqs.size());
    for (int i = 0; i < m_targetFreqs.size(); ++i) {
        m_gStates[i].frequency = m_targetFreqs[i];
        m_gStates[i].coeff = computeCoeff(m_targetFreqs[i]);
        m_gStates[i].s0 = 0.0;
        m_gStates[i].s1 = 0.0;
        m_gStates[i].s2 = 0.0;
    }
}

double Goertzel7::computeCoeff(double targetFreq) const
{
    return 2.0 * qCos(2.0 * M_PI * targetFreq / m_sampleRate);
}

/* ---- Block Goertzel ---- */

double Goertzel7::runGoertzelBlock(const QVector<double>& samples, double coeff) const
{
    double s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < samples.size(); ++i) {
        double s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    // Magnitude = sqrt(s1^2 + s2^2 - coeff*s1*s2)
    return qSqrt(s1 * s1 + s2 * s2 - coeff * s1 * s2);
}

double Goertzel7::goertzelMag(const QVector<double>& samples, double targetFreq) const
{
    double coeff = computeCoeff(targetFreq);
    return runGoertzelBlock(samples, coeff);
}

/* ---- Block energy ---- */

double Goertzel7::blockEnergy(const QVector<double>& samples) const
{
    double e = 0.0;
    for (auto s : samples) e += s * s;
    return e;
}

/* ---- Detect (batch) ---- */

QVector<Goertzel7::ToneResult> Goertzel7::detect(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<ToneResult> results(m_targetFreqs.size());
    double totalEnergy = blockEnergy(samples);

    for (int i = 0; i < m_targetFreqs.size(); ++i) {
        double mag = runGoertzelBlock(samples, m_gStates[i].coeff);
        double energyRatio = (totalEnergy > 0.0) ? (mag * mag) / totalEnergy : 0.0;

        results[i].frequency = m_targetFreqs[i];
        results[i].magnitude = mag;
        results[i].energyRatio = energyRatio;
        results[i].detected = (energyRatio > 0.02); // 2% energy threshold
    }

    m_lastResults = results;
    m_stats.numDetected = 0;
    for (auto& r : results) if (r.detected) m_stats.numDetected++;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tonesDetected(m_stats.numDetected, timer.elapsed());
    return results;
}

/* ---- Sliding window push ---- */

QVector<Goertzel7::ToneResult> Goertzel7::pushSample(double sample)
{
    // Push into circular buffer
    m_windowBuf[m_windowPos] = sample;
    m_windowPos = (m_windowPos + 1) % m_blockSize;
    if (m_windowPos == 0) m_windowFull = true;

    if (!m_windowFull) return QVector<ToneResult>();

    // Window is full: run detection
    QVector<double> block(m_blockSize);
    for (int i = 0; i < m_blockSize; ++i)
        block[i] = m_windowBuf[(m_windowPos + i) % m_blockSize];

    return detect(block);
}

/* ---- DTMF validation ---- */

bool Goertzel7::validateDTMF(const QVector<ToneResult>& results,
                                double lowFreq, double highFreq,
                                double minRatio) const
{
    double lowMag = 0.0, highMag = 0.0;
    bool lowFound = false, highFound = false;

    for (auto& r : results) {
        if (qAbs(r.frequency - lowFreq) < 10.0) {
            lowMag = r.magnitude;
            lowFound = r.detected;
        }
        if (qAbs(r.frequency - highFreq) < 10.0) {
            highMag = r.magnitude;
            highFound = r.detected;
        }
    }

    if (!lowFound || !highFound) return false;

    // Check energy ratio: both tones should have comparable energy
    double ratio = qMin(lowMag, highMag) / qMax(lowMag, highMag);
    return ratio >= minRatio;
}

/* ---- Detected frequencies ---- */

QVector<double> Goertzel7::detectedFrequencies() const
{
    QVector<double> detected;
    for (auto& r : m_lastResults)
        if (r.detected) detected.append(r.frequency);
    return detected;
}

QVector<double> Goertzel7::detectedFrequencies() const
{
    QVector<double> detected;
    for (auto& r : m_lastResults)
        if (r.detected) detected.append(r.frequency);
    return detected;
}

/* ---- Reset ---- */

void Goertzel7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_windowBuf.clear();
    m_gStates.clear();
    m_lastResults.clear();
    m_windowPos = 0;
    m_windowFull = false;
}
