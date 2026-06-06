/**
 * @file DynamicEQ2.cpp
 * @brief DynamicEQ2 实现
 *
 * 实现动态均衡器：频率依赖阈值、包络跟随器、双二阶带通滤波、侧链检测。
 */

#include "utils/dsp185/DynamicEQ2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DynamicEQ2::DynamicEQ2(QObject *parent) : QObject(parent) {}
DynamicEQ2::~DynamicEQ2() = default;

/* ---- Configuration ---- */

void DynamicEQ2::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }

void DynamicEQ2::setBands(const QVector<BandParams>& bands)
{
    m_bands = bands;
    int n = bands.size();
    m_envelopeState.resize(n);
    m_bpState.resize(n);
    m_bpSideState.resize(n);
    std::fill(m_envelopeState.begin(), m_envelopeState.end(), 0.0);
    std::fill(m_bpState.begin(), m_bpState.end(), BiquadState{});
    std::fill(m_bpSideState.begin(), m_bpSideState.end(), BiquadState{});
}

/* ---- Biquad bandpass design ---- */

DynamicEQ2::BiquadCoeffs DynamicEQ2::designBandpass(double freq, double q) const
{
    BiquadCoeffs c;
    double w0 = 2.0 * M_PI * freq / m_sampleRate;
    double alpha = qSin(w0) / (2.0 * q);
    double cosW0 = qCos(w0);

    c.b0 = alpha;
    c.b1 = 0.0;
    c.b2 = -alpha;
    double a0 = 1.0 + alpha;
    c.a1 = -2.0 * cosW0;
    c.a2 = 1.0 - alpha;

    // Normalize
    c.b0 /= a0; c.b1 /= a0; c.b2 /= a0;
    c.a1 /= a0; c.a2 /= a0;
    return c;
}

/* ---- Apply biquad ---- */

double DynamicEQ2::applyBiquad(double sample, const BiquadCoeffs& c,
                                BiquadState& s) const
{
    double y = c.b0 * sample + c.b1 * s.x1 + c.b2 * s.x2
               - c.a1 * s.y1 - c.a2 * s.y2;
    s.x2 = s.x1; s.x1 = sample;
    s.y2 = s.y1; s.y1 = y;
    return y;
}

/* ---- Envelope follower ---- */

double DynamicEQ2::computeEnvelope(double sample, double attack, double release,
                                    double& state) const
{
    double absSample = qAbs(sample);
    double attackCoeff = qExp(-1.0 / (attack * m_sampleRate * 0.001));
    double releaseCoeff = qExp(-1.0 / (release * m_sampleRate * 0.001));

    if (absSample > state)
        state = attackCoeff * state + (1.0 - attackCoeff) * absSample;
    else
        state = releaseCoeff * state + (1.0 - releaseCoeff) * absSample;

    return state;
}

/* ---- Gain reduction ---- */

double DynamicEQ2::gainReduction(double level, double threshold, double ratio) const
{
    double dbLevel = 20.0 * qLog10(qMax(1e-10, level));
    if (dbLevel < threshold) return 0.0;
    double overDb = dbLevel - threshold;
    return -overDb * (1.0 - 1.0 / ratio);
}

/* ---- Process mono ---- */

QVector<double> DynamicEQ2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    int nb = m_bands.size();
    if (n == 0 || nb == 0) return input;

    // Pre-compute filter coefficients
    QVector<BiquadCoeffs> coeffs(nb);
    for (int b = 0; b < nb; ++b)
        coeffs[b] = designBandpass(m_bands[b].frequency, m_bands[b].qFactor);

    QVector<double> output = input;
    double peakReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double correction = 0.0;

        for (int b = 0; b < nb; ++b) {
            // Bandpass filter the signal
            double bp = applyBiquad(input[i], coeffs[b], m_bpState[b]);

            // Compute envelope
            double env = computeEnvelope(bp, m_bands[b].attack,
                                          m_bands[b].release,
                                          m_envelopeState[b]);

            // Compute gain reduction
            double gr = gainReduction(env, m_bands[b].threshold, m_bands[b].ratio);
            peakReduction = qMin(peakReduction, gr);

            // Frequency-dependent threshold adjustment
            double freqNorm = m_bands[b].frequency / (m_sampleRate * 0.5);
            double freqAdj = 1.0 + 0.5 * freqNorm;
            gr *= freqAdj;

            // Convert dB reduction to linear and apply as band gain
            double linearGain = qPow(10.0, (m_bands[b].gain + gr) * 0.05);
            correction += bp * (linearGain - 1.0);
        }

        output[i] += correction;
    }

    m_stats.totalSamples += n;
    m_stats.numBands = nb;
    m_stats.sampleRate = m_sampleRate;
    m_timeSum += timer.elapsed();
    if (m_stats.totalSamples > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalSamples / qMax(1, n));

    emit processingCompleted(n, peakReduction);
    return output;
}

/* ---- Process with sidechain ---- */

QVector<double> DynamicEQ2::processWithSidechain(const QVector<double>& input,
                                                   const QVector<double>& sidechain)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), sidechain.size());
    int nb = m_bands.size();
    if (n == 0 || nb == 0) return input;

    QVector<BiquadCoeffs> coeffs(nb);
    for (int b = 0; b < nb; ++b)
        coeffs[b] = designBandpass(m_bands[b].frequency, m_bands[b].qFactor);

    QVector<double> output = input;
    double peakReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double correction = 0.0;

        for (int b = 0; b < nb; ++b) {
            // Filter sidechain for detection
            double bpSide = applyBiquad(sidechain[i], coeffs[b], m_bpSideState[b]);

            // Compute envelope from sidechain
            double env = computeEnvelope(bpSide, m_bands[b].attack,
                                          m_bands[b].release,
                                          m_envelopeState[b]);

            // Filter main signal for gain application
            double bpMain = applyBiquad(input[i], coeffs[b], m_bpState[b]);

            double gr = gainReduction(env, m_bands[b].threshold, m_bands[b].ratio);
            peakReduction = qMin(peakReduction, gr);

            double linearGain = qPow(10.0, (m_bands[b].gain + gr) * 0.05);
            correction += bpMain * (linearGain - 1.0);
        }

        output[i] += correction;
    }

    m_stats.totalSamples += n;
    m_stats.numBands = nb;
    m_timeSum += timer.elapsed();
    if (m_stats.totalSamples > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalSamples / qMax(1, n));

    emit processingCompleted(n, peakReduction);
    return output;
}

/* ---- Reset ---- */

void DynamicEQ2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    std::fill(m_envelopeState.begin(), m_envelopeState.end(), 0.0);
    std::fill(m_bpState.begin(), m_bpState.end(), BiquadState{});
    std::fill(m_bpSideState.begin(), m_bpSideState.end(), BiquadState{});
}
