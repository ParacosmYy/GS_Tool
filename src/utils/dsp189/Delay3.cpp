/**
 * @file Delay3.cpp
 * @brief Delay3 实现
 *
 * 实现音频延迟线：节拍同步、乒乓立体声、反馈滤波、抽头调制、线性插值。
 */

#include "utils/dsp189/Delay3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Delay3::Delay3(QObject *parent) : QObject(parent)
{
    // Allocate max delay buffer (2 seconds)
    int maxSamples = static_cast<int>(m_sampleRate * 2.0);
    m_bufferL.resize(maxSamples, 0.0);
    m_bufferR.resize(maxSamples, 0.0);
    updateDelaySamples();
}

Delay3::~Delay3() = default;

/* ---- Configuration ---- */

void Delay3::setSampleRate(double sr)
{
    m_sampleRate = qMax(8000.0, sr);
    int maxSamples = static_cast<int>(m_sampleRate * 2.0);
    m_bufferL.resize(maxSamples, 0.0);
    m_bufferR.resize(maxSamples, 0.0);
    m_writePos = 0;
    updateDelaySamples();
}

void Delay3::setDelayTimeMs(double ms) { m_delayMs = qMax(1.0, ms); updateDelaySamples(); }
void Delay3::setBpm(double bpm) { m_bpm = qMax(20.0, qMin(300.0, bpm)); updateDelaySamples(); }
void Delay3::setTempoSync(bool sync) { m_tempoSync = sync; updateDelaySamples(); }
void Delay3::setFeedback(double gain) { m_feedback = qBound(0.0, gain, 0.95); }
void Delay3::setWetMix(double mix) { m_wetMix = qBound(0.0, mix, 1.0); }
void Delay3::setPingPong(bool enabled) { m_pingPong = enabled; }
void Delay3::setFilterType(FilterType type) { m_filterType = type; }
void Delay3::setFilterCutoff(double cutoff) { m_filterCutoff = qBound(20.0, cutoff, m_sampleRate * 0.45); }
void Delay3::setTapModulation(double depth, double rate)
{
    m_modDepth = qBound(0.0, depth, 1.0);
    m_modRate = qMax(0.01, rate);
}

/* ---- Compute delay samples ---- */

void Delay3::updateDelaySamples()
{
    double ms = m_delayMs;
    if (m_tempoSync) {
        // Sync to quarter note at BPM
        ms = 60000.0 / m_bpm;
    }
    m_delaySamples = qMax(1, static_cast<int>(ms * m_sampleRate / 1000.0));
}

/* ---- Linear interpolation read ---- */

double Delay3::readInterp(const QVector<double>& buf, double delaySamps) const
{
    int bufSize = buf.size();
    int intDelay = static_cast<int>(delaySamps);
    double frac = delaySamps - intDelay;

    int pos0 = (m_writePos - intDelay + bufSize) % bufSize;
    int pos1 = (pos0 - 1 + bufSize) % bufSize;

    return buf[pos0] * (1.0 - frac) + buf[pos1] * frac;
}

/* ---- Feedback filter ---- */

double Delay3::applyFilter(double input, double& state) const
{
    if (m_filterType == None) return input;

    // Simple one-pole filter
    double rc = 1.0 / (2.0 * M_PI * m_filterCutoff);
    double dt = 1.0 / m_sampleRate;
    double alpha = dt / (rc + dt);

    if (m_filterType == LowPass) {
        state += alpha * (input - state);
        return state;
    } else {
        double y = input - state;
        state += alpha * (input - state);
        return y;
    }
}

/* ---- Process single mono sample ---- */

double Delay3::processSample(double input)
{
    int bufSize = m_bufferL.size();

    // Compute modulated delay
    double modDelay = m_delaySamples;
    if (m_modDepth > 0.0) {
        m_modPhase += m_modRate * 2.0 * M_PI / m_sampleRate;
        if (m_modPhase > 2.0 * M_PI) m_modPhase -= 2.0 * M_PI;
        modDelay += m_modDepth * m_delaySamples * 0.1 * qSin(m_modPhase);
    }
    modDelay = qMax(1.0, modDelay);

    // Read delayed signal
    double delayed = readInterp(m_bufferL, modDelay);

    // Apply feedback filter
    double filtered = applyFilter(delayed, m_filterStateL);

    // Write: input + feedback
    m_bufferL[m_writePos] = input + filtered * m_feedback;

    // Output: dry + wet
    double output = input * (1.0 - m_wetMix) + filtered * m_wetMix;

    m_writePos = (m_writePos + 1) % bufSize;
    m_stats.totalSamples++;
    return output;
}

/* ---- Process stereo sample pair ---- */

QPair<double, double> Delay3::processStereo(double inL, double inR)
{
    if (!m_pingPong) {
        double outL = processSample(inL);
        double outR = processSample(inR);
        return {outL, outR};
    }

    // Ping-pong: L feeds R delay, R feeds L delay
    int bufSize = m_bufferL.size();
    double modDelay = m_delaySamples;
    if (m_modDepth > 0.0) {
        m_modPhase += m_modRate * 2.0 * M_PI / m_sampleRate;
        if (m_modPhase > 2.0 * M_PI) m_modPhase -= 2.0 * M_PI;
        modDelay += m_modDepth * m_delaySamples * 0.1 * qSin(m_modPhase);
    }
    modDelay = qMax(1.0, modDelay);

    double delL = readInterp(m_bufferL, modDelay);
    double delR = readInterp(m_bufferR, modDelay);
    double filtL = applyFilter(delL, m_filterStateL);
    double filtR = applyFilter(delR, m_filterStateR);

    // Ping-pong cross feedback
    m_bufferL[m_writePos] = inL + filtR * m_feedback;
    m_bufferR[m_writePos] = inR + filtL * m_feedback;

    double outL = inL * (1.0 - m_wetMix) + filtL * m_wetMix;
    double outR = inR * (1.0 - m_wetMix) + filtR * m_wetMix;

    m_writePos = (m_writePos + 1) % bufSize;
    m_stats.totalSamples += 2;
    return {outL, outR};
}

/* ---- Batch process ---- */

QVector<double> Delay3::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processSample(input[i]);

    m_timeSum += timer.elapsed();
    m_stats.feedbackGain = m_feedback;
    m_stats.wetMix = m_wetMix;
    m_stats.avgProcessingTimeMs = (m_stats.totalSamples > 0)
        ? m_timeSum * 1000.0 / m_stats.totalSamples : 0.0;

    emit processingCompleted(input.size(), timer.elapsed());
    return output;
}

/* ---- Flush buffers ---- */

void Delay3::flush()
{
    m_bufferL.fill(0.0);
    m_bufferR.fill(0.0);
    m_writePos = 0;
    m_filterStateL = 0.0;
    m_filterStateR = 0.0;
    m_modPhase = 0.0;
}

/* ---- Reset statistics ---- */

void Delay3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    flush();
}
