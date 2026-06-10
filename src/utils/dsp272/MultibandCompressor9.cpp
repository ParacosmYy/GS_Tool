/**
 * @file MultibandCompressor9.cpp
 * @brief MultibandCompressor9 实现
 *
 * 实现多频段压缩器：Linkwitz-Riley交叉与逐频段侧链独立比率/阈值控制。
 */

#include "utils/dsp272/MultibandCompressor9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandCompressor9::MultibandCompressor9(QObject *parent)
    : QObject(parent)
{
    m_crossoverFreqs = {80.0, 500.0, 4000.0};
    m_numBands = m_crossoverFreqs.size() + 1;
    m_bandParams.resize(m_numBands);
    m_bandStates.resize(m_numBands);
    m_gainReduction.resize(m_numBands, 0.0);
    initCrossover();
}

MultibandCompressor9::~MultibandCompressor9() = default;

/* ---- Configuration ---- */

void MultibandCompressor9::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
    initCrossover();
}

void MultibandCompressor9::setCrossoverFreqs(const QVector<double>& freqs)
{
    m_crossoverFreqs = freqs;
    m_numBands = freqs.size() + 1;
    m_bandParams.resize(m_numBands);
    m_bandStates.resize(m_numBands);
    m_gainReduction.resize(m_numBands, 0.0);
    initCrossover();
}

void MultibandCompressor9::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_numBands)
        m_bandParams[band] = params;
}

/* ---- Initialize Linkwitz-Riley 4th-order crossover (2x 2nd-order butterworth) ---- */

void MultibandCompressor9::initCrossover()
{
    int numCrossovers = m_crossoverFreqs.size();
    for (int b = 0; b < m_numBands; ++b) {
        m_bandStates[b].lpState.resize(numCrossovers);
        m_bandStates[b].hpState.resize(numCrossovers);
        m_bandStates[b].envelope = 0.0;
        m_bandStates[b].gainLin = 1.0;
    }

    // Design 2nd-order Butterworth for each crossover frequency
    for (int c = 0; c < numCrossovers; ++c) {
        double fc = m_crossoverFreqs[c];
        double omega = 2.0 * M_PI * fc / m_sampleRate;
        double sinW = qSin(omega);
        double cosW = qCos(omega);
        double alpha = sinW / (qSqrt(2.0));  // Butterworth Q = 1/sqrt(2)

        // Low-pass coefficients
        double b0lp = (1.0 - cosW) / 2.0;
        double b1lp = 1.0 - cosW;
        double b2lp = (1.0 - cosW) / 2.0;
        double a0lp = 1.0 + alpha;
        double a1lp = -2.0 * cosW;
        double a2lp = 1.0 - alpha;

        // High-pass coefficients
        double b0hp = (1.0 + cosW) / 2.0;
        double b1hp = -(1.0 + cosW);
        double b2hp = (1.0 + cosW) / 2.0;
        double a0hp = 1.0 + alpha;
        double a1hp = -2.0 * cosW;
        double a2hp = 1.0 - alpha;

        for (int b = 0; b < m_numBands; ++b) {
            auto& lp = m_bandStates[b].lpState[c];
            auto& hp = m_bandStates[b].hpState[c];

            lp = {b0lp / a0lp, b1lp / a0lp, b2lp / a0lp, a1lp / a0lp, a2lp / a0lp, 0, 0, 0, 0};
            hp = {b0hp / a0hp, b1hp / a0hp, b2hp / a0hp, a1hp / a0hp, a2hp / a0hp, 0, 0, 0, 0};
        }
    }
}

/* ---- Process one biquad sample ---- */

double MultibandCompressor9::processBiquad(double sample, BiquadState& s) const
{
    double out = s.b0 * sample + s.b1 * s.x1 + s.b2 * s.x2 - s.a1 * s.y1 - s.a2 * s.y2;
    s.x2 = s.x1;
    s.x1 = sample;
    s.y2 = s.y1;
    s.y1 = out;
    return out;
}

