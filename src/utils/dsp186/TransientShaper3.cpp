/**
 * @file TransientShaper3.cpp
 * @brief TransientShaper3 实现
 *
 * 实现瞬态塑形器：快速/慢速包络差分、瞬态/持续分离、多频带处理。
 */

#include "utils/dsp186/TransientShaper3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransientShaper3::TransientShaper3(QObject *parent) : QObject(parent)
{
    // Default: single fullband
    BandParams fullband;
    fullband.attack = 0.01;
    fullband.sustain = 0.1;
    fullband.transientGain = 1.0;
    fullband.sustainGain = 1.0;
    m_bands.append(fullband);
    m_envStates.append(EnvState{});
}

TransientShaper3::~TransientShaper3() = default;

/* ---- Configuration ---- */

void TransientShaper3::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); }

void TransientShaper3::setBandParams(const QVector<BandParams>& bands)
{
    m_bands = bands.isEmpty() ? QVector<BandParams>{BandParams{}} : bands;
    m_envStates.fill(EnvState{}, m_bands.size());
}

/* ---- Envelope computation (attack/release follower) ---- */

QVector<double> TransientShaper3::computeEnvelope(const QVector<double>& input,
                                                   double attack, double release) const
{
    int n = input.size();
    if (n == 0) return {};

    // Convert time constants to coefficients
    double atkCoeff = (attack > 0.0) ? qExp(-1.0 / (attack * m_sampleRate)) : 0.0;
    double relCoeff = (release > 0.0) ? qExp(-1.0 / (release * m_sampleRate)) : 0.0;

    QVector<double> env(n);
    double e = 0.0;
    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(input[i]);
        double coeff = (absVal > e) ? atkCoeff : relCoeff;
        e = coeff * e + (1.0 - coeff) * absVal;
        env[i] = e;
    }
    return env;
}

/* ---- Envelope difference ---- */

QVector<double> TransientShaper3::envelopeDifference(const QVector<double>& fast,
                                                      const QVector<double>& slow) const
{
    int n = qMin(fast.size(), slow.size());
    QVector<double> diff(n);
    for (int i = 0; i < n; ++i)
        diff[i] = qMax(0.0, fast[i] - slow[i]);
    return diff;
}

/* ---- Crossover split (2nd-order Linkwitz-Riley style) ---- */

void TransientShaper3::crossover(const QVector<double>& input, double freq,
                                  QVector<double>& low, QVector<double>& high) const
{
    int n = input.size();
    low.resize(n);
    high.resize(n);
    if (freq <= 0.0 || freq >= m_sampleRate / 2.0) {
        low = input;
        high.fill(0.0, n);
        return;
    }

    // Simple 1st-order IIR lowpass
    double omega = 2.0 * M_PI * freq / m_sampleRate;
    double alpha = qSin(omega) / (2.0 * 0.7071);
    double b0 = (1.0 - qCos(omega)) / 2.0;
    double b1 = 1.0 - qCos(omega);
    double b2 = b0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * qCos(omega);
    double a2 = 1.0 - alpha;
    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;

    for (int i = 0; i < n; ++i) {
        double y = (b0 * input[i] + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        x2 = x1; x1 = input[i]; y2 = y1; y1 = y;
        low[i] = y;
        high[i] = input[i] - y;
    }
}

/* ---- Shape a single band ---- */

QVector<double> TransientShaper3::shapeBand(const QVector<double>& input,
                                             BandParams& params, EnvState& state)
{
    int n = input.size();
    if (n == 0) return {};

    double atkCoeff = qExp(-1.0 / (params.attack * m_sampleRate));
    double relCoeff = qExp(-1.0 / (params.sustain * m_sampleRate));

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(input[i]);

        // Update fast and slow envelopes
        double fastCoeff = (absVal > state.fastEnv) ? atkCoeff : relCoeff;
        state.fastEnv = fastCoeff * state.fastEnv + (1.0 - fastCoeff) * absVal;

        double slowCoeff = (absVal > state.slowEnv) ?
            qExp(-1.0 / (params.sustain * 2.0 * m_sampleRate)) :
            qExp(-1.0 / (params.sustain * m_sampleRate));
        state.slowEnv = slowCoeff * state.slowEnv + (1.0 - slowCoeff) * absVal;

        // Transient detection
        double transient = qMax(0.0, state.fastEnv - state.slowEnv);
        double sustain = state.slowEnv;
        double total = transient + sustain;
        double tRatio = (total > 1e-10) ? transient / total : 0.5;
        double sRatio = 1.0 - tRatio;

        // Apply gains
        double gain = tRatio * params.transientGain + sRatio * params.sustainGain;
        result[i] = input[i] * gain;
    }
    return result;
}

/* ---- Separate transient and sustain ---- */

void TransientShaper3::separateTransientSustain(const QVector<double>& input,
                                                  QVector<double>& transient,
                                                  QVector<double>& sustain)
{
    auto fastEnv = computeEnvelope(input, m_bands[0].attack, m_bands[0].attack * 0.5);
    auto slowEnv = computeEnvelope(input, m_bands[0].sustain, m_bands[0].sustain * 2.0);
    auto diff = envelopeDifference(fastEnv, slowEnv);

    int n = input.size();
    transient.resize(n);
    sustain.resize(n);
    for (int i = 0; i < n; ++i) {
        double total = fastEnv[i] + slowEnv[i];
        double tRatio = (total > 1e-10) ? diff[i] / total : 0.0;
        transient[i] = input[i] * tRatio;
        sustain[i] = input[i] * (1.0 - tRatio);
    }
}

/* ---- Process frame ---- */

QVector<double> TransientShaper3::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    int n = frame.size();
    if (n == 0) return {};

    QVector<double> output(n, 0.0);
    double peakT = 0.0;

    if (m_bands.size() == 1 && m_bands[0].lowFreq <= 0.0) {
        // Fullband mode
        auto shaped = shapeBand(frame, m_bands[0], m_envStates[0]);
        for (int i = 0; i < n; ++i) output[i] = shaped[i];
    } else {
        // Multiband: cascade crossovers
        QVector<double> remaining = frame;
        for (int b = 0; b < m_bands.size(); ++b) {
            QVector<double> low, high;
            if (b < m_bands.size() - 1 && m_bands[b].highFreq > 0.0) {
                crossover(remaining, m_bands[b].highFreq, low, high);
                auto shaped = shapeBand(low, m_bands[b], m_envStates[b]);
                for (int i = 0; i < n; ++i) output[i] += shaped[i];
                remaining = high;
            } else {
                auto shaped = shapeBand(remaining, m_bands[b], m_envStates[b]);
                for (int i = 0; i < n; ++i) output[i] += shaped[i];
                remaining.fill(0.0, n);
            }
        }
    }

    for (int i = 0; i < n; ++i)
        if (qAbs(output[i]) > peakT) peakT = qAbs(output[i]);

    m_stats.totalFrames++;
    m_stats.frameSize = n;
    m_stats.sampleRate = m_sampleRate;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(n, peakT);
    return output;
}

/* ---- Reset ---- */

void TransientShaper3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envStates.fill(EnvState{}, m_bands.size());
}
