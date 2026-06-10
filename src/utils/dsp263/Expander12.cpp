/**
 * @file Expander12.cpp
 * @brief Expander12 实现
 *
 * 实现扩展器：程序依赖包络跟踪频谱泄漏控制透明动态缩减。
 */

#include "utils/dsp263/Expander12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander12::Expander12(QObject *parent)
    : QObject(parent)
{
    m_prevSpectrum.resize(m_fftSize / 2 + 1);
}

Expander12::~Expander12() = default;

/* ---- Configuration ---- */

void Expander12::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(8000.0, sampleRate);
}

void Expander12::setParameters(double threshold, double ratio, double range,
                               double attack, double release, double knee)
{
    m_threshold = threshold;
    m_ratio = qMax(1.0, ratio);
    m_range = range;
    m_attack = qMax(0.01, attack);
    m_release = qMax(0.01, release);
    m_knee = qMax(0.0, knee);
}

/* ---- Time to coefficient ---- */

double Expander12::timeToCoeff(double timeMs) const
{
    return qExp(-1.0 / (timeMs * 0.001 * m_sampleRate));
}

/* ---- Compute gain with soft knee ---- */

double Expander12::computeGain(double inputDb) const
{
    double halfKnee = m_knee / 2.0;
    double tLow = m_threshold - halfKnee;
    double tHigh = m_threshold + halfKnee;

    if (m_knee <= 0.0 || inputDb >= tHigh) {
        // Below threshold: apply expansion
        double overDb = inputDb - m_threshold;
        return overDb * (1.0 - 1.0 / m_ratio);
    }
    if (inputDb <= tLow) {
        // Far below threshold: apply full expansion
        double overDb = inputDb - m_threshold;
        double gain = overDb * (1.0 - 1.0 / m_ratio);
        return qMax(gain, m_range);
    }
    // In knee region: quadratic interpolation
    double x = inputDb - tLow;
    double gain = (1.0 - 1.0 / m_ratio) * (x * x / (2.0 * m_knee));
    return qMax(gain, m_range);
}

/* ---- Spectral bleed correction ---- */

double Expander12::spectralBleedCorrection(const QVector<double>& block) const
{
    // Simple spectral bleed estimate using block energy distribution
    int n = block.size();
    if (n == 0) return 1.0;

    double energy = 0.0;
    for (int i = 0; i < n; ++i)
        energy += block[i] * block[i];
    energy /= n;

    if (energy < 1e-20) return 1.0;

    // Detect transients: high energy ratio vs smoothed estimate
    double ratio = 1.0;
    if (m_prevSpectrum.size() > 0) {
        double prevE = 0.0;
        for (int i = 0; i < qMin(n, m_prevSpectrum.size()); ++i)
            prevE += m_prevSpectrum[i];
        prevE /= qMax(1, qMin(n, m_prevSpectrum.size()));
        if (prevE > 1e-20)
            ratio = qSqrt(energy / prevE);
    }

    // Limit correction to prevent overshoot
    return qBound(0.5, ratio, 2.0);
}

/* ---- Process block ---- */

QVector<double> Expander12::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);

    double attackCoeff = timeToCoeff(m_attack);
    double releaseCoeff = timeToCoeff(m_release);

    double totalReduction = 0.0;
    double peakReduction = 0.0;

    // Spectral bleed correction factor
    double bleedFactor = spectralBleedCorrection(input);

    for (int i = 0; i < n; ++i) {
        // Input level in dB
        double absSample = qAbs(input[i]);
        double inputDb = (absSample > 1e-10) ? 20.0 * qLog10(absSample) : -120.0;

        // Program-dependent envelope tracking
        if (inputDb > m_envelope) {
            m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * inputDb;
        } else {
            m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * inputDb;
        }

        // Compute gain from envelope
        double gainDb = computeGain(m_envelope);

        // Apply range limiting
        gainDb = qMax(gainDb, m_range);

        // Apply spectral bleed correction to gain
        double correctionDb = (bleedFactor - 1.0) * 3.0;
        gainDb += qBound(-6.0, correctionDb, 6.0);

        // Convert to linear gain
        double gainLin = qPow(10.0, gainDb / 20.0);

        output[i] = input[i] * gainLin;
        m_gainDb = gainDb;

        double reduction = qAbs(gainDb);
        totalReduction += reduction;
        if (reduction > peakReduction) peakReduction = reduction;
    }

    // Update spectral estimate for bleed control
    int copyLen = qMin(n, m_prevSpectrum.size());
    for (int i = 0; i < copyLen; ++i)
        m_prevSpectrum[i] = input[i] * input[i];

    double elapsed = timer.elapsed();
    m_stats.blockSize = n;
    m_stats.sampleRate = m_sampleRate;
    m_reductionSum += (n > 0) ? totalReduction / n : 0.0;
    m_reductionCount++;
    m_stats.avgGainReduction = m_reductionSum / m_reductionCount;
    m_stats.peakReduction = qMax(m_stats.peakReduction, peakReduction);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit gainReductionComputed(m_stats.avgGainReduction, m_stats.peakReduction, elapsed);
    return output;
}

/* ---- Reset ---- */

void Expander12::resetStatistics()
{
    m_envelope = 0.0;
    m_gainDb = 0.0;
    m_prevSpectrum.fill(0.0);
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reductionSum = 0.0;
    m_reductionCount = 0;
}
