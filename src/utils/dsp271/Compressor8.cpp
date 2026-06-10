/**
 * @file Compressor8.cpp
 * @brief Compressor8 实现
 *
 * 实现动态范围压缩器：侧链检测与自动增益补偿软拐点压缩曲线。
 */

#include "utils/dsp271/Compressor8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Compressor8::Compressor8(QObject *parent)
    : QObject(parent) {}

Compressor8::~Compressor8() = default;

/* ---- Configuration ---- */

void Compressor8::setThreshold(double thresholdDb)
{
    m_thresholdDb = qBound(-60.0, thresholdDb, 0.0);
}

void Compressor8::setRatio(double ratio)
{
    m_ratio = qBound(1.0, ratio, 100.0);
}

void Compressor8::setKneeWidth(double kneeDb)
{
    m_kneeDb = qBound(0.0, kneeDb, 24.0);
}

void Compressor8::setAttack(double ms)
{
    m_attackMs = qBound(0.01, ms, 500.0);
}

void Compressor8::setRelease(double ms)
{
    m_releaseMs = qBound(1.0, ms, 5000.0);
}

void Compressor8::setMakeUpGain(double gainDb)
{
    m_makeUpDb = qBound(-24.0, gainDb, 48.0);
}

/* ---- Soft-knee gain reduction computation ---- */

double Compressor8::computeGainReduction(double inputDb) const
{
    double T = m_thresholdDb;
    double W = m_kneeDb;
    double R = m_ratio;

    if (W <= 0.0) {
        // Hard knee
        if (inputDb <= T)
            return 0.0;
        return T + (inputDb - T) / R - inputDb;
    }

    // Soft knee: quadratic interpolation within knee band
    double halfW = W / 2.0;
    double kneeLo = T - halfW;
    double kneeHi = T + halfW;

    if (inputDb <= kneeLo) {
        return 0.0;
    } else if (inputDb >= kneeHi) {
        return T + (inputDb - T) / R - inputDb;
    } else {
        // Within soft knee region
        double x = inputDb - kneeLo;
        double gr = (1.0 / R - 1.0) * x * x / (2.0 * W);
        return gr;
    }
}

/* ---- Envelope smoothing with attack/release ---- */

double Compressor8::smoothEnvelope(double target, double sampleRate)
{
    double coeff;
    if (target > m_envelope) {
        // Attack phase: signal increasing
        coeff = qExp(-1.0 / (m_attackMs * 0.001 * sampleRate));
    } else {
        // Release phase: signal decreasing
        coeff = qExp(-1.0 / (m_releaseMs * 0.001 * sampleRate));
    }
    m_envelope = target + coeff * (m_envelope - target);
    return m_envelope;
}

/* ---- Main processing ---- */

QVector<double> Compressor8::process(const QVector<double>& input,
                                      const QVector<double>& sidechain)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    // Use sidechain if provided, otherwise use input as detector
    const QVector<double>& detector = (sidechain.size() == n) ? sidechain : input;

    double sampleRate = 44100.0; // default sample rate
    double makeUpLinear = qPow(10.0, m_makeUpDb / 20.0);

    m_gainReduction.resize(n);
    QVector<double> output(n);
    double peakReduction = 0.0;
    double sumReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        // Compute input level in dB
        double absVal = qAbs(detector[i]);
        double inputDb = 20.0 * qLn(qMax(absVal, 1e-10)) / qLn(10.0);

        // Compute gain reduction
        double gr = computeGainReduction(inputDb);

        // Target gain in linear domain
        double targetGainLinear = qPow(10.0, gr / 20.0);

        // Smooth the gain via attack/release ballistics
        double smoothedGain = smoothEnvelope(targetGainLinear, sampleRate);

        // Actual gain reduction in dB for monitoring
        double actualGrDb = 20.0 * qLn(qMax(smoothedGain, 1e-10)) / qLn(10.0);
        m_gainReduction[i] = actualGrDb;

        // Apply gain + make-up
        output[i] = input[i] * smoothedGain * makeUpLinear;

        double absGr = qAbs(actualGrDb);
        if (absGr > peakReduction) peakReduction = absGr;
        sumReduction += absGr;
    }

    double elapsed = timer.elapsed();
    m_stats.blockSize = n;
    m_stats.peakGainReduction = peakReduction;
    m_stats.avgGainReduction = sumReduction / n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(n, peakReduction, elapsed);

    return output;
}

/* ---- Gain reduction accessor ---- */

QVector<double> Compressor8::gainReduction() const
{
    return m_gainReduction;
}

/* ---- Reset ---- */

void Compressor8::resetStatistics()
{
    m_envelope = 0.0;
    m_gainReduction.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
