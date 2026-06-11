/**
 * @file NoiseGate10.cpp
 * @brief NoiseGate10 实现
 *
 * 实现噪声门：迟滞阈值与前瞻缓冲区实现无咔嗒声瞬态保护音频门控。
 */

#include "utils/dsp289/NoiseGate10.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

NoiseGate10::NoiseGate10(QObject *parent)
    : QObject(parent)
{
    m_lookaheadBuf.resize(m_lookahead, 0.0);
}

NoiseGate10::~NoiseGate10() = default;

/* ---- Configuration ---- */

void NoiseGate10::setThreshold(double dB) { m_threshold = dB; }
void NoiseGate10::setCloseThreshold(double dB) { m_closeThreshold = dB; }
void NoiseGate10::setAttack(double ms) { m_attack = qMax(0.01, ms); }
void NoiseGate10::setRelease(double ms) { m_release = qMax(0.01, ms); }
void NoiseGate10::setHold(double ms) { m_hold = qMax(0.0, ms); }
void NoiseGate10::setLookahead(int samples) {
    m_lookahead = qBound(0, samples, 4096);
    m_lookaheadBuf.resize(m_lookahead, 0.0);
    m_lookaheadPos = 0;
}
void NoiseGate10::setSampleRate(double sr) { m_sampleRate = qMax(1.0, sr); }

/* ---- Utility ---- */

double NoiseGate10::dbToLinear(double dB) { return qPow(10.0, dB / 20.0); }
double NoiseGate10::linearToDb(double lin) { return 20.0 * qLog10(qMax(lin, 1e-10)); }

/* ---- Smooth coefficient for attack/release ---- */

double NoiseGate10::smoothCoeff(double timeMs) const
{
    if (timeMs <= 0.0) return 1.0;
    return 1.0 - qExp(-1.0 / (timeMs * m_sampleRate / 1000.0));
}

/* ---- Envelope detection (peak) ---- */

double NoiseGate10::detectEnvelope(double sample) const
{
    return qAbs(sample);
}

/* ---- Process audio ---- */

NoiseGate10::GateResult NoiseGate10::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    GateResult result;
    int n = input.size();
    if (n == 0) return result;

    double attackCoeff = smoothCoeff(m_attack);
    double releaseCoeff = smoothCoeff(m_release);
    double holdSamples = m_hold * m_sampleRate / 1000.0;
    double threshLin = dbToLinear(m_threshold);
    double closeLin = dbToLinear(m_closeThreshold);
    int la = m_lookahead;

    result.output.resize(n);
    result.gainEnvelope.resize(n);

    for (int i = 0; i < n; ++i) {
        // Lookahead: read from future, write current to buffer
        double sample;
        if (la > 0 && i + la < n) {
            sample = input[i + la]; // Look ahead
        } else if (la > 0 && i < la) {
            // Use buffered samples from previous call
            sample = m_lookaheadBuf[m_lookaheadPos % la];
            m_lookaheadBuf[m_lookaheadPos % la] = input[i];
            m_lookaheadPos++;
            if (i + la < n) sample = input[i + la];
        } else {
            sample = input[i];
        }

        // Envelope detection
        double env = detectEnvelope(sample);

        // Hysteresis gate logic
        if (m_gateOpen) {
            if (env < closeLin) {
                m_holdCounter += 1.0;
                if (m_holdCounter > holdSamples) {
                    m_gateOpen = false;
                }
            } else {
                m_holdCounter = 0.0;
            }
        } else {
            if (env >= threshLin) {
                m_gateOpen = true;
                m_holdCounter = 0.0;
                result.openCount++;
            }
        }

        // Target gain: 1.0 if open, 0.0 if closed
        double targetGain = m_gateOpen ? 1.0 : 0.0;

        // Smooth gain transitions to avoid clicks
        if (targetGain > m_currentGain) {
            // Attack: open gate smoothly
            m_currentGain += attackCoeff * (targetGain - m_currentGain);
        } else {
            // Release: close gate smoothly
            m_currentGain += releaseCoeff * (targetGain - m_currentGain);
        }
        m_currentGain = qBound(0.0, m_currentGain, 1.0);

        // Apply gain to the *current* input sample (not lookahead)
        result.output[i] = input[i] * m_currentGain;
        result.gainEnvelope[i] = m_currentGain;
    }

    // Update lookahead buffer tail for next call
    if (la > 0) {
        for (int j = 0; j < qMin(la, n); ++j) {
            m_lookaheadBuf[j] = input[n - la + j];
        }
    }

    double elapsed = timer.elapsed();
    m_stats.lastFrameSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processDone(n, result.openCount, elapsed);
    return result;
}

/* ---- Reset ---- */

void NoiseGate10::reset()
{
    m_currentGain = 0.0;
    m_holdCounter = 0.0;
    m_gateOpen = false;
    m_lookaheadBuf.fill(0.0);
    m_lookaheadPos = 0;
}

void NoiseGate10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}
