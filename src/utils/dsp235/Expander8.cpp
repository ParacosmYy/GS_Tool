/**
 * @file Expander8.cpp
 * @brief Expander8 实现
 *
 * 实现动态范围扩展器：程序依赖攻击/释放时间与可变拐点软扩展曲线。
 */

#include "utils/dsp235/Expander8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander8::Expander8(QObject *parent) : QObject(parent) {}
Expander8::~Expander8() = default;

/* ---- Configuration ---- */

void Expander8::setRatio(double ratio) { m_ratio = qMax(1.0, ratio); }
void Expander8::setThreshold(double thresholdDb) { m_thresholdDb = thresholdDb; }
void Expander8::setAttack(double ms) { m_attackMs = qMax(0.01, ms); }
void Expander8::setRelease(double ms) { m_releaseMs = qMax(0.01, ms); }
void Expander8::setKnee(double kneeDb) { m_kneeDb = qMax(0.0, kneeDb); }
void Expander8::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); }

/* ---- dB conversions ---- */

double Expander8::ampToDb(double amp) const
{
    if (qAbs(amp) < 1e-10) return -120.0;
    return 20.0 * qLn(qAbs(amp)) / M_LN10;
}

double Expander8::dbToAmp(double db) const
{
    return qPow(10.0, db / 20.0);
}

/* ---- Compute expansion gain with variable knee ---- */

double Expander8::computeGain(double inputDb) const
{
    // Below threshold: apply expansion
    // gain = threshold + (input - threshold) / ratio
    double halfKnee = m_kneeDb / 2.0;
    double tLow = m_thresholdDb - halfKnee;
    double tHigh = m_thresholdDb + halfKnee;

    if (m_kneeDb < 0.01) {
        // Hard knee
        if (inputDb < m_thresholdDb) {
            return m_thresholdDb + (inputDb - m_thresholdDb) / m_ratio;
        }
        return inputDb;  // No expansion above threshold
    }

    // Soft knee: quadratic interpolation in knee region
    if (inputDb < tLow) {
        // Below knee: full expansion
        return m_thresholdDb + (inputDb - m_thresholdDb) / m_ratio;
    } else if (inputDb > tHigh) {
        // Above knee: unity gain
        return inputDb;
    } else {
        // In knee region: smooth quadratic transition
        double x = inputDb - tLow;
        double w = m_kneeDb;
        // Interpolate between expanded and unity
        double expanded = m_thresholdDb + (inputDb - m_thresholdDb) / m_ratio;
        double unity = inputDb;
        double t = x / w;  // 0..1 across knee
        // Smoothstep interpolation
        t = t * t * (3.0 - 2.0 * t);
        return expanded * (1.0 - t) + unity * t;
    }
}

/* ---- Program-dependent envelope coefficient ---- */

double Expander8::envelopeCoeff(double targetDb, double currentDb) const
{
    double diff = targetDb - currentDb;

    // Program-dependent: adapt time constant based on signal behavior
    double baseTime;
    if (diff < 0) {
        // Gain decreasing (expanding): use attack
        baseTime = m_attackMs;
        // Faster attack for larger transients
        if (diff < -10.0) baseTime *= 0.5;
    } else {
        // Gain increasing (recovering): use release
        baseTime = m_releaseMs;
        // Slower release for small changes
        if (diff < 3.0) baseTime *= 1.5;
    }

    // Convert time constant to per-sample coefficient
    double alpha = 1.0 - qExp(-1.0 / (baseTime * 0.001 * m_sampleRate));
    return alpha;
}

/* ---- Process ---- */

QVector<double> Expander8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double grSum = 0.0;
    double peakGr = 0.0;

    for (int i = 0; i < n; ++i) {
        // Detect input level
        double inputDb = ampToDb(input[i]);

        // Compute target gain from transfer function
        double targetDb = computeGain(inputDb);

        // Smooth envelope with program-dependent coefficient
        double alpha = envelopeCoeff(targetDb, m_envelopeDb);
        m_envelopeDb += alpha * (targetDb - m_envelopeDb);

        // Convert envelope to linear gain
        double gainLin = dbToAmp(m_envelopeDb - inputDb);

        // Apply gain
        output[i] = input[i] * gainLin;

        // Track gain reduction (negative = expansion)
        double gr = m_envelopeDb - inputDb;
        grSum += gr;
        if (gr < peakGr) peakGr = gr;

        // Store envelope point (sample every 100)
        if (i % 100 == 0) {
            EnvelopePoint ep;
            ep.sample = static_cast<double>(i);
            ep.gainDb = m_envelopeDb;
            ep.inputDb = inputDb;
            m_envelope.append(ep);
        }
    }

    m_grCount += n;
    m_gainReductionSum += grSum;

    m_stats.numSamples += n;
    m_stats.avgGainReduction = (m_grCount > 0) ? m_gainReductionSum / m_grCount : 0.0;
    m_stats.peakReduction = qMin(m_stats.peakReduction, peakGr);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, grSum / qMax(1, n), timer.elapsed());
    return output;
}

/* ---- Get envelope ---- */

QVector<Expander8::EnvelopePoint> Expander8::envelope() const { return m_envelope; }

/* ---- Reset ---- */

void Expander8::resetStatistics()
{
    m_envelopeDb = 0.0;
    m_gainReductionSum = 0.0;
    m_grCount = 0;
    m_envelope.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
