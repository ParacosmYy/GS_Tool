/**
 * @file Chorus8.cpp
 * @brief Chorus8 实现
 *
 * 实现合唱效果器：多声部微调与随机相位初始化的厚重立体声合唱纹理生成。
 */

#include "utils/dsp278/Chorus8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chorus8::Chorus8(QObject *parent)
    : QObject(parent)
{
    initVoices();
    resizeDelayLine(m_baseDelay + m_depth + 5.0);
}

Chorus8::~Chorus8() = default;

/* ---- Configuration ---- */

void Chorus8::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); resizeDelayLine(m_baseDelay + m_depth + 5.0); }
void Chorus8::setBaseDelay(double ms) { m_baseDelay = qBound(1.0, ms, 50.0); resizeDelayLine(m_baseDelay + m_depth + 5.0); }
void Chorus8::setDepth(double ms) { m_depth = qBound(0.1, ms, 20.0); resizeDelayLine(m_baseDelay + m_depth + 5.0); }
void Chorus8::setRate(double hz) { m_rate = qBound(0.05, hz, 10.0); initVoices(); }
void Chorus8::setNumVoices(int n) { m_numVoices = qBound(2, n, 12); initVoices(); }
void Chorus8::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Chorus8::setMix(double mix) { m_mix = qBound(0.0, mix, 1.0); }

/* ---- Initialize voice configurations ---- */

void Chorus8::initVoices()
{
    m_voices.resize(m_numVoices);
    for (int i = 0; i < m_numVoices; ++i) {
        // Spread voices across different rates and pans
        double voiceRatio = static_cast<double>(i) / m_numVoices;
        m_voices[i].rate = m_rate * (0.8 + 0.4 * voiceRatio);
        m_voices[i].depth = m_depth * (0.7 + 0.6 * voiceRatio);
        m_voices[i].delay = m_baseDelay * (0.9 + 0.2 * voiceRatio);
        m_voices[i].pan = -1.0 + 2.0 * voiceRatio; // Spread L to R
        m_voices[i].gain = 0.7 / qSqrt(static_cast<double>(m_numVoices));
        // Random phase initialization for thick texture
        m_voices[i].phase = static_cast<double>(qrand()) / RAND_MAX * 2.0 * M_PI;
    }
}

/* ---- Resize delay line ---- */

void Chorus8::resizeDelayLine(double maxDelayMs)
{
    int needed = static_cast<int>(maxDelayMs * m_sampleRate / 1000.0) + 2;
    if (needed > m_delayLineSize) {
        m_delayLineSize = needed;
        m_delayBuffer.resize(needed, 0.0);
        m_writePos = 0;
    }
}

/* ---- Fractional delay read (linear interpolation) ---- */

double Chorus8::readFractional(double position) const
{
    int posInt = static_cast<int>(position);
    double frac = position - posInt;

    int idx0 = (m_writePos - posInt - 1 + m_delayLineSize) % m_delayLineSize;
    int idx1 = (m_writePos - posInt - 2 + m_delayLineSize) % m_delayLineSize;

    return m_delayBuffer[idx0] * (1.0 - frac) + m_delayBuffer[idx1] * frac;
}

/* ---- Compute modulated delay for a voice ---- */

double Chorus8::computeModulatedDelay(int voiceIdx, int sampleIdx) const
{
    const VoiceConfig& v = m_voices[voiceIdx];
    double phase = v.phase + 2.0 * M_PI * v.rate * sampleIdx / m_sampleRate;
    double modulation = qSin(phase);
    double delayMs = v.delay + v.depth * modulation;
    return delayMs * m_sampleRate / 1000.0;
}

/* ---- Process mono -> stereo ---- */

Chorus8::ChorusResult Chorus8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ChorusResult result;
    int n = input.size();
    result.outputLeft.resize(n, 0.0);
    result.outputRight.resize(n, 0.0);

    double peakL = 0.0, peakR = 0.0;
    double sumSqL = 0.0, sumSqR = 0.0;

    for (int i = 0; i < n; ++i) {
        double sample = input[i];

        // Write to delay line with feedback
        double feedbackSample = sample;
        if (m_feedback > 0.0) {
            int readBack = (m_writePos - static_cast<int>(m_baseDelay * m_sampleRate / 1000.0)
                            - 1 + m_delayLineSize) % m_delayLineSize;
            feedbackSample += m_delayBuffer[readBack] * m_feedback;
        }
        m_delayBuffer[m_writePos] = feedbackSample;

        // Sum voices with pan
        double wetL = 0.0, wetR = 0.0;
        for (int v = 0; v < m_numVoices; ++v) {
            double delaySamples = computeModulatedDelay(v, i);
            double voiceOut = readFractional(delaySamples) * m_voices[v].gain;

            // Pan: cos/sin law
            double panAngle = (m_voices[v].pan + 1.0) * 0.25 * M_PI;
            wetL += voiceOut * qCos(panAngle);
            wetR += voiceOut * qSin(panAngle);
        }

        // Dry/wet mix
        double dry = sample * (1.0 - m_mix);
        result.outputLeft[i] = dry + wetL * m_mix;
        result.outputRight[i] = dry + wetR * m_mix;

        // Track levels
        double absL = qAbs(result.outputLeft[i]);
        double absR = qAbs(result.outputRight[i]);
        peakL = qMax(peakL, absL);
        peakR = qMax(peakR, absR);
        sumSqL += result.outputLeft[i] * result.outputLeft[i];
        sumSqR += result.outputRight[i] * result.outputRight[i];

        // Advance write position
        m_writePos = (m_writePos + 1) % m_delayLineSize;
    }

    result.peakLevel = qMax(peakL, peakR);
    result.rmsLevel = qSqrt((sumSqL + sumSqR) / (2.0 * n));

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.numVoices = m_numVoices;
    m_stats.sampleRate = m_sampleRate;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(n, m_numVoices, elapsed);

    return result;
}

/* ---- Process stereo -> stereo ---- */

Chorus8::ChorusResult Chorus8::processStereo(
    const QVector<double>& inputLeft,
    const QVector<double>& inputRight)
{
    int n = qMin(inputLeft.size(), inputRight.size());

    // Mix to mono for delay line input
    QVector<double> mono(n);
    for (int i = 0; i < n; ++i)
        mono[i] = (inputLeft[i] + inputRight[i]) * 0.5;

    return process(mono);
}

/* ---- Reset ---- */

void Chorus8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_delayBuffer.fill(0.0);
    m_writePos = 0;
}
