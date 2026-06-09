/**
 * @file Expander9.cpp
 * @brief Expander9 实现
 *
 * 实现多频带扩展器：独立逐频带下行扩展与频带间串扰抑制。
 */

#include "utils/dsp245/Expander9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander9::Expander9(int sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(sampleRate) {}
Expander9::~Expander9() = default;

/* ---- Configuration ---- */

void Expander9::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); }
void Expander9::setCrosstalkSuppression(double strength) { m_xtalkSuppress = qBound(0.0, strength, 1.0); }

void Expander9::setBands(const QVector<BandConfig>& bands)
{
    m_bands = bands;
    int n = bands.size();
    m_state.envelope.resize(n);
    m_state.gainReduction.resize(n);
    m_state.prevGain.resize(n);
    m_state.envelope.fill(0.0);
    m_state.gainReduction.fill(1.0);
    m_state.prevGain.fill(1.0);

    // Design crossover filters
    m_lowCoeffs.resize(n);
    m_highCoeffs.resize(n);
    m_lowState.resize(n);
    m_highState.resize(n);
    for (int i = 0; i < n; ++i) {
        double freq = bands[i].lowFreq;
        if (freq > 0.0 && freq < m_sampleRate * 0.5) {
            m_lowCoeffs[i] = designLR4(freq);
            m_highCoeffs[i] = designLR4(qMin(bands[i].highFreq, m_sampleRate * 0.49));
        }
        m_lowState[i] = FilterState{};
        m_highState[i] = FilterState{};
    }
    m_stats.numBands = n;
}

/* ---- Design Linkwitz-Riley 4th-order crossover ---- */

Expander9::FilterCoeffs Expander9::designLR4(double freq) const
{
    // 2nd-order butterworth coefficients, then cascade for LR4
    double w0 = 2.0 * M_PI * freq / m_sampleRate;
    double cosW = qCos(w0);
    double sinW = qSin(w0);
    double Q = 0.7071067811865476;  // Butterworth Q
    double alpha = sinW / (2.0 * Q);

    double b0 = (1.0 - cosW) / 2.0;
    double b1 = 1.0 - cosW;
    double b2 = (1.0 - cosW) / 2.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosW;
    double a2 = 1.0 - alpha;

    // Normalize and cascade: LR4 = two BW2 in series
    FilterCoeffs c;
    c.b0 = (b0 / a0) * (b0 / a0);
    c.b1 = 2.0 * (b0 / a0) * (b1 / a0);
    c.b2 = (b1 / a0) * (b1 / a0) + 2.0 * (b0 / a0) * (b2 / a0);
    c.b3 = 2.0 * (b0 / a0) * (b2 / a0);
    c.b4 = (b2 / a0) * (b2 / a0);
    c.a1 = 2.0 * a1 / a0;
    c.a2 = (a1 / a0) * (a1 / a0) + 2.0 * a2 / a0;
    c.a3 = 2.0 * a1 * a2 / (a0 * a0);
    c.a4 = (a2 / a0) * (a2 / a0);
    return c;
}

/* ---- Apply 4th-order filter ---- */

double Expander9::applyFilter(double sample, const FilterCoeffs& c, FilterState& s) const
{
    double w0 = sample - c.a1 * s.w1 - c.a2 * s.w2 - c.a3 * s.w3 - c.a4 * s.w4;
    double out = c.b0 * w0 + c.b1 * s.w1 + c.b2 * s.w2 + c.b3 * s.w3 + c.b4 * s.w4;
    s.w4 = s.w3; s.w3 = s.w2; s.w2 = s.w1; s.w1 = w0;
    return out;
}

/* ---- Time constant coefficient ---- */

double Expander9::timeCoeff(double timeMs) const
{
    return qExp(-1.0 / (timeMs * 0.001 * m_sampleRate));
}

/* ---- Expansion gain ---- */

double Expander9::expandGain(double levelDb, const BandConfig& cfg) const
{
    if (levelDb >= cfg.threshold) return 1.0;
    double belowDb = cfg.threshold - levelDb;
    double gainDb = -belowDb * (cfg.ratio - 1.0) / cfg.ratio;
    // Limit expansion to -60dB
    gainDb = qMax(gainDb, -60.0);
    return qPow(10.0, gainDb / 20.0);
}

/* ---- Crosstalk suppression ---- */

