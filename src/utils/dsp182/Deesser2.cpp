/**
 * @file Deesser2.cpp
 * @brief Deesser2 实现
 *
 * 实现去齿音：频谱质心嘶嘶音检测、分频带处理(高低分离+高频压缩)、自适应阈值。
 */

#include "utils/dsp182/Deesser2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Deesser2::Deesser2(QObject *parent) : QObject(parent) {}
Deesser2::~Deesser2() = default;

/* ---- Configuration ---- */

void Deesser2::setThreshold(double threshDb) { m_thresholdDb = threshDb; }
void Deesser2::setFrequency(double freqHz) { m_frequency = qBound(1000.0, freqHz, 16000.0); }
void Deesser2::setRatio(double ratio) { m_ratio = qMax(1.0, ratio); }
void Deesser2::setAttack(double ms) { m_attackMs = qMax(0.1, ms); }
void Deesser2::setRelease(double ms) { m_releaseMs = qMax(1.0, ms); }
void Deesser2::setAutoThreshold(bool enabled) { m_autoThreshold = enabled; }
void Deesser2::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); }

/* ---- RMS energy ---- */

double Deesser2::rmsEnergy(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : frame) sum += s * s;
    return qSqrt(sum / frame.size());
}

/* ---- Spectral centroid ---- */

double Deesser2::spectralCentroid(const QVector<double>& frame) const
{
    int n = frame.size();
    if (n < 2) return 0.0;

    // Simple DFT-based centroid (use small window for speed)
    int fftN = 1;
    while (fftN < n) fftN *= 2;

    // Zero-padded magnitude spectrum
    double num = 0.0, den = 0.0;
    for (int k = 0; k < fftN / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / fftN;
            re += frame[i] * qCos(angle);
            im += frame[i] * qSin(angle);
        }
        double mag = qSqrt(re * re + im * im);
        double freq = static_cast<double>(k) * m_sampleRate / fftN;
        num += freq * mag;
        den += mag;
    }

    return (den > 1e-10) ? num / den : 0.0;
}

/* ---- Split band (1st-order Linkwitz-Riley style) ---- */

void Deesser2::splitBand(const QVector<double>& input,
                           QVector<double>& low, QVector<double>& high)
{
    int n = input.size();
    low.resize(n);
    high.resize(n);

    // Coefficient for 1st-order lowpass at crossover
    double omega = 2.0 * M_PI * m_frequency / m_sampleRate;
    double alpha = omega / (omega + 1.0); // Simplified one-pole

    double prevLow = 0.0;
    for (int i = 0; i < n; ++i) {
        double lpf = prevLow + alpha * (input[i] - prevLow);
        low[i] = lpf;
        high[i] = input[i] - lpf; // High-pass = input - low-pass
        prevLow = lpf;
    }
}

/* ---- Merge band ---- */

QVector<double> Deesser2::mergeBand(const QVector<double>& low,
                                      const QVector<double>& high) const
{
    int n = qMin(low.size(), high.size());
    QVector<double> output(n);
    for (int i = 0; i < n; ++i)
        output[i] = low[i] + high[i];
    return output;
}

/* ---- Estimate adaptive threshold ---- */

double Deesser2::estimateThreshold(const QVector<double>& frame) const
{
    // Use RMS energy to set threshold relative to signal level
    double rms = rmsEnergy(frame);
    if (rms < 1e-10) return m_thresholdDb;
    double rmsDb = 20.0 * qLn(rms) / qLn(10.0);
    // Threshold sits 6dB above the average level
    return rmsDb - 6.0;
}

/* ---- Process ---- */

QVector<double> Deesser2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return input;

    // Compute spectral centroid for sibilance detection
    double centroid = spectralCentroid(input);

    // Split into bands
    QVector<double> lowBand, highBand;
    splitBand(input, lowBand, highBand);

    // Compute high-band level in dB
    double highRms = rmsEnergy(highBand);
    double highDb = (highRms > 1e-10) ? 20.0 * qLn(highRms) / qLn(10.0) : -120.0;

    // Determine threshold
    double threshDb = m_thresholdDb;
    if (m_autoThreshold) threshDb = estimateThreshold(input);

    // Sibilance detection: high centroid AND high-band above threshold
    bool isSibilant = (centroid > m_frequency * 0.8) && (highDb > threshDb);

    // Compute gain reduction
    double targetGain = 1.0;
    double reductionDb = 0.0;
    if (isSibilant && highDb > threshDb) {
        double overDb = highDb - threshDb;
        reductionDb = overDb * (1.0 - 1.0 / m_ratio);
        targetGain = qPow(10.0, -reductionDb / 20.0);
    }

    // Smooth envelope (attack/release)
    double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * m_sampleRate / n));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * m_sampleRate / n));

    if (targetGain < m_envelope)
        m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * targetGain;
    else
        m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * targetGain;

    // Apply gain to high band
    QVector<double> processedHigh(n);
    for (int i = 0; i < n; ++i)
        processedHigh[i] = highBand[i] * m_envelope;

    // Merge bands
    QVector<double> output = mergeBand(lowBand, processedHigh);

    // Update statistics
    m_stats.totalFrames++;
    if (isSibilant) m_stats.sibilanceDetections++;
    m_stats.spectralCentroidHz = centroid;
    m_stats.avgReductionDb = (m_stats.avgReductionDb * (m_stats.totalFrames - 1) + reductionDb)
                              / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    if (isSibilant)
        emit sibilanceDetected(static_cast<int>(m_stats.totalFrames), centroid, reductionDb);

    return output;
}

/* ---- Reset ---- */

void Deesser2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 0.0;
}
