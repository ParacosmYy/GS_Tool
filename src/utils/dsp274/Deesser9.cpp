/**
 * @file Deesser9.cpp
 * @brief Deesser9 实现
 *
 * 实现去齿音器：心理声学加权齿音检测与多频带动态抑制自然语音处理。
 */

#include "utils/dsp274/Deesser9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Deesser9::Deesser9(QObject *parent)
    : QObject(parent)
{
    designBandpass();
}

Deesser9::~Deesser9() = default;

/* ---- Configuration ---- */

void Deesser9::setSampleRate(int rate) { m_sampleRate = qBound(8000, rate, 192000); designBandpass(); }
void Deesser9::setSibilanceBand(double lowHz, double highHz) {
    m_lowHz = qBound(1000.0, lowHz, 16000.0);
    m_highHz = qBound(m_lowHz + 500.0, highHz, 20000.0);
    designBandpass();
}
void Deesser9::setThreshold(double thresholdDb) { m_thresholdDb = qBound(-60.0, thresholdDb, 0.0); }
void Deesser9::setMaxReduction(double maxReductionDb) { m_maxReductionDb = qBound(-30.0, maxReductionDb, 0.0); }
void Deesser9::setAttack(double ms) { m_attackMs = qBound(0.1, ms, 50.0); }
void Deesser9::setRelease(double ms) { m_releaseMs = qBound(1.0, ms, 500.0); }

/* ---- Design 2nd-order Butterworth bandpass for sibilance band ---- */

void Deesser9::designBandpass()
{
    double fL = m_lowHz / m_sampleRate;
    double fH = m_highHz / m_sampleRate;
    double wL = 2.0 * M_PI * fL;
    double wH = 2.0 * M_PI * fH;

    // Bandpass via cascade of highpass and lowpass
    double cL = qCos(wL) / (1.0 + qSin(wL));
    double cH = qCos(wH) / (1.0 + qSin(wH));

    // Simplified 2nd-order bandpass coefficients
    double bw = wH - wL;
    double wm = (wH + wL) / 2.0;
    double alpha = qSin(bw) / 2.0;
    double cosWm = qCos(wm);

    m_b0 = alpha;
    m_b1 = 0.0;
    m_b2 = -alpha;
    m_a0 = 1.0 + alpha;
    m_a1 = -2.0 * cosWm;
    m_a2 = 1.0 - alpha;

    // Normalize
    m_b0 /= m_a0; m_b1 /= m_a0; m_b2 /= m_a0;
    m_a1 /= m_a0; m_a2 /= m_a0;

    // Reset filter state
    m_x1 = m_x2 = m_y1 = m_y2 = 0.0;
}

/* ---- Apply bandpass filter ---- */

double Deesser9::bandpassSample(double x)
{
    double y = m_b0 * x + m_b1 * m_x1 + m_b2 * m_x2 - m_a1 * m_y1 - m_a2 * m_y2;
    m_x2 = m_x1; m_x1 = x;
    m_y2 = m_y1; m_y1 = y;
    return y;
}

/* ---- A-weighted psychoacoustic approximation ---- */

double Deesser9::aWeightedLevel(double sibilanceLevel) const
{
    // Approximate A-weighting: boost in 2-5 kHz range by ~2 dB
    double weightDb = 2.0;
    return sibilanceLevel * qPow(10.0, weightDb / 20.0);
}

/* ---- Main processing ---- */

QVector<double> Deesser9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * m_sampleRate));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * m_sampleRate));

    m_sibilanceEnv.resize(n);
    m_gainReduction.resize(n);

    QVector<double> output(n);
    int sibilantCount = 0;
    double totalReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        // Bandpass to extract sibilance band
        double sibilant = bandpassSample(input[i]);

        // Envelope detection via RMS
        double sibLevel = sibilant * sibilant;

        // Smooth envelope follower with separate attack/release
        if (sibLevel > m_envLevel)
            m_envLevel = attackCoeff * m_envLevel + (1.0 - attackCoeff) * sibLevel;
        else
            m_envLevel = releaseCoeff * m_envLevel + (1.0 - releaseCoeff) * sibLevel;

        // Convert to dB with psychoacoustic weighting
        double envDb = 10.0 * qLog10(qMax(m_envLevel, 1e-10));
        double weightedDb = aWeightedLevel(qPow(10.0, envDb / 20.0));
        double weightedLevelDb = 20.0 * qLog10(qMax(weightedDb, 1e-10));

        m_sibilanceEnv[i] = weightedLevelDb;

        // Compute gain reduction
        double reduction = 0.0;
        if (weightedLevelDb > m_thresholdDb) {
            double overDb = weightedLevelDb - m_thresholdDb;
            reduction = qMin(overDb, -m_maxReductionDb);
            sibilantCount++;
        }

        // Smooth gain reduction
        double targetGain = -reduction;
        if (targetGain < m_gainState)
            m_gainState = attackCoeff * m_gainState + (1.0 - attackCoeff) * targetGain;
        else
            m_gainState = releaseCoeff * m_gainState + (1.0 - releaseCoeff) * targetGain;

        m_gainReduction[i] = m_gainState;
        totalReduction += qAbs(m_gainState);

        // Apply gain reduction (multiplicative in linear domain)
        double gainLin = qPow(10.0, m_gainState / 20.0);
        output[i] = input[i] * gainLin;
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.sibilanceRate = (n > 0) ? static_cast<double>(sibilantCount) / n : 0.0;
    m_stats.avgReductionDb = (n > 0) ? totalReduction / n : 0.0;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(n, m_stats.sibilanceRate, elapsed);

    return output;
}

/* ---- Accessors ---- */

QVector<double> Deesser9::sibilanceEnvelope() const { return m_sibilanceEnv; }
QVector<double> Deesser9::gainReduction() const { return m_gainReduction; }

/* ---- Reset ---- */

void Deesser9::resetStatistics()
{
    m_sibilanceEnv.clear();
    m_gainReduction.clear();
    m_x1 = m_x2 = m_y1 = m_y2 = 0.0;
    m_envLevel = 0.0;
    m_gainState = 0.0;
    designBandpass();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