void Expander9::suppressCrosstalk(QVector<QVector<double>>& bandSignals)
{
    int n = bandSignals.size();
    if (n < 2) return;

    // Average energy across bands
    double avgEnergy = 0.0;
    for (int b = 0; b < n; ++b) {
        double e = 0.0;
        for (int i = 0; i < bandSignals[b].size(); ++i)
            e += bandSignals[b][i] * bandSignals[b][i];
        avgEnergy += e / bandSignals[b].size();
    }
    avgEnergy /= n;

    // Suppress bands whose energy is far below average (crosstalk)
    for (int b = 0; b < n; ++b) {
        double e = 0.0;
        for (int i = 0; i < bandSignals[b].size(); ++i)
            e += bandSignals[b][i] * bandSignals[b][i];
        e /= bandSignals[b].size();
        if (avgEnergy > 1e-10) {
            double ratio = e / avgEnergy;
            if (ratio < 0.01) {
                // Likely crosstalk: apply additional suppression
                double suppress = 1.0 - m_xtalkSuppress * (1.0 - qSqrt(ratio) * 10.0);
                suppress = qBound(0.0, suppress, 1.0);
                for (int i = 0; i < bandSignals[b].size(); ++i)
                    bandSignals[b][i] *= suppress;
            }
        }
    }
}

/* ---- Process mono ---- */

QVector<double> Expander9::processMono(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    int nb = m_bands.size();
    if (nb == 0) return input;

    // Split into frequency bands using crossover filters
    QVector<QVector<double>> bandSignals(nb);
    for (int b = 0; b < nb; ++b)
        bandSignals[b].resize(n);

    for (int i = 0; i < n; ++i) {
        double sample = input[i];
        for (int b = 0; b < nb; ++b) {
            if (m_bands[b].lowFreq <= 0.0 && b == 0) {
                bandSignals[b][i] = sample;
            } else {
                bandSignals[b][i] = applyFilter(sample, m_lowCoeffs[b], m_lowState[b]);
            }
        }
    }

    // Apply crosstalk suppression
    if (m_xtalkSuppress > 0.0)
        suppressCrosstalk(bandSignals);

    // Process each band with expansion
    QVector<double> output(n, 0.0);
    for (int b = 0; b < nb; ++b) {
        double attackCoeff = timeCoeff(m_bands[b].attack);
        double releaseCoeff = timeCoeff(m_bands[b].release);

        for (int i = 0; i < n; ++i) {
            double absVal = qAbs(bandSignals[b][i]);

            // Envelope follower
            if (absVal > m_state.envelope[b])
                m_state.envelope[b] = attackCoeff * m_state.envelope[b] + (1.0 - attackCoeff) * absVal;
            else
                m_state.envelope[b] = releaseCoeff * m_state.envelope[b] + (1.0 - releaseCoeff) * absVal;

            // Convert to dB
            double levelDb = 20.0 * qLn(qMax(m_state.envelope[b], 1e-10)) / M_LN2 * 0.30103;

            // Compute expansion gain
            double targetGain = expandGain(levelDb, m_bands[b]);

            // Smooth gain changes
            m_state.gainReduction[b] = 0.999 * m_state.gainReduction[b] + 0.001 * targetGain;

            // Apply gain and makeup
            double makeupLin = qPow(10.0, m_bands[b].makeupGain / 20.0);
            output[i] += bandSignals[b][i] * m_state.gainReduction[b] * makeupLin;
        }
    }

    m_stats.framesProcessed += n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Process stereo (interleaved) ---- */

QVector<double> Expander9::process(const QVector<double>& input)
{
    // Deinterleave, process each channel, reinterleave
    int n = input.size() / 2;
    QVector<double> left(n), right(n);
    for (int i = 0; i < n; ++i) {
        left[i] = input[2 * i];
        right[i] = input[2 * i + 1];
    }
    QVector<double> leftOut = processMono(left);
    // Reset filter states for right channel processing
    for (int b = 0; b < m_lowState.size(); ++b) {
        m_lowState[b] = FilterState{};
        m_highState[b] = FilterState{};
    }
    QVector<double> rightOut = processMono(right);

    QVector<double> output(2 * n);
    for (int i = 0; i < n; ++i) {
        output[2 * i] = leftOut[i];
        output[2 * i + 1] = rightOut[i];
    }
    return output;
}

/* ---- Accessors ---- */

QVector<double> Expander9::bandGainReduction() const { return m_state.gainReduction; }

/* ---- Reset ---- */

void Expander9::resetStatistics()
{
    m_state.envelope.clear(); m_state.gainReduction.clear(); m_state.prevGain.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
