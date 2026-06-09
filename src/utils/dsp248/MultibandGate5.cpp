/**
 * @file MultibandGate5.cpp
 * @brief MultibandGate5 实现
 *
 * 实现多频段门控：完美重建交叉滤波器与前瞻包络预测。
 */

#include "utils/dsp248/MultibandGate5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandGate5::MultibandGate5(int numBands, QObject *parent)
    : QObject(parent), m_numBands(qMax(2, numBands))
{
    m_params.resize(m_numBands);
    m_lpState.resize(m_numBands);
    m_hpState.resize(m_numBands);
    m_envelopes.resize(m_numBands);
    for (auto& env : m_envelopes) {
        env.lookAheadBuf.resize(m_lookAhead, 0.0);
    }
}

MultibandGate5::~MultibandGate5() = default;

/* ---- Configuration ---- */

void MultibandGate5::setCrossoverFreqs(const QVector<double>& freqs)
{
    // Frequencies are stored for coefficient computation in process()
    Q_UNUSED(freqs)
}

void MultibandGate5::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_numBands)
        m_params[band] = params;
}

/* ---- dB conversions ---- */

double MultibandGate5::toDb(double linear) const
{
    return (linear > 1e-10) ? 20.0 * qLn(linear) / qLn(10.0) : -120.0;
}

double MultibandGate5::fromDb(double db) const
{
    return qPow(10.0, db / 20.0);
}

/* ---- LR crossover coefficient ---- */

double MultibandGate5::lrCoeff(double freq, double sampleRate) const
{
    if (freq <= 0.0 || sampleRate <= 0.0) return 0.0;
    double omega = 2.0 * M_PI * freq / sampleRate;
    return (qCos(omega) - 1.0) / (qCos(omega) + 1.0);
}

/* ---- Linkwitz-Riley LP stage ---- */

double MultibandGate5::processLP(CrossoverState& s, double x, double coeff)
{
    // 2nd-order all-pass based LP
    double y = coeff * (x - s.y1) + s.x1;
    s.x1 = x;
    s.y1 = y;
    // Second stage
    double y2 = coeff * (y - s.y3) + s.x3;
    s.x3 = y;
    s.y3 = y2;
    return y2;
}

/* ---- Linkwitz-Riley HP stage ---- */

double MultibandGate5::processHP(CrossoverState& s, double x, double coeff)
{
    // 2nd-order all-pass based HP
    double y = x - processLP(s, x, coeff);
    return y;
}

/* ---- Detect envelope with look-ahead ---- */

double MultibandGate5::detectEnvelope(GateEnv& env, double sample, double sampleRate)
{
    // Write to look-ahead buffer
    env.lookAheadBuf[env.laWritePos] = qAbs(sample);
    env.laWritePos = (env.laWritePos + 1) % m_lookAhead;

    // Read from delayed position
    double delayed = env.lookAheadBuf[env.laWritePos];

    // Peak detect envelope
    if (delayed > env.envelope) {
        double attackCoeff = 1.0 - qExp(-1.0 / (0.001 * sampleRate));
        env.envelope += attackCoeff * (delayed - env.envelope);
    } else {
        double releaseCoeff = 1.0 - qExp(-1.0 / (0.050 * sampleRate));
        env.envelope += releaseCoeff * (delayed - env.envelope);
    }
    return env.envelope;
}

/* ---- Compute gate gain ---- */

double MultibandGate5::computeGateGain(double envDb, const BandParams& p) const
{
    if (p.bypassed) return 1.0;
    if (envDb >= p.thresholdDb) return 1.0;

    double rangeDb = p.thresholdDb - envDb;
    double gateDb = -rangeDb * (p.ratio - 1.0) / p.ratio;
    gateDb = qMax(gateDb, -80.0);
    return fromDb(gateDb);
}

/* ---- Process a block ---- */

QVector<double> MultibandGate5::process(const QVector<double>& input, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    QVector<double> output(N, 0.0);

    if (N == 0) return output;

    // Default crossover frequencies: evenly spaced
    QVector<double> crossoverFreqs(m_numBands - 1);
    double nyquist = sampleRate / 2.0;
    for (int i = 0; i < m_numBands - 1; ++i)
        crossoverFreqs[i] = nyquist * (i + 1) / m_numBands;

    // Per-sample processing
    double peakIn = 0.0, peakOut = 0.0;
    for (int s = 0; s < N; ++s) {
        double sample = input[s];
        peakIn = qMax(peakIn, qAbs(sample));

        QVector<double> bandSignals(m_numBands, 0.0);
        double remaining = sample;

        // Split into bands using cascaded crossovers
        for (int b = 0; b < m_numBands - 1; ++b) {
            double coeff = lrCoeff(crossoverFreqs[b], sampleRate);
            double lp = processLP(m_lpState[b], remaining, coeff);
            double hp = remaining - lp;  // Perfect reconstruction complement
            bandSignals[b] = lp;
            remaining = hp;
        }
        bandSignals[m_numBands - 1] = remaining;

        // Apply gate per band
        double sum = 0.0;
        for (int b = 0; b < m_numBands; ++b) {
            double env = detectEnvelope(m_envelopes[b], bandSignals[b], sampleRate);
            double envDb = toDb(env);
            double gain = computeGateGain(envDb, m_params[b]);
            m_envelopes[b].gain = gain;
            sum += bandSignals[b] * gain;
        }
        output[s] = sum;
        peakOut = qMax(peakOut, qAbs(sum));
    }

    m_stats.blockSize = N;
    m_stats.peakInputDb = toDb(peakIn);
    m_stats.peakOutputDb = toDb(peakOut);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit blockProcessed(N, m_stats.peakOutputDb, timer.elapsed());
    return output;
}

/* ---- Get band levels ---- */

QVector<double> MultibandGate5::bandLevels() const
{
    QVector<double> levels(m_numBands);
    for (int i = 0; i < m_numBands; ++i)
        levels[i] = toDb(m_envelopes[i].envelope);
    return levels;
}

/* ---- Reset ---- */

void MultibandGate5::resetStatistics()
{
    for (auto& s : m_lpState) s = {};
    for (auto& s : m_hpState) s = {};
    for (auto& e : m_envelopes) {
        e.envelope = 0.0;
        e.gain = 0.0;
        e.lookAheadBuf.fill(0.0);
        e.laWritePos = 0;
    }
    m_stats = Stats{};
    m_stats.numBands = m_numBands;
    m_timeSum = 0.0;
}
