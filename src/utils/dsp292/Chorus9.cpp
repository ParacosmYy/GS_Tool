/**
 * @file Chorus9.cpp
 * @brief Chorus9 实现
 *
 * 实现合唱效果器：多声部失谐合唱与随机LFO相位偏移立体声宽度调制实现丰富空间增厚。
 */

#include "utils/dsp292/Chorus9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chorus9::Chorus9(QObject *parent)
    : QObject(parent)
{
    initVoices();
}

Chorus9::~Chorus9() = default;

/* ---- Configuration ---- */

void Chorus9::setSampleRate(double sr) { m_sampleRate = qBound(8000.0, sr, 192000.0); initVoices(); }
void Chorus9::setNumVoices(int n) { m_numVoices = qBound(1, n, 16); initVoices(); }
void Chorus9::setBaseDelay(double ms) { m_baseDelay = qBound(1.0, ms, 50.0); }
void Chorus9::setDepth(double ms) { m_depth = qBound(0.0, ms, 20.0); }
void Chorus9::setRate(double hz) { m_rate = qBound(0.01, hz, 20.0); }
void Chorus9::setStereoWidth(double width) { m_stereoWidth = qBound(0.0, width, 1.0); }
void Chorus9::setMix(double mix) { m_mix = qBound(0.0, mix, 1.0); }

/* ---- Initialize voices with random phases ---- */

void Chorus9::initVoices()
{
    m_voices.resize(m_numVoices);
    m_lfoPhase.resize(m_numVoices);
    int maxDelaySamples = static_cast<int>((m_baseDelay + m_depth + 5.0) * m_sampleRate / 1000.0);
    maxDelaySamples = qMax(maxDelaySamples, 512);
    m_delayBuffer.resize(m_numVoices);
    m_writePos.resize(m_numVoices, 0);

    for (int i = 0; i < m_numVoices; ++i) {
        // Distribute voices with slight parameter variations
        double voiceRatio = static_cast<double>(i) / m_numVoices;
        m_voices[i].delayMs = m_baseDelay + voiceRatio * 2.0;
        m_voices[i].depth = m_depth * (0.7 + 0.6 * voiceRatio);
        m_voices[i].rate = m_rate * (0.8 + 0.4 * voiceRatio);
        // Randomized LFO phase offsets for spatial thickening
        m_voices[i].phaseOffset = (static_cast<double>(qrand()) / RAND_MAX) * 2.0 * M_PI;
        // Distribute pan across stereo field
        m_voices[i].pan = -1.0 + 2.0 * voiceRatio;
        m_voices[i].gain = 1.0 / qSqrt(m_numVoices);
        m_lfoPhase[i] = m_voices[i].phaseOffset;
        m_delayBuffer[i].resize(maxDelaySamples, 0.0);
    }
}

/* ---- Compute modulated delay in samples ---- */

double Chorus9::modulatedDelay(int voiceIdx) const
{
    double mod = qSin(m_lfoPhase[voiceIdx]) * m_voices[voiceIdx].depth;
    return (m_voices[voiceIdx].delayMs + mod) * m_sampleRate / 1000.0;
}

/* ---- Read from delay buffer with linear interpolation ---- */

double Chorus9::readBuffer(int voiceIdx, double delaySamples) const
{
    const auto& buf = m_delayBuffer[voiceIdx];
    int bufLen = buf.size();
    int wPos = m_writePos[voiceIdx];

    double exactPos = wPos - delaySamples;
    while (exactPos < 0) exactPos += bufLen;
    while (exactPos >= bufLen) exactPos -= bufLen;

    int idx0 = static_cast<int>(exactPos);
    int idx1 = (idx0 + 1) % bufLen;
    double frac = exactPos - idx0;

    return buf[idx0] * (1.0 - frac) + buf[idx1] * frac;
}

/* ---- Write to delay buffer ---- */

void Chorus9::writeBuffer(int voiceIdx, double sample)
{
    int bufLen = m_delayBuffer[voiceIdx].size();
    m_delayBuffer[voiceIdx][m_writePos[voiceIdx]] = sample;
    m_writePos[voiceIdx] = (m_writePos[voiceIdx] + 1) % bufLen;
}

/* ---- Process mono input ---- */

Chorus9::ChorusResult Chorus9::process(const QVector<double>& input)
{
    return processStereo(input, input);
}

/* ---- Process stereo input ---- */

Chorus9::ChorusResult Chorus9::processStereo(const QVector<double>& leftIn,
                                                const QVector<double>& rightIn)
{
    QElapsedTimer timer;
    timer.start();

    ChorusResult result;
    int n = qMin(leftIn.size(), rightIn.size());
    result.leftOut.resize(n);
    result.rightOut.resize(n);

    double lfoStep = 2.0 * M_PI * m_rate / m_sampleRate;
    double peak = 0.0;

    for (int s = 0; s < n; ++s) {
        double dryL = leftIn[s];
        double dryR = rightIn[s];
        double mono = (dryL + dryR) * 0.5;

        double wetL = 0.0, wetR = 0.0;

        for (int v = 0; v < m_numVoices; ++v) {
            // Write input to delay buffer
            writeBuffer(v, mono);

            // Read with modulated delay
            double delay = modulatedDelay(v);
            double delayed = readBuffer(v, delay);

            // Apply pan with stereo width modulation
            double pan = m_voices[v].pan * m_stereoWidth;
            double panL = qSqrt(qMax(0.0, (1.0 - pan) * 0.5));
            double panR = qSqrt(qMax(0.0, (1.0 + pan) * 0.5));
            double gain = m_voices[v].gain;

            wetL += delayed * panL * gain;
            wetR += delayed * panR * gain;

            // Advance LFO phase
            m_lfoPhase[v] += 2.0 * M_PI * m_voices[v].rate / m_sampleRate;
            if (m_lfoPhase[v] > 2.0 * M_PI)
                m_lfoPhase[v] -= 2.0 * M_PI;
        }

        // Dry/wet mix
        result.leftOut[s] = dryL * (1.0 - m_mix) + wetL * m_mix;
        result.rightOut[s] = dryR * (1.0 - m_mix) + wetR * m_mix;

        double absL = qAbs(result.leftOut[s]);
        double absR = qAbs(result.rightOut[s]);
        peak = qMax(peak, qMax(absL, absR));
    }

    result.peakLevel = peak;

    double elapsed = timer.elapsed();
    m_stats.totalSamples += n;
    m_stats.numVoices = m_numVoices;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit processDone(n, peak, elapsed);
    return result;
}

/* ---- Reset ---- */

void Chorus9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    initVoices();
}
