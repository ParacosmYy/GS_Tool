/**
 * @file NoiseGate5.cpp
 * @brief NoiseGate5 实现
 *
 * 实现谱减法噪声门：噪声谱估计、瞬态检测、自适应攻击释放增益包络。
 */

#include "utils/dsp219/NoiseGate5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

NoiseGate5::NoiseGate5(QObject *parent) : QObject(parent)
{
    updateCoefficients();
}

NoiseGate5::~NoiseGate5() = default;

/* ---- Configuration ---- */

void NoiseGate5::setParameters(double thresholdDb, double attackMs,
                                double releaseMs, int frameSize, int sampleRate)
{
    m_thresholdDb = thresholdDb;
    m_attackMs = qMax(0.1, attackMs);
    m_releaseMs = qMax(1.0, releaseMs);
    m_frameSize = qMax(64, frameSize);
    m_sampleRate = qMax(8000, sampleRate);
    m_stats.frameSize = m_frameSize;
    m_stats.sampleRate = m_sampleRate;
    updateCoefficients();
}

/* ---- Update coefficients ---- */

void NoiseGate5::updateCoefficients()
{
    // One-pole smoother: alpha = exp(-1 / (tau * Fs / frameSize))
    double attackTau = m_attackMs * 0.001 * m_sampleRate / m_frameSize;
    double releaseTau = m_releaseMs * 0.001 * m_sampleRate / m_frameSize;
    m_attackCoeff = (attackTau > 0.0) ? qExp(-1.0 / attackTau) : 0.0;
    m_releaseCoeff = (releaseTau > 0.0) ? qExp(-1.0 / releaseTau) : 0.0;
}

/* ---- Frame energy in dB ---- */

double NoiseGate5::frameEnergyDb(const QVector<double>& frame) const
{
    double sum = 0.0;
    for (int i = 0; i < frame.size(); ++i)
        sum += frame[i] * frame[i];
    double rms = qSqrt(sum / frame.size());
    if (rms < 1e-10) return -120.0;
    return 20.0 * qLn(rms) / qLn(10.0);
}

/* ---- Estimate noise profile ---- */

void NoiseGate5::estimateNoise(const QVector<double>& noiseSamples)
{
    int n = noiseSamples.size();
    if (n < m_frameSize) return;

    int numFrames = n / m_frameSize;
    int halfN = m_frameSize / 2;
    m_noiseProfile.resize(halfN);

    // Average magnitude spectrum over noise frames
    QVector<double> avgMag(halfN, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        // Simple DFT magnitude estimation via windowed segments
        for (int k = 0; k < halfN; ++k) {
            double re = 0.0, im = 0.0;
            for (int i = 0; i < m_frameSize; ++i) {
                int idx = f * m_frameSize + i;
                if (idx >= n) break;
                double angle = -2.0 * M_PI * k * i / m_frameSize;
                // Hann window
                double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_frameSize));
                re += noiseSamples[idx] * w * qCos(angle);
                im += noiseSamples[idx] * w * qSin(angle);
            }
            avgMag[k] += qSqrt(re * re + im * im) / numFrames;
        }
    }

    m_noiseProfile = avgMag;
    m_noiseEstimated = true;
}

/* ---- Spectral subtraction ---- */

QVector<double> NoiseGate5::spectralSubtract(const QVector<double>& frame) const
{
    int n = frame.size();
    int halfN = n / 2;
    QVector<double> result(n, 0.0);

    if (!m_noiseEstimated || m_noiseProfile.size() < halfN) {
        return frame;  // Pass through if no noise estimate
    }

    // Compute magnitude spectrum
    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            re += frame[i] * qCos(angle);
            im += frame[i] * qSin(angle);
        }
        double mag = qSqrt(re * re + im * im);
        double phase = qAtan2(im, re);

        // Spectral subtraction: max(mag - noise, 0)
        double cleaned = qMax(mag - m_noiseProfile[k] * 1.5, 0.0);

        // Reconstruct (symmetric)
        for (int i = 0; i < n; ++i) {
            double angle2 = 2.0 * M_PI * k * i / n;
            result[i] += 2.0 * cleaned * qCos(angle2 + phase) / n;
        }
    }
    return result;
}

/* ---- Transient detection ---- */

bool NoiseGate5::detectTransient(const QVector<double>& frame) const
{
    double energy = 0.0;
    for (int i = 0; i < frame.size(); ++i)
        energy += frame[i] * frame[i];

    double ratio = (m_prevEnergy > 1e-20) ? energy / m_prevEnergy : 1.0;
    // Transient if energy increases by > 6dB suddenly
    return ratio > 4.0;
}

/* ---- Apply gain envelope ---- */

QVector<double> NoiseGate5::applyGain(const QVector<double>& frame, double targetGain)
{
    // Smooth gain transition across frame
    QVector<double> out(frame.size());
    for (int i = 0; i < frame.size(); ++i) {
        if (m_gain < targetGain)
            m_gain = targetGain + (m_gain - targetGain) * m_attackCoeff;
        else
            m_gain = targetGain + (m_gain - targetGain) * m_releaseCoeff;
        out[i] = frame[i] * m_gain;
    }
    return out;
}

/* ---- Process single frame ---- */

QVector<double> NoiseGate5::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    double energyDb = frameEnergyDb(frame);
    bool transient = detectTransient(frame);
    bool gateOpen = energyDb > m_thresholdDb || transient;

    // Adaptive attack/release: faster attack on transients
    double origAttack = m_attackMs;
    if (transient) m_attackCoeff = 0.0;  // Instant attack for transients

    double targetGain = gateOpen ? 1.0 : 0.0;
    QVector<double> result;

    if (m_noiseEstimated) {
        QVector<double> cleaned = spectralSubtract(frame);
        result = applyGain(cleaned, targetGain);
    } else {
        result = applyGain(frame, targetGain);
    }

    if (transient) {
        m_attackMs = origAttack;
        updateCoefficients();
    }

    m_prevEnergy = 0.0;
    for (int i = 0; i < frame.size(); ++i)
        m_prevEnergy += frame[i] * frame[i];

    if (gateOpen) m_gateOpenCount++;
    m_stats.framesProcessed++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    m_stats.gateOpenRatio = (m_stats.framesProcessed > 0)
        ? double(m_gateOpenCount) / m_stats.framesProcessed : 0.0;

    emit frameProcessed(m_stats.framesProcessed, energyDb, gateOpen);
    return result;
}

/* ---- Process entire signal ---- */

QVector<double> NoiseGate5::processSignal(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    QVector<double> result(n, 0.0);
    int frameCount = 0;

    for (int i = 0; i + m_frameSize <= n; i += m_frameSize) {
        QVector<double> frame(m_frameSize);
        for (int j = 0; j < m_frameSize; ++j)
            frame[j] = signal[i + j];

        QVector<double> processed = process(frame);
        for (int j = 0; j < m_frameSize; ++j)
            result[i + j] = processed[j];
        frameCount++;
    }

    emit processingCompleted(frameCount, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void NoiseGate5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_gateOpenCount = 0;
    m_gain = 0.0;
    m_prevEnergy = 0.0;
    m_noiseProfile.clear();
    m_noiseEstimated = false;
}
