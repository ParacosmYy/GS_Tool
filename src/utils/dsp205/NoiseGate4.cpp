/**
 * @file NoiseGate4.cpp
 * @brief NoiseGate4 实现
 *
 * 实现自适应噪声门：最小统计噪声估计、谱底跟踪、增益平滑。
 */

#include "utils/dsp205/NoiseGate4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

NoiseGate4::NoiseGate4(QObject *parent) : QObject(parent)
{
    m_attackCoeff = 1.0 / m_attack;
    m_releaseCoeff = 1.0 / m_release;
}

NoiseGate4::~NoiseGate4() = default;

/* ---- Configuration ---- */

void NoiseGate4::setThreshold(double dB) { m_thresholdDb = dB; }
void NoiseGate4::setAttack(int samples) { m_attack = qMax(1, samples); m_attackCoeff = 1.0 / m_attack; }
void NoiseGate4::setRelease(int samples) { m_release = qMax(1, samples); m_releaseCoeff = 1.0 / m_release; }
void NoiseGate4::setHistoryFrames(int frames) { m_historyLen = qMax(1, frames); }

/* ---- Compute power spectrum ---- */

QVector<double> NoiseGate4::computePower(const QVector<double>& frame)
{
    int n = frame.size();
    QVector<double> power(n, 0.0);
    for (int i = 0; i < n; ++i)
        power[i] = frame[i] * frame[i];
    return power;
}

/* ---- Estimate noise floor (minimum statistics) ---- */

double NoiseGate4::estimateNoiseFloor(const QVector<double>& powerSpectrum)
{
    if (powerSpectrum.isEmpty()) return 0.0;
    double minVal = std::numeric_limits<double>::max();
    for (double v : powerSpectrum)
        if (v < minVal) minVal = v;
    return minVal;
}

/* ---- Update noise estimate ---- */

void NoiseGate4::updateNoiseEstimate(const QVector<double>& powerSpectrum)
{
    int bins = powerSpectrum.size();
    if (bins == 0) return;

    // Store power spectrum in circular history buffer
    if (m_powerHistory.size() < m_historyLen) {
        m_powerHistory.append(powerSpectrum);
    } else {
        m_powerHistory[m_historyIdx] = powerSpectrum;
    }
    m_historyIdx = (m_historyIdx + 1) % m_historyLen;

    // Initialize noise estimate if needed
    if (m_noiseEstimate.size() != bins)
        m_noiseEstimate = QVector<double>(bins, 0.0);

    // Minimum statistics: for each bin, take minimum across history
    for (int b = 0; b < bins; ++b) {
        double minPower = std::numeric_limits<double>::max();
        for (int h = 0; h < m_powerHistory.size(); ++h) {
            double val = (b < m_powerHistory[h].size()) ? m_powerHistory[h][b] : 0.0;
            if (val < minPower) minPower = val;
        }
        // Smooth update with bias factor to prevent underestimation
        double bias = 1.5;
        m_noiseEstimate[b] = 0.9 * m_noiseEstimate[b] + 0.1 * bias * minPower;
    }
}

/* ---- Apply gate ---- */

QVector<double> NoiseGate4::applyGate(const QVector<double>& frame,
                                       const QVector<double>& noiseFloor) const
{
    int n = frame.size();
    QVector<double> output(n, 0.0);
    double thresholdLin = qPow(10.0, m_thresholdDb / 10.0);

    for (int i = 0; i < n; ++i) {
        double nf = (i < noiseFloor.size()) ? noiseFloor[i] : 0.0;
        double power = frame[i] * frame[i];
        double snr = (nf > 1e-12) ? power / nf : 0.0;

        // Gate decision: open if SNR above threshold
        double targetGain = (snr > thresholdLin) ? 1.0 : 0.0;
        output[i] = frame[i] * targetGain;
    }
    return output;
}

/* ---- Process frame ---- */

QVector<double> NoiseGate4::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();
    int n = frame.size();
    if (n == 0) return {};

    // Compute power spectrum
    QVector<double> power = computePower(frame);

    // Update noise estimate using minimum statistics
    updateNoiseEstimate(power);

    // Compute noise floor level
    double nfLevel = 0.0;
    for (double v : m_noiseEstimate) nfLevel += v;
    if (!m_noiseEstimate.isEmpty()) nfLevel /= m_noiseEstimate.size();

    // Convert to dB for threshold comparison
    double framePower = 0.0;
    for (double v : power) framePower += v;
    framePower /= n;
    double frameDb = (framePower > 1e-12) ? 10.0 * qLn(framePower) / qLn(10.0) : -120.0;
    double nfDb = (nfLevel > 1e-12) ? 10.0 * qLn(nfLevel) / qLn(10.0) : -120.0;

    // Smooth gain with attack/release
    double targetGain = (frameDb > m_thresholdDb + nfDb) ? 1.0 : 0.0;
    if (targetGain > m_gain)
        m_gain += m_attackCoeff * (targetGain - m_gain);
    else
        m_gain += m_releaseCoeff * (targetGain - m_gain);

    // Apply gain
    QVector<double> output(n);
    for (int i = 0; i < n; ++i)
        output[i] = frame[i] * m_gain;

    m_stats.totalFrames++;
    m_stats.frameSize = n;
    m_stats.avgNoiseFloor = nfDb;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(nfDb, timer.elapsed());
    return output;
}

/* ---- Get noise estimate ---- */

QVector<double> NoiseGate4::noiseEstimate() const { return m_noiseEstimate; }

/* ---- Reset ---- */

void NoiseGate4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_noiseEstimate.clear();
    m_powerHistory.clear();
    m_historyIdx = 0;
    m_gain = 0.0;
}
