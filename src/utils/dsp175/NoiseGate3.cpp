/**
 * @file NoiseGate3.cpp
 * @brief NoiseGate3 实现
 *
 * 实现噪声门：前瞻缓冲、迟滞阈值、攻击/释放/保持平滑增益。
 */

#include "utils/dsp175/NoiseGate3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NoiseGate3::NoiseGate3(QObject *parent)
    : QObject(parent)
{
    setSampleRate(m_sampleRate);
}

NoiseGate3::~NoiseGate3() = default;

/* ---- Configuration ---- */

void NoiseGate3::setThreshold(double db) { m_thresholdDb = db; }
void NoiseGate3::setHysteresis(double db) { m_hysteresisDb = qMax(0.0, db); }

void NoiseGate3::setAttack(double ms)
{
    m_attackMs = qMax(0.01, ms);
    m_attackCoeff = qExp(-1.0 / (m_attackMs * m_sampleRate * 0.001));
}

void NoiseGate3::setRelease(double ms)
{
    m_releaseMs = qMax(0.01, ms);
    m_releaseCoeff = qExp(-1.0 / (m_releaseMs * m_sampleRate * 0.001));
}

void NoiseGate3::setHold(double ms)
{
    m_holdMs = qMax(0.0, ms);
    m_holdSamples = static_cast<int>(m_holdMs * m_sampleRate * 0.001);
}

void NoiseGate3::setLookaheadSamples(int samples)
{
    m_lookahead = qMax(0, samples);
    m_lookaheadBuf.resize(m_lookahead, 0.0);
    m_laWritePos = 0;
    m_laReadPos = 0;
    m_laFilled = false;
}

void NoiseGate3::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    setAttack(m_attackMs);
    setRelease(m_releaseMs);
    setHold(m_holdMs);
    setLookaheadSamples(m_lookahead);
}

/* ---- Helpers ---- */

double NoiseGate3::dbToLinear(double db)
{
    return qPow(10.0, db * 0.05);
}

double NoiseGate3::linearToDb(double linear)
{
    return (linear > 1e-10) ? 20.0 * qLog10(linear) : -200.0;
}

/* ---- Envelope follower ---- */

double NoiseGate3::computeEnvelope(double sample)
{
    double abs = qAbs(sample);
    double coeff = (abs > m_envelope) ? m_attackCoeff : m_releaseCoeff;
    m_envelope = coeff * m_envelope + (1.0 - coeff) * abs;
    return m_envelope;
}

/* ---- Smooth gain ---- */

double NoiseGate3::smoothGain(double target, double coeff)
{
    m_gain = coeff * m_gain + (1.0 - coeff) * target;
    return m_gain;
}

/* ---- Process one sample ---- */

double NoiseGate3::processOne(double sample)
{
    double env = computeEnvelope(sample);
    double envDb = linearToDb(env);

    /* Hysteresis threshold logic */
    double openThresh = m_thresholdDb;
    double closeThresh = m_thresholdDb - m_hysteresisDb;

    bool wasOpen = m_gateOpen;
    if (!m_gateOpen && envDb >= openThresh) {
        m_gateOpen = true;
        m_holdCounter = m_holdSamples;
    } else if (m_gateOpen && envDb < closeThresh) {
        if (m_holdCounter > 0) {
            m_holdCounter--;
        } else {
            m_gateOpen = false;
        }
    } else if (m_gateOpen && envDb >= closeThresh) {
        m_holdCounter = m_holdSamples;
    }

    if (wasOpen != m_gateOpen)
        emit gateStateChanged(m_gateOpen);

    /* Apply gain with smooth attack/release */
    double target = m_gateOpen ? 1.0 : 0.0;
    double coeff = m_gateOpen ? m_attackCoeff : m_releaseCoeff;
    double gain = smoothGain(target, coeff);

    return sample * gain;
}

/* ---- Process buffer with lookahead ---- */

QVector<double> NoiseGate3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    if (m_lookahead == 0) {
        /* No lookahead: direct processing */
        for (int i = 0; i < n; ++i)
            output[i] = processOne(input[i]);
    } else {
        /* Lookahead: pre-analyze envelope, then apply gain */
        /* Step 1: Compute envelope with lookahead */
        QVector<double> envelope(n);
        for (int i = 0; i < n; ++i)
            envelope[i] = computeEnvelope(input[i]);

        /* Step 2: Lookahead gate decision */
        QVector<int> gateState(n, 0);
        for (int i = 0; i < n; ++i) {
            double maxEnv = envelope[i];
            /* Look ahead within window */
            int laEnd = qMin(i + m_lookahead, n);
            for (int j = i; j < laEnd; ++j)
                maxEnv = qMax(maxEnv, envelope[j]);

            double envDb = linearToDb(maxEnv);
            if (envDb >= m_thresholdDb)
                gateState[i] = 1;
        }

        /* Step 3: Propagate hold time */
        int holdRemaining = 0;
        for (int i = 0; i < n; ++i) {
            if (gateState[i] == 1) {
                holdRemaining = m_holdSamples;
            } else if (holdRemaining > 0) {
                gateState[i] = 1;
                holdRemaining--;
            }
        }

        /* Step 4: Apply smooth gain */
        m_gain = 0.0;
        for (int i = 0; i < n; ++i) {
            double target = gateState[i] ? 1.0 : 0.0;
            double coeff = gateState[i] ? m_attackCoeff : m_releaseCoeff;
            m_gain = coeff * m_gain + (1.0 - coeff) * target;
            output[i] = input[i] * m_gain;
        }
    }

    /* Update stats */
    int gated = 0;
    for (int i = 0; i < n; ++i)
        if (qAbs(output[i]) < 1e-10) ++gated;
    m_stats.totalSamples += n;
    m_stats.gatedSamples += gated;
    m_stats.openRatio = (m_stats.totalSamples > 0)
        ? 1.0 - static_cast<double>(m_stats.gatedSamples) / m_stats.totalSamples
        : 0.0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1ULL, m_stats.totalSamples / qMax(1, n));

    emit processingCompleted(n, m_stats.openRatio);
    return output;
}

/* ---- Reset ---- */

void NoiseGate3::reset()
{
    m_envelope = 0.0;
    m_gain = 0.0;
    m_gateOpen = false;
    m_holdCounter = 0;
    m_lookaheadBuf.fill(0.0);
    m_laWritePos = 0;
    m_laReadPos = 0;
    m_laFilled = false;
}

void NoiseGate3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
