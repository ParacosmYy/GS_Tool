/**
 * @file Expander14.cpp
 * @brief Expander14 实现
 *
 * 实现动态扩展器：并行压缩下行混合与RMS电平检测的透明向上动态扩展。
 */

#include "utils/dsp277/Expander14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander14::Expander14(QObject *parent)
    : QObject(parent) {}

Expander14::~Expander14() = default;

/* ---- Configuration ---- */

void Expander14::setParams(const Params& params) { m_params = params; }

/* ---- dB conversion helpers ---- */

double Expander14::dbToLinear(double db) { return qPow(10.0, db / 20.0); }
double Expander14::linearToDb(double linear) { return (linear > 1e-10) ? 20.0 * qLn(linear) / M_LN10 : -200.0; }

/* ---- Compute RMS level ---- */

double Expander14::computeRms(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double sumSq = 0.0;
    for (double s : frame) sumSq += s * s;
    return qSqrt(sumSq / frame.size());
}

/* ---- Compute expansion gain ---- */

double Expander14::computeGain(double inputDb) const
{
    double threshold = m_params.threshold;
    double ratio = m_params.ratio;
    double knee = m_params.knee;
    double range = m_params.range;

    double diff = inputDb - threshold;
    double gainDb = 0.0;

    if (knee > 0.0 && qAbs(diff) <= knee / 2.0) {
        // Soft knee: quadratic interpolation
        double halfKnee = knee / 2.0;
        double x = diff + halfKnee;
        gainDb = ((1.0 - ratio) / (2.0 * knee)) * x * x;
    } else if (diff < 0.0) {
        // Below threshold: expand (attenuate further)
        gainDb = diff * (1.0 - 1.0 / ratio);
    }
    // Above threshold: unity gain

    // Clamp expansion range
    gainDb = qMax(-range, gainDb);
    return gainDb;
}

/* ---- Ballistics (envelope follower) ---- */

double Expander14::applyBallistics(double targetGain, double sampleRate)
{
    double coeff = 0.0;
    if (targetGain < m_envelope) {
        // Attack: signal is getting louder (expanding less)
        double attackSamples = m_params.attack * sampleRate / 1000.0;
        coeff = (attackSamples > 0.0) ? qExp(-1.0 / attackSamples) : 0.0;
    } else {
        // Release: signal is getting quieter (expanding more)
        double releaseSamples = m_params.release * sampleRate / 1000.0;
        coeff = (releaseSamples > 0.0) ? qExp(-1.0 / releaseSamples) : 0.0;
    }
    m_envelope = coeff * m_envelope + (1.0 - coeff) * targetGain;
    return m_envelope;
}

/* ---- Process single sample ---- */

double Expander14::processSample(double sample)
{
    double inputDb = linearToDb(qAbs(sample));
    double targetGain = computeGain(inputDb);
    double smoothGain = applyBallistics(targetGain, m_sampleRate);

    // Main expansion path
    double expanded = sample * dbToLinear(smoothGain);

    // Parallel compression blend: mix in a compressed version
    if (m_params.parallelMix > 0.0) {
        double compressedGain = qMin(0.0, -smoothGain); // Invert expansion for blend
        double compressed = sample * dbToLinear(compressedGain);
        expanded = (1.0 - m_params.parallelMix) * expanded +
                   m_params.parallelMix * compressed;
    }

    // Apply makeup gain
    expanded *= dbToLinear(m_params.makeupGain);

    m_stats.numFrames++;
    m_stats.avgInputDb = (m_stats.avgInputDb * (m_stats.numFrames - 1) + inputDb) / m_stats.numFrames;
    m_stats.avgGainDb = (m_stats.avgGainDb * (m_stats.numFrames - 1) + smoothGain) / m_stats.numFrames;

    return expanded;
}

/* ---- Process block ---- */

QVector<double> Expander14::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());

    // RMS-level detection: compute RMS first for lookahead hint
    double rmsLevel = computeRms(input);
    double rmsDb = linearToDb(rmsLevel);

    for (int i = 0; i < input.size(); ++i) {
        // Blend sample-level and RMS-level detection
        double sampleDb = linearToDb(qAbs(input[i]));
        double effectiveDb = 0.6 * sampleDb + 0.4 * rmsDb;
        effectiveDb = qMax(-120.0, effectiveDb);

        double targetGain = computeGain(effectiveDb);
        double smoothGain = applyBallistics(targetGain, m_sampleRate);

        double expanded = input[i] * dbToLinear(smoothGain);

        // Parallel compression blend
        if (m_params.parallelMix > 0.0) {
            double compressedGain = qMin(0.0, -smoothGain);
            double compressed = input[i] * dbToLinear(compressedGain);
            expanded = (1.0 - m_params.parallelMix) * expanded +
                       m_params.parallelMix * compressed;
        }

        expanded *= dbToLinear(m_params.makeupGain);
        output[i] = expanded;
    }

    double elapsed = timer.elapsed();
    m_stats.numFrames += input.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    double avgGain = 0.0;
    for (int i = 0; i < output.size(); ++i) {
        double g = (qAbs(input[i]) > 1e-10) ? linearToDb(qAbs(output[i]) / qAbs(input[i])) : 0.0;
        avgGain += g;
    }
    if (!output.isEmpty()) avgGain /= output.size();

    emit blockProcessed(input.size(), avgGain, elapsed);
    return output;
}

/* ---- Analyze frame without processing ---- */

Expander14::FrameAnalysis Expander14::analyzeFrame(const QVector<double>& frame) const
{
    FrameAnalysis analysis;
    double rms = computeRms(frame);
    analysis.rmsDb = linearToDb(rms);

    double peak = 0.0;
    for (double s : frame) peak = qMax(peak, qAbs(s));
    analysis.inputDb = linearToDb(peak);

    analysis.gainDb = computeGain(analysis.inputDb);
    analysis.outputDb = analysis.inputDb + analysis.gainDb;
    analysis.envelope = 0.0; // Not tracked in const analysis

    return analysis;
}

/* ---- Reset internal state ---- */

void Expander14::reset()
{
    m_envelope = 0.0;
}

/* ---- Reset statistics ---- */

void Expander14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 0.0;
}
