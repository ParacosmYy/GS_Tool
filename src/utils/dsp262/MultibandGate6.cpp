/**
 * @file MultibandGate6.cpp
 * @brief MultibandGate6 实现
 *
 * 实现多频段门控：交叉滤波器组独立逐带阈值释放包络整形。
 */

#include "utils/dsp262/MultibandGate6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MultibandGate6::MultibandGate6(int numBands, QObject *parent)
    : QObject(parent), m_numBands(qMax(2, numBands))
{
    m_configs.resize(m_numBands);
    m_bandStates.resize(m_numBands);
    m_bandGains.resize(m_numBands, 1.0);
    m_bandLevels.resize(m_numBands, -120.0);

    // Default crossover frequencies (log-spaced)
    m_crossoverFreqs.resize(m_numBands - 1);
    for (int i = 0; i < m_numBands - 1; ++i) {
        double frac = static_cast<double>(i + 1) / m_numBands;
        m_crossoverFreqs[i] = 80.0 * qPow(1000.0 / 80.0, frac);
    }
}

MultibandGate6::~MultibandGate6() = default;

/* ---- Configuration ---- */

void MultibandGate6::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(8000.0, sampleRate);
}

void MultibandGate6::setCrossoverFrequencies(const QVector<double>& freqs)
{
    int needed = m_numBands - 1;
    for (int i = 0; i < qMin(freqs.size(), needed); ++i)
        m_crossoverFreqs[i] = qBound(20.0, freqs[i], m_sampleRate / 2.0 - 100.0);
}

void MultibandGate6::setBandConfig(int band, const BandConfig& config)
{
    if (band >= 0 && band < m_numBands)
        m_configs[band] = config;
}

/* ---- dB <-> linear ---- */

double MultibandGate6::dbToLinear(double db) { return qPow(10.0, db / 20.0); }
double MultibandGate6::linearToDb(double lin)
{
    return (lin > 1e-10) ? 20.0 * qLog10(lin) : -120.0;
}

/* ---- 2nd-order Linkwitz-Riley filters ---- */

double MultibandGate6::processLP(FilterState& s, double input, double freq)
{
    // Butterworth 2nd-order lowpass (part of LR4)
    double omega = 2.0 * M_PI * freq / m_sampleRate;
    double sinO = qSin(omega);
    double cosO = qCos(omega);
    double alpha = sinO * 0.7071; // Q = 0.7071 for Butterworth
    double b0 = (1.0 - cosO) / 2.0;
    double b1 = 1.0 - cosO;
    double b2 = (1.0 - cosO) / 2.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosO;
    double a2 = 1.0 - alpha;

    double output = (b0 * input + b1 * s.x1 + b2 * s.x2 - a1 * s.y1 - a2 * s.y2) / a0;
    s.x2 = s.x1; s.x1 = input;
    s.y2 = s.y1; s.y1 = output;
    return output;
}

double MultibandGate6::processHP(FilterState& s, double input, double freq)
{
    // Butterworth 2nd-order highpass (part of LR4)
    double omega = 2.0 * M_PI * freq / m_sampleRate;
    double sinO = qSin(omega);
    double cosO = qCos(omega);
    double alpha = sinO * 0.7071;
    double b0 = (1.0 + cosO) / 2.0;
    double b1 = -(1.0 + cosO);
    double b2 = (1.0 + cosO) / 2.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosO;
    double a2 = 1.0 - alpha;

    double output = (b0 * input + b1 * s.x1 + b2 * s.x2 - a1 * s.y1 - a2 * s.y2) / a0;
    s.x2 = s.x1; s.x1 = input;
    s.y2 = s.y1; s.y1 = output;
    return output;
}

/* ---- Split signal into bands ---- */

QVector<QVector<double>> MultibandGate6::splitBands(const QVector<double>& input)
{
    int n = input.size();
    QVector<QVector<double>> bands(m_numBands);
    for (auto& b : bands) b.resize(n);

    // Cascaded crossover: band 0 = LP(cutoff[0]), band N-1 = HP(cutoff[N-2])
    // Intermediate: HP(cutoff[i-1]) then LP(cutoff[i])
    QVector<double> workLP(n), workHP(n);

    for (int s = 0; s < n; ++s) {
        double sample = input[s];
        double lpOut = sample, hpOut = sample;

        for (int band = 0; band < m_numBands; ++band) {
            BandState& state = m_bandStates[band];
            if (state.lpState.isEmpty()) {
                state.lpState.resize(2);
                state.hpState.resize(2);
            }

            if (band < m_numBands - 1) {
                double freq = m_crossoverFreqs[band];
                // Two passes for LR4 (cascade two 2nd-order)
                double lp1 = processLP(state.lpState[0], sample, freq);
                double lp2 = processLP(state.lpState[1], lp1, freq);
                double hp1 = processHP(state.hpState[0], sample, freq);
                double hp2 = processHP(state.hpState[1], hp1, freq);

                bands[band][s] = lp2;
                sample = hp2; // Pass HP result to next band
            } else {
                bands[band][s] = sample;
            }
        }
    }
    return bands;
}

/* ---- Compute gate gain ---- */

double MultibandGate6::computeGateGain(double envelopeLin, const BandConfig& cfg) const
{
    double envDb = linearToDb(envelopeLin);
    double threshold = cfg.threshold;

    if (envDb >= threshold) return 1.0; // Above threshold: open

    // Below threshold: apply gating with range
    double rangeLin = dbToLinear(cfg.range);
    double fraction = (envDb - (-120.0)) / (threshold - (-120.0));
    fraction = qBound(0.0, fraction, 1.0);

    // Shaped release envelope: exponential curve
    double shapedGain = fraction * fraction * (3.0 - 2.0 * fraction); // smoothstep
    return rangeLin + (1.0 - rangeLin) * shapedGain;
}

/* ---- Process ---- */

QVector<double> MultibandGate6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return input;

    // Split into frequency bands
    QVector<QVector<double>> bands = splitBands(input);

    // Process each band independently
    QVector<double> output(n, 0.0);

    for (int band = 0; band < m_numBands; ++band) {
        BandState& state = m_bandStates[band];
        const BandConfig& cfg = m_configs[band];

        double attackCoeff = qExp(-1.0 / (cfg.attack * 0.001 * m_sampleRate));
        double releaseCoeff = qExp(-1.0 / (cfg.release * 0.001 * m_sampleRate));

        for (int s = 0; s < n; ++s) {
            double absVal = qAbs(bands[band][s]);

            // Envelope follower with attack/release
            double coeff = (absVal > state.envelope) ? attackCoeff : releaseCoeff;
            state.envelope = coeff * state.envelope + (1.0 - coeff) * absVal;

            // Compute gate gain
            state.gain = computeGateGain(state.envelope, cfg);

            // Apply gain to band signal
            output[s] += bands[band][s] * state.gain;
        }

        m_bandGains[band] = linearToDb(state.gain);
        m_bandLevels[band] = linearToDb(state.envelope);
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples += n;
    m_stats.numBands = m_numBands;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(n, m_numBands, elapsed);
    return output;
}

/* ---- Accessors ---- */

QVector<double> MultibandGate6::bandGains() const { return m_bandGains; }
QVector<double> MultibandGate6::bandLevels() const { return m_bandLevels; }

/* ---- Reset ---- */

void MultibandGate6::resetStatistics()
{
    m_bandStates.clear();
    m_bandStates.resize(m_numBands);
    m_bandGains.fill(1.0);
    m_bandLevels.fill(-120.0);
    m_stats = Stats{};
    m_timeSum = 0.0;
}
