/**
 * @file Delay10.cpp
 * @brief Delay10 实现
 *
 * 实现延迟效果：节拍同步调制与乒乓立体声路由实现节奏回声与空间延迟效果。
 */

#include "utils/dsp296/Delay10.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Delay10::Delay10(QObject *parent)
    : QObject(parent)
{
    initBuffers();
}

Delay10::~Delay10() = default;

/* ---- Configuration ---- */

void Delay10::setTempo(double bpm) { m_tempoBpm = qBound(20.0, bpm, 300.0); }
void Delay10::setDelayTime(double s) { m_delayTime = qBound(0.01, s, 5.0); initBuffers(); }
void Delay10::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Delay10::setMix(double wet) { m_mix = qBound(0.0, wet, 1.0); }
void Delay10::setPingPong(bool enabled) { m_pingPong = enabled; }

/* ---- Initialize delay buffers ---- */

void Delay10::initBuffers()
{
    // Allocate buffer for max delay with modulation headroom
    int maxSamples = static_cast<int>(44100.0 * (m_delayTime + 0.05)) + 1;
    m_bufferLen = qMax(256, maxSamples);
    m_bufferL.resize(m_bufferLen, 0.0);
    m_bufferR.resize(m_bufferLen, 0.0);
    m_writePos = 0;
    m_stats.bufferSize = m_bufferLen;
    m_stats.tempoBpm = m_tempoBpm;
}

/* ---- Fractional delay read with linear interpolation ---- */

double Delay10::readBuffer(const QVector<double>& buf, double delaySamples) const
{
    int intDelay = static_cast<int>(delaySamples);
    double frac = delaySamples - intDelay;

    int idx0 = m_writePos - intDelay;
    int idx1 = idx0 - 1;

    // Wrap around circular buffer
    if (idx0 < 0) idx0 += m_bufferLen;
    if (idx1 < 0) idx1 += m_bufferLen;

    double s0 = buf[idx0];
    double s1 = buf[idx1];
    return s0 + frac * (s1 - s0);
}

/* ---- LFO modulation for tempo-synced delay variation ---- */

double Delay10::lfoModulation()
{
    // LFO synced to tempo: quarter-note subdivision
    double lfoFreq = m_tempoBpm / 60.0 * 0.25;
    double mod = qSin(2.0 * M_PI * m_lfoPhase) * 0.002 * 44100.0;
    m_lfoPhase += lfoFreq / 44100.0;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;
    return mod;
}

/* ---- Simple one-pole lowpass for feedback damping ---- */

double Delay10::lowpass(double input, double& state) const
{
    double coeff = 0.6;
    state = state + coeff * (input - state);
    return state;
}

/* ---- Process interleaved stereo samples ---- */

QVector<double> Delay10::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    // Convert to frames
    int numFrames = n / 2;
    QVector<StereoFrame> frames(numFrames);
    for (int i = 0; i < numFrames; ++i) {
        frames[i].left = input[i * 2];
        frames[i].right = input[i * 2 + 1];
    }

    auto processed = processFrames(frames);

    // Interleave back
    QVector<double> output(n, 0.0);
    for (int i = 0; i < numFrames; ++i) {
        output[i * 2] = processed[i].left;
        output[i * 2 + 1] = processed[i].right;
    }

    double elapsed = timer.elapsed();
    m_stats.totalProcess++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcess;

    emit processDone(numFrames, elapsed);
    return output;
}

/* ---- Process stereo frames ---- */

QVector<Delay10::StereoFrame> Delay10::processFrames(const QVector<StereoFrame>& input)
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = input.size();
    QVector<StereoFrame> output(numFrames);

    double delaySamples = m_delayTime * 44100.0;
    double lpStateL = 0.0;
    double lpStateR = 0.0;

    for (int i = 0; i < numFrames; ++i) {
        // Tempo-synced LFO modulation
        double modOffset = lfoModulation();
        double effectiveDelay = qMax(1.0, delaySamples + modOffset);

        // Read delayed signals from buffers
        double delayedL = readBuffer(m_bufferL, effectiveDelay);
        double delayedR = readBuffer(m_bufferR, effectiveDelay);

        // Ping-pong routing: L output feeds R input and vice versa
        double feedbackL, feedbackR;
        if (m_pingPong) {
            // Cross-channel feedback
            feedbackL = lowpass(delayedR * m_feedback, lpStateL);
            feedbackR = lowpass(delayedL * m_feedback, lpStateR);
        } else {
            feedbackL = lowpass(delayedL * m_feedback, lpStateL);
            feedbackR = lowpass(delayedR * m_feedback, lpStateR);
        }

        // Write input + feedback to buffers
        m_bufferL[m_writePos] = input[i].left + feedbackL;
        m_bufferR[m_writePos] = input[i].right + feedbackR;

        // Dry/wet mix
        output[i].left = input[i].left * (1.0 - m_mix) + delayedL * m_mix;
        output[i].right = input[i].right * (1.0 - m_mix) + delayedR * m_mix;

        // Advance write position
        m_writePos = (m_writePos + 1) % m_bufferLen;
    }

    double elapsed = timer.elapsed();
    m_stats.totalProcess++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcess;
    m_stats.bufferSize = m_bufferLen;
    m_stats.tempoBpm = m_tempoBpm;

    emit processDone(numFrames, elapsed);
    return output;
}

/* ---- Reset ---- */

void Delay10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bufferL.fill(0.0);
    m_bufferR.fill(0.0);
    m_writePos = 0;
    m_lfoPhase = 0.0;
}
