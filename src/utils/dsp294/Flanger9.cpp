/**
 * @file Flanger9.cpp
 * @brief Flanger9 实现
 *
 * 实现镶边效果器：过零延迟与调制全通插值实现金属扫频梳状滤波反馈效果。
 */

#include "utils/dsp294/Flanger9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Flanger9::Flanger9(QObject *parent)
    : QObject(parent)
{
    resetDelayLine();
}

Flanger9::~Flanger9() = default;

/* ---- Configuration ---- */

void Flanger9::setSampleRate(double rate)
{
    m_sampleRate = qBound(8000.0, rate, 192000.0);
    resetDelayLine();
}

void Flanger9::setParameters(const Parameters& params)
{
    m_params = params;
    m_params.feedback = qBound(0.0, params.feedback, 0.99);
    m_params.mix = qBound(0.0, params.mix, 1.0);
    m_params.depth = qBound(0.0, params.depth, 0.01);
    m_params.rate = qBound(0.01, params.rate, 20.0);
    m_params.baseDelay = qBound(0.0001, params.baseDelay, 0.01);
}

/* ---- Initialize delay buffer ---- */

void Flanger9::resetDelayLine()
{
    // Max delay = baseDelay + depth, plus margin for through-zero
    double maxDelay = m_params.baseDelay + m_params.depth + 0.005;
    m_delayLength = static_cast<int>(maxDelay * m_sampleRate) + 2;
    m_delayLength = qMax(m_delayLength, 512);
    m_delayBuffer.resize(m_delayLength, 0.0);
    m_delayWritePos = 0;
    m_feedbackSample = 0.0;
    m_lfoPhase = 0.0;
}

/* ---- LFO value for given waveform ---- */

double Flanger9::lfoValue(LfoWaveform wave, double phase) const
{
    // Phase is [0, 1), output is [-1, 1]
    switch (wave) {
    case Sine:
        return qSin(2.0 * M_PI * phase);
    case Triangle:
        return phase < 0.5 ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
    case Sawtooth:
        return 2.0 * phase - 1.0;
    case Square:
        return phase < 0.5 ? 1.0 : -1.0;
    }
    return 0.0;
}

/* ---- Advance LFO ---- */

double Flanger9::tickLfo()
{
    double val = lfoValue(m_params.waveform, m_lfoPhase);
    m_lfoPhase += m_params.rate / m_sampleRate;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;
    return val;
}

/* ---- All-pass interpolation read ---- */

double Flanger9::readDelayAllpass(double delaySamples) const
{
    int intDelay = static_cast<int>(delaySamples);
    double frac = delaySamples - intDelay;

    // Wrap indices into circular buffer
    int idx0 = (m_delayWritePos - intDelay - 1 + m_delayLength) % m_delayLength;
    int idx1 = (m_delayWritePos - intDelay + m_delayLength) % m_delayLength;

    // All-pass interpolation: better than linear for modulated delays
    // y(n) = (1-frac)*x(n-intDelay-1) + frac*x(n-intDelay) + (frac-1)*y(n-1)
    // Simplified Thiran all-pass first-order
    double x0 = m_delayBuffer[idx0];
    double x1 = m_delayBuffer[idx1];

    // First-order Thiran all-pass interpolation coefficient
    double coeff = (1.0 - frac) / (1.0 + frac);
    // Simple all-pass interpolated output
    double output = x1 + coeff * (x0 - output);  // Recursive all-pass
    // Use linear interpolation as stable fallback mixed with all-pass
    double linear = x0 * (1.0 - frac) + x1 * frac;
    // Blend: primarily linear with all-pass character for modulation smoothness
    return linear + 0.3 * coeff * (x0 - linear);
}

/* ---- Through-zero delay read ---- */

double Flanger9::readThroughZero(double delaySamples, double currentInput) const
{
    if (delaySamples >= 1.0) {
        return readDelayAllpass(delaySamples);
    } else if (delaySamples > 0.0) {
        // Short positive delay: interpolate between current input and buffer
        int idx = (m_delayWritePos - 1 + m_delayLength) % m_delayLength;
        double frac = delaySamples;
        return currentInput * (1.0 - frac) + m_delayBuffer[idx] * frac;
    } else {
        // Negative delay (through-zero): mix with "future" sample (current)
        // Crossfade between current input and very short delay
        double absDelay = -delaySamples;
        return currentInput * (1.0 - absDelay);
    }
}

/* ---- Process mono ---- */

QVector<double> Flanger9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double lfo = tickLfo();
        double modDelay = m_params.baseDelay + m_params.depth * 0.5 * (1.0 + lfo);
        double delaySamples = modDelay * m_sampleRate;

        // Read delayed signal with through-zero support
        double delayed = readThroughZero(delaySamples, input[i]);

        // Apply feedback
        double feedbackSignal = delayed * m_params.feedback;
        m_feedbackSample = feedbackSignal;

        // Write to delay buffer: input + feedback
        m_delayBuffer[m_delayWritePos] = input[i] + m_feedbackSample;
        m_delayWritePos = (m_delayWritePos + 1) % m_delayLength;

        // Mix dry and wet
        output[i] = input[i] * (1.0 - m_params.mix) + delayed * m_params.mix;
    }

    double elapsed = timer.elapsed();
    m_stats.totalSamples += n;
    m_stats.blockSize = n;
    m_stats.sampleRate = m_sampleRate;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit processDone(n, elapsed);
    return output;
}

/* ---- Process stereo ---- */

QVector<double> Flanger9::processStereo(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int totalSamples = input.size();
    int numFrames = totalSamples / 2;
    QVector<double> output(totalSamples, 0.0);

    for (int i = 0; i < numFrames; ++i) {
        // Left channel
        double lfoL = tickLfo();
        double modDelayL = m_params.baseDelay + m_params.depth * 0.5 * (1.0 + lfoL);
        double delaySamplesL = modDelayL * m_sampleRate;

        double delayedL = readThroughZero(delaySamplesL, input[2 * i]);
        m_delayBuffer[m_delayWritePos] = input[2 * i] + delayedL * m_params.feedback;
        m_delayWritePos = (m_delayWritePos + 1) % m_delayLength;
        output[2 * i] = input[2 * i] * (1.0 - m_params.mix) + delayedL * m_params.mix;

        // Right channel with optional phase offset
        double lfoR = m_params.stereoPhase ? -lfoL : lfoL;
        double modDelayR = m_params.baseDelay + m_params.depth * 0.5 * (1.0 + lfoR);
        double delaySamplesR = modDelayR * m_sampleRate;

        double delayedR = readThroughZero(delaySamplesR, input[2 * i + 1]);
        m_delayBuffer[m_delayWritePos] = input[2 * i + 1] + delayedR * m_params.feedback;
        m_delayWritePos = (m_delayWritePos + 1) % m_delayLength;
        output[2 * i + 1] = input[2 * i + 1] * (1.0 - m_params.mix) + delayedR * m_params.mix;
    }

    double elapsed = timer.elapsed();
    m_stats.totalSamples += totalSamples;
    m_stats.blockSize = totalSamples;
    m_stats.sampleRate = m_sampleRate;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit processDone(totalSamples, elapsed);
    return output;
}

/* ---- Reset ---- */

void Flanger9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
