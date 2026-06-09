/**
 * @file Delay7.cpp
 * @brief Delay7 实现
 *
 * 实现延迟效果：节拍同步多抽头+乒乓立体声路由+每抽头滤波反馈控制。
 */

#include "utils/dsp254/Delay7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Delay7::Delay7(int sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(sampleRate)
{
    m_stats.sampleRate = sampleRate;
}
Delay7::~Delay7() = default;

/* ---- Configuration ---- */

void Delay7::setTempo(double bpm)
{
    m_tempoBpm = qMax(20.0, bpm);
    m_stats.tempoBpm = m_tempoBpm;
}

void Delay7::setTaps(const QVector<TapConfig>& taps)
{
    m_taps = taps;
    m_stats.numTaps = taps.size();

    // Allocate delay buffers for each tap
    m_delayBuffers.resize(taps.size());
    m_writePos.resize(taps.size(), 0);
    m_filterState.resize(taps.size(), 0.0);

    for (int t = 0; t < taps.size(); ++t) {
        int delaySamples = beatsToSamples(taps[t].delayBeats);
        m_delayBuffers[t].resize(qMax(delaySamples, 1), 0.0);
        m_writePos[t] = 0;
        m_filterState[t] = 0.0;
    }
}

void Delay7::setPingPong(bool enabled)
{
    m_pingPong = enabled;
}

/* ---- Convert beats to samples ---- */

int Delay7::beatsToSamples(double beats) const
{
    double secPerBeat = 60.0 / m_tempoBpm;
    return qMax(1, static_cast<int>(beats * secPerBeat * m_sampleRate));
}

/* ---- One-pole low-pass filter ---- */

double Delay7::applyFilter(double input, double cutoff, double& state) const
{
    // cutoff in [0,1], map to filter coefficient
    double coeff = qBound(0.001, cutoff, 0.999);
    state = state + coeff * (input - state);
    return state;
}

/* ---- Process single sample ---- */

QVector<double> Delay7::processSample(double sample)
{
    double left = 0.0;
    double right = 0.0;

    for (int t = 0; t < m_taps.size(); ++t) {
        int bufSize = m_delayBuffers[t].size();
        int readPos = (m_writePos[t] - qMax(1, beatsToSamples(m_taps[t].delayBeats))
                       + bufSize) % bufSize;

        // Read delayed sample with feedback
        double delayed = m_delayBuffers[t][readPos];

        // Apply per-tap low-pass filter
        double filtered = applyFilter(delayed, m_taps[t].filterCutoff,
                                      m_filterState[t]);

        // Write input + feedback to buffer
        m_delayBuffers[t][m_writePos[t]] = sample + filtered * m_taps[t].feedback;
        m_writePos[t] = (m_writePos[t] + 1) % bufSize;

        // Pan: -1(left) to +1(right) -> stereo gain
        double pan = m_taps[t].pan;
        double gain = m_taps[t].gain;
        double panAngle = (pan + 1.0) * 0.25 * M_PI;
        double leftGain = qCos(panAngle) * gain;
        double rightGain = qSin(panAngle) * gain;

        if (m_pingPong) {
            // Ping-pong: alternate taps between L and R
            if (t % 2 == 0) {
                left += filtered * gain;
            } else {
                right += filtered * gain;
            }
        } else {
            left += filtered * leftGain;
            right += filtered * rightGain;
        }
    }

    return {left, right};
}

/* ---- Process block ---- */

QVector<QVector<double>> Delay7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> leftCh(n, 0.0);
    QVector<double> rightCh(n, 0.0);

    for (int i = 0; i < n; ++i) {
        QVector<double> stereo = processSample(input[i]);
        leftCh[i] = stereo[0];
        rightCh[i] = stereo[1];
    }

    m_stats.numFramesProcessed += n;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, elapsed);
    return {leftCh, rightCh};
}

/* ---- Reset ---- */

void Delay7::resetStatistics()
{
    for (auto& buf : m_delayBuffers) buf.fill(0.0);
    m_filterState.fill(0.0);
    m_writePos.fill(0);
    m_stats = Stats{};
    m_stats.sampleRate = m_sampleRate;
    m_timeSum = 0.0;
}
