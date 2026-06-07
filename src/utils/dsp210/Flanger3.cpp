/**
 * @file Flanger3.cpp
 * @brief Flanger3 实现
 *
 * 实现镶边效果器：可变深度梳状滤波、过零镶边、反相信号混合。
 */

#include "utils/dsp210/Flanger3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Flanger3::Flanger3(QObject *parent) : QObject(parent) {}
Flanger3::~Flanger3() = default;

/* ---- Configuration ---- */

void Flanger3::setSampleRate(int rate) { m_sampleRate = qMax(8000, rate); }
void Flanger3::setBaseDelayMs(double ms) { m_baseDelayMs = qMax(0.1, ms); }
void Flanger3::setDepthMs(double ms) { m_depthMs = qMax(0.0, ms); }
void Flanger3::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.99); }
void Flanger3::setMix(double mix) { m_mix = qBound(0.0, mix, 1.0); }
void Flanger3::setLfoRate(double hz) { m_lfoRate = qBound(0.01, hz, 20.0); }
void Flanger3::setLfoType(LfoType type) { m_lfoType = type; }
void Flanger3::setInvertPhase(bool invert) { m_invertPhase = invert; }

/* ---- Initialize ---- */

void Flanger3::initialize(int bufferSize)
{
    // Max delay = base + depth in samples (with margin)
    double maxDelayMs = m_baseDelayMs + m_depthMs + 5.0;
    m_bufferSize = qMax(1024, qCeil(maxDelayMs * m_sampleRate / 1000.0));
    m_bufferSize = qMax(m_bufferSize, bufferSize);
    m_delayBuffer.resize(m_bufferSize, 0.0);
    m_writeIdx = 0;
    m_lfoPhase = 0.0;

    m_stats.bufferSize = m_bufferSize;
    m_stats.sampleRate = m_sampleRate;
}

/* ---- Compute LFO ---- */

double Flanger3::computeLfo() const
{
    double phase = m_lfoPhase;
    switch (m_lfoType) {
    case Sine:     return qSin(2.0 * M_PI * phase);
    case Triangle: return 1.0 - 4.0 * qAbs(phase - 0.5);
    case Sawtooth: return 2.0 * phase - 1.0;
    case Square:   return (phase < 0.5) ? 1.0 : -1.0;
    default:       return qSin(2.0 * M_PI * phase);
    }
}

/* ---- Fractional delay read ---- */

double Flanger3::readDelay(double delaySamples) const
{
    if (m_delayBuffer.isEmpty()) return 0.0;
    delaySamples = qBound(0.0, delaySamples, m_bufferSize - 1.0);
    int idx0 = m_writeIdx - static_cast<int>(qCeil(delaySamples));
    int idx1 = idx0 + 1;

    // Wrap indices
    while (idx0 < 0) idx0 += m_bufferSize;
    while (idx1 < 0) idx1 += m_bufferSize;
    idx0 %= m_bufferSize;
    idx1 %= m_bufferSize;

    // Linear interpolation
    double frac = delaySamples - qFloor(delaySamples);
    return m_delayBuffer[idx0] * (1.0 - frac) + m_delayBuffer[idx1] * frac;
}

/* ---- Advance LFO ---- */

void Flanger3::advanceLfo()
{
    m_lfoPhase += m_lfoRate / m_sampleRate;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;
}

/* ---- Process single sample ---- */

double Flanger3::processSample(double input)
{
    if (m_delayBuffer.isEmpty()) initialize(4096);

    // Write input to delay buffer
    m_delayBuffer[m_writeIdx] = input;

    // Compute modulated delay
    double lfoVal = computeLfo();
    double delayMs = m_baseDelayMs + m_depthMs * 0.5 * (1.0 + lfoVal);
    double delaySamples = delayMs * m_sampleRate / 1000.0;

    // Through-zero: if delay < 1 sample, blend inverted signal
    double delayed = readDelay(delaySamples);

    // Inverted signal blend for through-zero flanging
    double wet = delayed;
    if (m_invertPhase) wet = -wet;

    // Feedback path
    double feedbackSample = wet * m_feedback;
    m_delayBuffer[m_writeIdx] = input + feedbackSample;

    // Output: dry + wet mix
    double output = input * (1.0 - m_mix) + wet * m_mix;

    // Advance write pointer and LFO
    m_writeIdx = (m_writeIdx + 1) % m_bufferSize;
    advanceLfo();

    return output;
}

/* ---- Process block ---- */

QVector<double> Flanger3::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processSample(input[i]);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(input.size(), timer.elapsed());

    return output;
}

/* ---- Reset ---- */

void Flanger3::reset()
{
    std::fill(m_delayBuffer.begin(), m_delayBuffer.end(), 0.0);
    m_writeIdx = 0;
    m_lfoPhase = 0.0;
}

/* ---- Current LFO value ---- */

double Flanger3::currentLfoValue() const { return computeLfo(); }

/* ---- Reset statistics ---- */

void Flanger3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
