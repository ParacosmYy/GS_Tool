/**
 * @file Chorus2.cpp
 * @brief Chorus2 实现
 *
 * 实现合唱效果器：多声部LFO调制、立体声展宽、颤音深度控制。
 */

#include "utils/dsp191/Chorus2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chorus2::Chorus2(QObject *parent) : QObject(parent)
{
    initDefaultVoices();
    // Allocate delay buffer (~50ms at 44100 Hz)
    int bufSize = static_cast<int>(m_sampleRate * 0.05) + 1;
    m_delayBuffer.resize(bufSize, 0.0);
}

Chorus2::~Chorus2() = default;

/* ---- Configuration ---- */

void Chorus2::setSampleRate(int sr)
{
    m_sampleRate = qMax(8000, sr);
    int bufSize = static_cast<int>(m_sampleRate * 0.05) + 1;
    m_delayBuffer.resize(bufSize, 0.0);
}

void Chorus2::setBaseDelay(double ms) { m_baseDelay = qBound(1.0, ms, 30.0); }
void Chorus2::setVibratoDepth(double d) { m_vibratoDepth = qBound(0.0, d, 10.0); }
void Chorus2::setStereoSpread(double s) { m_stereoSpread = qBound(0.0, s, 1.0); }
void Chorus2::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Chorus2::setMix(double wet) { m_wet = qBound(0.0, wet, 1.0); }
void Chorus2::setVoices(const QVector<VoiceConfig>& v) { m_voices = v; }

/* ---- Initialize default voices ---- */

void Chorus2::initDefaultVoices()
{
    m_voices = {
        {7.0, 1.0, 0.5, 0.0, -0.5},
        {9.0, 0.8, 0.6, 2.094, 0.5},
        {11.0, 0.6, 0.4, 4.189, 0.0},
        {5.0, 0.9, 0.7, 1.047, -0.3}
    };
}

/* ---- LFO: sine oscillator ---- */

double Chorus2::lfo(double phase, double rate, int sample) const
{
    double t = sample / static_cast<double>(m_sampleRate);
    return qSin(2.0 * M_PI * rate * t + phase);
}

/* ---- Fractional delay read via linear interpolation ---- */

double Chorus2::fractionalRead(double delaySamples) const
{
    int bufSize = m_delayBuffer.size();
    double readPos = m_writePos - delaySamples;
    while (readPos < 0) readPos += bufSize;
    while (readPos >= bufSize) readPos -= bufSize;

    int idx0 = static_cast<int>(readPos);
    int idx1 = (idx0 + 1) % bufSize;
    double frac = readPos - idx0;

    return m_delayBuffer[idx0] * (1.0 - frac) + m_delayBuffer[idx1] * frac;
}

/* ---- Process mono -> stereo ---- */

QPair<QVector<double>, QVector<double>> Chorus2::process(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    QVector<double> left(N, 0.0);
    QVector<double> right(N, 0.0);
    int bufSize = m_delayBuffer.size();

    for (int i = 0; i < N; ++i) {
        // Write input + feedback into delay buffer
        double feedbackSample = 0.0;
        if (m_writePos > 0)
            feedbackSample = m_delayBuffer[(m_writePos - 1 + bufSize) % bufSize];
        m_delayBuffer[m_writePos] = input[i] + m_feedback * feedbackSample;

        double dry = input[i];
        double wetL = 0.0;
        double wetR = 0.0;

        for (int v = 0; v < m_voices.size(); ++v) {
            const auto& voice = m_voices[v];

            // Compute modulated delay
            double baseSamps = voice.delayMs * m_sampleRate / 1000.0;
            double mod = lfo(voice.phase, voice.rate, i)
                         * m_vibratoDepth * voice.depth;
            double delay = baseSamps + mod * m_sampleRate / 1000.0;
            delay = qMax(1.0, delay);

            double delayed = fractionalRead(delay);

            // Stereo panning: pan in [-1, 1] -> [left, right]
            double pan = voice.pan * m_stereoSpread;
            double gainL = qSqrt(qMax(0.0, (1.0 - pan) / 2.0));
            double gainR = qSqrt(qMax(0.0, (1.0 + pan) / 2.0));

            wetL += delayed * gainL;
            wetR += delayed * gainR;
        }

        // Normalize by voice count
        double norm = 1.0 / m_voices.size();
        left[i] = dry * (1.0 - m_wet) + wetL * norm * m_wet;
        right[i] = dry * (1.0 - m_wet) + wetR * norm * m_wet;

        m_writePos = (m_writePos + 1) % bufSize;
    }

    m_stats.totalFrames += N;
    m_stats.numVoices = m_voices.size();
    m_stats.sampleRate = m_sampleRate;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1ULL, m_stats.totalFrames / qMax(1, m_sampleRate));

    emit processingCompleted(N, m_voices.size(), timer.elapsed());
    return {left, right};
}

/* ---- Process mono -> mono ---- */

QVector<double> Chorus2::processMono(const QVector<double>& input)
{
    auto [left, right] = process(input);

    // Downmix stereo to mono
    QVector<double> mono(input.size());
    for (int i = 0; i < input.size(); ++i)
        mono[i] = (left[i] + right[i]) * 0.5;
    return mono;
}

/* ---- Reset ---- */

void Chorus2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_delayBuffer.fill(0.0);
    m_writePos = 0;
}
