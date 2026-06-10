/**
 * @file EnvelopeDetector9.cpp
 * @brief EnvelopeDetector9 实现
 *
 * 实现包络检测器：峰值与RMS双模式及可配置动力学的综合幅度分析。
 */

#include "utils/signal286/EnvelopeDetector9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector9::EnvelopeDetector9(QObject *parent)
    : QObject(parent)
{
    updateCoeffs();
}

EnvelopeDetector9::~EnvelopeDetector9() = default;

/* ---- Configuration ---- */

void EnvelopeDetector9::setConfig(const DetectorConfig& cfg)
{
    m_config = cfg;
    m_config.attack = qBound(0.01, cfg.attack, 1000.0);
    m_config.release = qBound(0.1, cfg.release, 10000.0);
    m_config.sampleRate = qBound(8000.0, cfg.sampleRate, 192000.0);
    m_config.rmsWindow = qBound(8, cfg.rmsWindow, 8192);

    m_rmsBuffer.resize(m_config.rmsWindow, 0.0);
    m_rmsPos = 0;
    m_rmsSum = 0.0;
    updateCoeffs();
}

/* ---- Update ballistics coefficients ---- */

void EnvelopeDetector9::updateCoeffs()
{
    double sr = m_config.sampleRate;
    // Time constant for exponential smoothing
    // tau = 1/(1-alpha), so alpha = 1 - exp(-1/(tau*sr))
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_config.attack * 0.001 * sr));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_config.release * 0.001 * sr));
}

/* ---- Apply ballistics ---- */

double EnvelopeDetector9::applyBallistics(double input, double state) const
{
    double coeff = (input > state) ? m_attackCoeff : m_releaseCoeff;
    return state + coeff * (input - state);
}

/* ---- Compute RMS ---- */

double EnvelopeDetector9::computeRMS(const QVector<double>& window) const
{
    if (window.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : window)
        sum += s * s;
    return qSqrt(sum / window.size());
}

/* ---- Process single sample (streaming) ---- */

double EnvelopeDetector9::processSample(double sample)
{
    double absSample = qAbs(sample);

    // Peak envelope with ballistics
    m_envelopePeak = applyBallistics(absSample, m_envelopePeak);

    // RMS: sliding window
    if (m_rmsBuffer.size() > 0) {
        int winSize = m_config.rmsWindow;
        if (m_rmsBuffer.size() != winSize)
            m_rmsBuffer.resize(winSize, 0.0);

        // Remove oldest from sum
        m_rmsSum -= m_rmsBuffer[m_rmsPos] * m_rmsBuffer[m_rmsPos];

        // Add new
        m_rmsBuffer[m_rmsPos] = sample;
        m_rmsSum += sample * sample;

        m_rmsPos = (m_rmsPos + 1) % winSize;

        double rmsRaw = qSqrt(qMax(m_rmsSum, 0.0) / winSize);
        m_envelopeRMS = applyBallistics(rmsRaw, m_envelopeRMS);
    }

    // Return based on mode
    switch (m_config.mode) {
    case Peak:
        return m_config.logOutput
               ? 20.0 * qLog10(qMax(m_envelopePeak, 1e-30))
               : m_envelopePeak;
    case RMS:
        return m_config.logOutput
               ? 20.0 * qLog10(qMax(m_envelopeRMS, 1e-30))
               : m_envelopeRMS;
    case DualMode:
    default:
        // Return max of peak and RMS
        double val = qMax(m_envelopePeak, m_envelopeRMS);
        return m_config.logOutput
               ? 20.0 * qLog10(qMax(val, 1e-30))
               : val;
    }
}

/* ---- Detect envelope of input ---- */

EnvelopeDetector9::EnvelopeResult EnvelopeDetector9::detect(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    EnvelopeResult result;
    int n = input.size();
    if (n == 0) return result;

    bool needPeak = (m_config.mode == Peak || m_config.mode == DualMode);
    bool needRMS = (m_config.mode == RMS || m_config.mode == DualMode);

    if (needPeak) result.envelopePeak.resize(n);
    if (needRMS) result.envelopeRMS.resize(n);

    double maxPeak = 0.0;
    double globalRMS = 0.0;

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(input[i]);

        // Peak detection with ballistics
        if (needPeak) {
            m_envelopePeak = applyBallistics(absSample, m_envelopePeak);
            result.envelopePeak[i] = m_config.logOutput
                                     ? 20.0 * qLog10(qMax(m_envelopePeak, 1e-30))
                                     : m_envelopePeak;
            if (m_envelopePeak > maxPeak) maxPeak = m_envelopePeak;
        }

        // RMS detection with sliding window
        if (needRMS) {
            int winSize = m_config.rmsWindow;
            if (m_rmsBuffer.size() != winSize)
                m_rmsBuffer.resize(winSize, 0.0);

            m_rmsSum -= m_rmsBuffer[m_rmsPos] * m_rmsBuffer[m_rmsPos];
            m_rmsBuffer[m_rmsPos] = input[i];
            m_rmsSum += input[i] * input[i];
            m_rmsPos = (m_rmsPos + 1) % winSize;

            double rmsRaw = qSqrt(qMax(m_rmsSum, 0.0) / winSize);
            m_envelopeRMS = applyBallistics(rmsRaw, m_envelopeRMS);

            result.envelopeRMS[i] = m_config.logOutput
                                    ? 20.0 * qLog10(qMax(m_envelopeRMS, 1e-30))
                                    : m_envelopeRMS;
            globalRMS += input[i] * input[i];
        }
    }

    // Overall statistics
    result.peakAmplitude = maxPeak;

    if (needRMS && n > 0) {
        globalRMS = qSqrt(globalRMS / n);
        result.rmsLevel = globalRMS;
    }

    // Crest factor (peak/RMS in dB)
    if (result.rmsLevel > 1e-30 && result.peakAmplitude > 1e-30) {
        result.crestFactor = 20.0 * qLog10(result.peakAmplitude / result.rmsLevel);
    }

    double elapsed = timer.elapsed();
    m_stats.frameSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit detectDone(n, result.peakAmplitude, result.rmsLevel, elapsed);

    return result;
}

/* ---- Reset ---- */

void EnvelopeDetector9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelopePeak = 0.0;
    m_envelopeRMS = 0.0;
    m_rmsBuffer.fill(0.0);
    m_rmsPos = 0;
    m_rmsSum = 0.0;
}
