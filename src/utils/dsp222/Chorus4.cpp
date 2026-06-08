/**
 * @file Chorus4.cpp
 * @brief Chorus4 实现
 *
 * 实现合唱效果器：调制延迟合奏、相位随机化LFO银行立体声扩展。
 */

#include "utils/dsp222/Chorus4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chorus4::Chorus4(QObject *parent) : QObject(parent) {}
Chorus4::~Chorus4() = default;

/* ---- Configure ---- */

void Chorus4::setConfig(const Config& config)
{
    m_config = config;
    m_config.numVoices = qBound(1, config.numVoices, 16);
    m_config.baseDelayMs = qBound(1.0, config.baseDelayMs, 50.0);
    m_config.depthMs = qBound(0.1, config.depthMs, 20.0);
    m_config.rateHz = qBound(0.01, config.rateHz, 10.0);
    m_config.feedback = qBound(0.0, config.feedback, 0.95);
    m_config.mix = qBound(0.0, config.mix, 1.0);
    m_config.stereoSpread = qBound(0.0, config.stereoSpread, 1.0);
}

/* ---- Prepare ---- */

void Chorus4::prepare(int sampleRate, int bufferSize)
{
    m_sampleRate = qMax(1, sampleRate);
    m_stats.sampleRate = m_sampleRate;
    m_stats.bufferSize = bufferSize;

    // Allocate delay lines (max delay = base + depth with margin)
    double maxDelayMs = m_config.baseDelayMs + m_config.depthMs + 5.0;
    int maxDelaySamples = qCeil(maxDelayMs * m_sampleRate / 1000.0);

    m_delayLines.resize(m_config.numVoices);
    m_writePos.resize(m_config.numVoices, 0);

    for (int v = 0; v < m_config.numVoices; ++v) {
        m_delayLines[v].resize(maxDelaySamples, 0.0);
    }

    initLFOBank();
    m_stats.numVoices = m_config.numVoices;
}

/* ---- Initialize LFO bank ---- */

void Chorus4::initLFOBank()
{
    int n = m_config.numVoices;
    m_lfoPhase.resize(n, 0.0);
    m_lfoPhaseInc.resize(n);
    m_lfoPhaseOffset.resize(n);

    double baseInc = 2.0 * M_PI * m_config.rateHz / m_sampleRate;

    for (int v = 0; v < n; ++v) {
        // Slightly detune each voice LFO
        double detune = 1.0 + 0.05 * (v - n / 2.0) / qMax(n / 2.0, 1.0);
        m_lfoPhaseInc[v] = baseInc * detune;
        // Random phase offset for stereo spread
        m_lfoPhaseOffset[v] = (v * 2.0 * M_PI / n) +
            (qrand() % 1000) / 1000.0 * m_config.stereoSpread * M_PI;
        m_lfoPhase[v] = m_lfoPhaseOffset[v];
    }
}

/* ---- Tick LFO ---- */

double Chorus4::tickLFO(int voice)
{
    double phase = m_lfoPhase[voice];
    // Sine LFO modulated with slight triangle blend
    double mod = 0.7 * qSin(phase) + 0.3 * (2.0 * qAbs(2.0 * (phase / (2 * M_PI) -
        qFloor(phase / (2 * M_PI) + 0.5)) - 1.0);
    m_lfoPhase[voice] += m_lfoPhaseInc[voice];
    if (m_lfoPhase[voice] > 2.0 * M_PI) m_lfoPhase[voice] -= 2.0 * M_PI;
    return mod;
}

/* ---- Read from fractional delay ---- */

double Chorus4::readDelay(int voice, double fraction) const
{
    int len = m_delayLines[voice].size();
    int pos = m_writePos[voice];
    double exact = pos - fraction;
    while (exact < 0) exact += len;
    while (exact >= len) exact -= len;

    int i0 = qFloor(exact);
    int i1 = (i0 + 1) % len;
    double frac = exact - i0;

    // Linear interpolation
    return m_delayLines[voice][i0] * (1.0 - frac) + m_delayLines[voice][i1] * frac;
}

/* ---- Write to delay line ---- */

void Chorus4::writeDelay(int voice, double sample)
{
    m_delayLines[voice][m_writePos[voice]] = sample;
    m_writePos[voice] = (m_writePos[voice] + 1) % m_delayLines[voice].size();
}

/* ---- Process single sample ---- */

void Chorus4::processSample(double input, double& outL, double& outR)
{
    double dry = input * (1.0 - m_config.mix);
    double wetL = 0.0, wetR = 0.0;

    for (int v = 0; v < m_config.numVoices; ++v) {
        double mod = tickLFO(v);
        double delayMs = m_config.baseDelayMs + m_config.depthMs * mod * 0.5;
        double delaySamples = delayMs * m_sampleRate / 1000.0;

        double delayed = readDelay(v, delaySamples);

        // Feedback into delay line
        writeDelay(v, input + delayed * m_config.feedback);

        // Stereo spread: alternate voices pan left/right
        double pan = (v % 2 == 0) ? 1.0 : -1.0;
        double spread = m_config.stereoSpread;
        double gainL = (pan > 0) ? 1.0 : (1.0 - spread);
        double gainR = (pan < 0) ? 1.0 : (1.0 - spread);

        wetL += delayed * gainL / m_config.numVoices;
        wetR += delayed * gainR / m_config.numVoices;
    }

    outL = dry + wetL * m_config.mix;
    outR = dry + wetR * m_config.mix;
}

/* ---- Process buffer ---- */

QVector<double> Chorus4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(2 * n, 0.0);

    for (int i = 0; i < n; ++i) {
        double l, r;
        processSample(input[i], l, r);
        output[2 * i] = l;
        output[2 * i + 1] = r;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Chorus4::reset()
{
    for (auto& dl : m_delayLines) dl.fill(0.0);
    m_writePos.fill(0);
    for (int v = 0; v < m_lfoPhase.size(); ++v)
        m_lfoPhase[v] = m_lfoPhaseOffset[v];
}

/* ---- Reset statistics ---- */

void Chorus4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}
