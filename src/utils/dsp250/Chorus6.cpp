/**
 * @file Chorus6.cpp
 * @brief Chorus6 实现
 *
 * 实现合唱效果器：颤音调制变延迟与立体声交叉混音反馈。
 */

#include "utils/dsp250/Chorus6.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- DelayLine implementation ---- */

void Chorus6::DelayLine::init(int size)
{
    buffer.resize(size, 0.0);
    writePos = 0;
}

void Chorus6::DelayLine::write(double sample)
{
    if (buffer.isEmpty()) return;
    buffer[writePos] = sample;
}

double Chorus6::DelayLine::readFrac(double delaySamples) const
{
    if (buffer.isEmpty()) return 0.0;
    int n = buffer.size();
    double exactPos = writePos - delaySamples;
    while (exactPos < 0) exactPos += n;
    while (exactPos >= n) exactPos -= n;

    int idx0 = static_cast<int>(exactPos) % n;
    int idx1 = (idx0 + 1) % n;
    double frac = exactPos - static_cast<int>(exactPos);

    return buffer[idx0] * (1.0 - frac) + buffer[idx1] * frac;
}

void Chorus6::DelayLine::advance()
{
    if (buffer.isEmpty()) return;
    writePos = (writePos + 1) % buffer.size();
}

/* ---- Construction / Destruction ---- */

Chorus6::Chorus6(QObject *parent) : QObject(parent) { initDelayLines(); }
Chorus6::~Chorus6() = default;

/* ---- Configuration ---- */

void Chorus6::setSampleRate(int rate)
{
    m_sampleRate = qMax(8000, rate);
    initDelayLines();
}

void Chorus6::setParameters(double baseDelayMs, double depthMs,
                              double rateHz, double feedback, double mix)
{
    m_baseDelayMs = qBound(1.0, baseDelayMs, 50.0);
    m_depthMs = qBound(0.1, depthMs, 20.0);
    m_rateHz = qBound(0.05, rateHz, 10.0);
    m_feedback = qBound(0.0, feedback, 0.95);
    m_mix = qBound(0.0, mix, 1.0);
}

void Chorus6::setNumVoices(int voices)
{
    m_numVoices = qBound(1, voices, 8);
    initDelayLines();
}

/* ---- Initialize delay lines ---- */

void Chorus6::initDelayLines()
{
    m_delayLinesL.resize(m_numVoices);
    m_delayLinesR.resize(m_numVoices);
    m_lfoPhase.resize(m_numVoices);

    for (int v = 0; v < m_numVoices; ++v) {
        m_delayLinesL[v].init(MAX_DELAY_SAMPLES);
        m_delayLinesR[v].init(MAX_DELAY_SAMPLES);
        // Spread voices across phase offsets
        m_lfoPhase[v] = (2.0 * M_PI * v) / m_numVoices;
    }
    m_crossL = 0.0;
    m_crossR = 0.0;
}

/* ---- LFO for given voice ---- */

double Chorus6::lfo(int voice) const
{
    // Sine LFO with per-voice phase offset and slight detune
    double detune = 1.0 + 0.05 * (voice - m_numVoices / 2.0);
    return qSin(m_lfoPhase[voice] * detune);
}

/* ---- Process mono to stereo ---- */

QVector<double> Chorus6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n * 2, 0.0);
    double phaseInc = 2.0 * M_PI * m_rateHz / m_sampleRate;

    double baseDelaySamples = m_baseDelayMs * m_sampleRate / 1000.0;
    double depthSamples = m_depthMs * m_sampleRate / 1000.0;

    for (int i = 0; i < n; ++i) {
        double dryL = input[i];
        double dryR = input[i];

        // Add cross-feedback from previous output
        double wetL = m_crossL * m_feedback;
        double wetR = m_crossR * m_feedback;

        for (int v = 0; v < m_numVoices; ++v) {
            // Modulated delay: baseDelay + depth * lfo
            double modDelay = baseDelaySamples + depthSamples * lfo(v);

            // Voice panning: spread voices across stereo field
            double pan = static_cast<double>(v) / qMax(1, m_numVoices - 1);
            double panL = qCos(pan * M_PI * 0.5);
            double panR = qSin(pan * M_PI * 0.5);

            double delayedL = m_delayLinesL[v].readFrac(modDelay);
            double delayedR = m_delayLinesR[v].readFrac(modDelay);

            wetL += delayedL * panL / m_numVoices;
            wetR += delayedR * panR / m_numVoices;

            // Write input + feedback into delay lines
            m_delayLinesL[v].write(dryL + m_crossL * m_feedback);
            m_delayLinesR[v].write(dryR + m_crossR * m_feedback);
            m_delayLinesL[v].advance();
            m_delayLinesR[v].advance();
        }

        // Advance LFO phases
        for (int v = 0; v < m_numVoices; ++v)
            m_lfoPhase[v] += phaseInc;

        // Stereo cross-mixing: swap and blend
        m_crossL = wetL * 0.7 + wetR * 0.3;
        m_crossR = wetR * 0.7 + wetL * 0.3;

        // Output: dry/wet mix
        output[i * 2] = dryL * (1.0 - m_mix) + m_crossL * m_mix;
        output[i * 2 + 1] = dryR * (1.0 - m_mix) + m_crossR * m_mix;
    }

    m_stats.numSamplesProcessed += n;
    m_stats.sampleRate = m_sampleRate;
    m_stats.numVoices = m_numVoices;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset delay lines ---- */

void Chorus6::reset()
{
    initDelayLines();
}

/* ---- Reset statistics ---- */

void Chorus6::resetStatistics()
{
    initDelayLines();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
