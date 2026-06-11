/**
 * @file Limiter13.cpp
 * @brief Limiter13 实现
 *
 * 实现砖墙限制器：前视延迟与自适应释放增益衰减零过冲峰值控制。
 */

#include "utils/dsp287/Limiter13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Limiter13::Limiter13(QObject *parent)
    : QObject(parent)
{
    setParams(LimiterParams{});
}

Limiter13::~Limiter13() = default;

/* ---- Configuration ---- */

void Limiter13::setParams(const LimiterParams& params)
{
    m_params = params;
    m_params.threshold = qBound(-60.0, m_params.threshold, 0.0);
    m_params.ceiling = qBound(-60.0, m_params.ceiling, 0.0);
    m_params.attack = qBound(0.01, m_params.attack, 100.0);
    m_params.release = qBound(1.0, m_params.release, 5000.0);
    m_params.sampleRate = qBound(8000.0, m_params.sampleRate, 192000.0);
    m_params.lookAhead = qBound(0, m_params.lookAhead, 8192);

    updateCoefficients();
    reset();
}

/* ---- Initialize coefficients ---- */

void Limiter13::updateCoefficients()
{
    double sr = m_params.sampleRate;
    if (sr <= 0) return;
    // Attack/release as exponential smoothing coefficients
    double attackSamples = m_params.attack * 0.001 * sr;
    double releaseSamples = m_params.release * 0.001 * sr;
    m_attackCoeff = (attackSamples > 0) ? 1.0 - qExp(-1.0 / attackSamples) : 1.0;
    m_releaseCoeff = (releaseSamples > 0) ? 1.0 - qExp(-1.0 / releaseSamples) : 1.0;
}

/* ---- Reset ---- */

void Limiter13::reset()
{
    m_delayLine.fill(0.0, m_params.lookAhead);
    m_gainReductionHistory.fill(0.0, m_params.lookAhead);
    m_lookAheadWritePos = 0;
    m_envelope = 0.0;
    m_gainComputerSmooth = 1.0;
}

/* ---- dB helpers ---- */

double Limiter13::ampToDb(double amp) { return (amp > 1e-10) ? 20.0 * qLn(amp) / M_LN10 : -200.0; }
double Limiter13::dbToAmp(double db) { return qExp(db * M_LN10 / 20.0); }

/* ---- Compute gain reduction ---- */

double Limiter13::computeGainReduction(double inputDb)
{
    if (inputDb <= m_params.threshold) return 0.0;
    // Gain reduction = threshold - inputDb (negative dB)
    return m_params.threshold - inputDb;
}

/* ---- Smooth with adaptive release ---- */

double Limiter13::smoothGainReduction(double targetReduction)
{
    // Adaptive release: faster release when gain reduction is small
    double adaptiveCoeff;
    if (targetReduction < m_envelope) {
        // Attack (getting deeper reduction)
        adaptiveCoeff = m_attackCoeff;
    } else {
        // Release (reducing reduction) - adaptive based on depth
        double ratio = qMin(1.0, qAbs(m_envelope) / 12.0);
        adaptiveCoeff = m_releaseCoeff * (0.2 + 0.8 * ratio);
    }

    if (targetReduction < m_envelope)
        m_envelope += (targetReduction - m_envelope) * adaptiveCoeff;
    else
        m_envelope += (targetReduction - m_envelope) * adaptiveCoeff;

    return m_envelope;
}

/* ---- Advance delay line ---- */

double Limiter13::advanceDelayLine(double sample)
{
    int delayLen = m_delayLine.size();
    if (delayLen == 0) return sample;

    double delayed = m_delayLine[m_lookAheadWritePos];
    m_delayLine[m_lookAheadWritePos] = sample;
    m_lookAheadWritePos = (m_lookAheadWritePos + 1) % delayLen;
    return delayed;
}

/* ---- Process mono ---- */

QVector<double> Limiter13::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double ceilingLinear = dbToAmp(m_params.ceiling);
    double maxReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(input[i]);
        double inputDb = ampToDb(absSample);

        // Compute target gain reduction
        double targetReduction = computeGainReduction(inputDb);

        // Apply ceiling constraint
        if (ampToDb(absSample) > m_params.ceiling)
            targetReduction = qMin(targetReduction, m_params.ceiling - ampToDb(absSample));

        // Smooth the gain reduction
        double smoothedReduction = smoothGainReduction(targetReduction);
        double gainLinear = dbToAmp(smoothedReduction);

        // Look-ahead: apply gain *before* the peak arrives
        double delayedSample = advanceDelayLine(input[i]);

        // Track gain reduction for look-ahead compensation
        double red = qAbs(smoothedReduction);
        if (red > maxReduction) maxReduction = red;

        output[i] = delayedSample * gainLinear * ceilingLinear;

        // Clamp to ceiling for zero-overshoot guarantee
        if (qAbs(output[i]) > ceilingLinear)
            output[i] = qCopysign(ceilingLinear, output[i]);
    }

    double elapsed = timer.elapsed();
    m_stats.totalSamples += n;
    m_stats.maxGainReduction = qMin(m_stats.maxGainReduction, -maxReduction);
    double peakOut = 0.0;
    for (int i = 0; i < n; ++i) peakOut = qMax(peakOut, qAbs(output[i]));
    m_stats.peakOutputDb = qMax(m_stats.peakOutputDb, ampToDb(peakOut));
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingDone(n, -maxReduction, elapsed);
    return output;
}

/* ---- Process multi-channel ---- */

QVector<QVector<double>> Limiter13::processMultiChannel(const QVector<QVector<double>>& input)
{
    int numChannels = input.size();
    if (numChannels == 0) return {};
    int numSamples = input[0].size();

    QVector<QVector<double>> output(numChannels);
    double ceilingLinear = dbToAmp(m_params.ceiling);

    for (int i = 0; i < numSamples; ++i) {
        // Find max absolute across channels for linked gain reduction
        double maxAbs = 0.0;
        for (int ch = 0; ch < numChannels; ++ch)
            maxAbs = qMax(maxAbs, qAbs(input[ch][i]));

        double inputDb = ampToDb(maxAbs);
        double targetReduction = computeGainReduction(inputDb);
        if (ampToDb(maxAbs) > m_params.ceiling)
            targetReduction = qMin(targetReduction, m_params.ceiling - ampToDb(maxAbs));

        double smoothedReduction = smoothGainReduction(targetReduction);
        double gainLinear = dbToAmp(smoothedReduction) * ceilingLinear;

        for (int ch = 0; ch < numChannels; ++ch) {
            output[ch].resize(numSamples);
            output[ch][i] = input[ch][i] * gainLinear;
            if (qAbs(output[ch][i]) > ceilingLinear)
                output[ch][i] = qCopysign(ceilingLinear, output[ch][i]);
        }
    }
    return output;
}

/* ---- Reset statistics ---- */

void Limiter13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