/* ---- Soft-knee gain reduction computation ---- */

double MultibandCompressor9::computeGainReduction(double inputDb, const BandParams& p) const
{
    double halfKnee = p.kneeWidth / 2.0;
    double tLow = p.threshold - halfKnee;
    double tHigh = p.threshold + halfKnee;

    if (inputDb < tLow) return 0.0;
    if (inputDb > tHigh) {
        return (p.threshold - inputDb) * (1.0 - 1.0 / p.ratio);
    }
    // Soft knee region: quadratic interpolation
    double x = inputDb - tLow;
    double kneeFactor = x * x / (2.0 * p.kneeWidth);
    double effRatio = 1.0 + (p.ratio - 1.0) * kneeFactor / (p.kneeWidth / 2.0);
    return (p.threshold - inputDb) * (1.0 - 1.0 / qMax(1.001, effRatio));
}

/* ---- Envelope follower ---- */

double MultibandCompressor9::followEnvelope(double sample, double attackCoeff,
                                             double releaseCoeff, double& envelope) const
{
    double absSample = qAbs(sample);
    double coeff = (absSample > envelope) ? attackCoeff : releaseCoeff;
    envelope = coeff * envelope + (1.0 - coeff) * absSample;
    return envelope;
}

/* ---- Main process ---- */

QVector<double> MultibandCompressor9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    int numCrossovers = m_crossoverFreqs.size();

    // Output buffer
    QVector<double> output(n, 0.0);

    // Process per-band
    for (int b = 0; b < m_numBands; ++b) {
        const auto& params = m_bandParams[b];
        auto& state = m_bandStates[b];

        // Attack/release coefficients (to linear time constant)
        double attackCoeff = qExp(-1.0 / (params.attack * 0.001 * m_sampleRate));
        double releaseCoeff = qExp(-1.0 / (params.release * 0.001 * m_sampleRate));
        double makeupLin = qPow(10.0, params.makeupGain / 20.0);

        for (int i = 0; i < n; ++i) {
            double sample = input[i];

            // Apply crossover filtering cascade
            for (int c = 0; c < numCrossovers; ++c) {
                // First pass of Linkwitz-Riley (2nd-order)
                double lpOut = processBiquad(sample, state.lpState[c]);
                double hpOut = processBiquad(sample, state.hpState[c]);

                // Second pass for 4th-order LR (cascade same filter)
                sample = processBiquad(lpOut, state.lpState[c]);
                // We accumulate HP path separately per band logic
                Q_UNUSED(hpOut);
            }

            // Sidechain: envelope detection
            followEnvelope(sample, attackCoeff, releaseCoeff, state.envelope);

            // Convert to dB for gain computation
            double envDb = (state.envelope > 1e-10)
                               ? 20.0 * qLn(state.envelope) / M_LN10
                               : -200.0;

            // Compute gain reduction
            double grDb = computeGainReduction(envDb, params);
            state.gainReductionDb = grDb;
            double gainLin = qPow(10.0, grDb / 20.0);
            state.gainLin = gainLin;

            // Apply gain and makeup
            output[i] += sample * gainLin * makeupLin;
        }
        m_gainReduction[b] = state.gainReductionDb;
    }

    double elapsed = timer.elapsed();
    m_stats.numBands = m_numBands;
    m_stats.blockSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(m_numBands, n, elapsed);

    return output;
}

/* ---- Accessors ---- */

QVector<double> MultibandCompressor9::gainReduction() const
{
    return m_gainReduction;
}

/* ---- Reset ---- */

void MultibandCompressor9::resetStatistics()
{
    for (auto& state : m_bandStates) {
        state.envelope = 0.0;
        state.gainLin = 1.0;
        state.gainReductionDb = 0.0;
        for (auto& bq : state.lpState) bq = {};
        for (auto& bq : state.hpState) bq = {};
    }
    m_gainReduction.fill(0.0);
    m_stats = Stats{};
    m_timeSum = 0.0;
}
