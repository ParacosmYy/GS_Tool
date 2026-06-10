/**
 * @file Delay9.cpp
 * @brief Delay9 实现
 *
 * 实现延迟效果：多拍节奏同步模式与乒乓立体声路由的节奏空间延迟效果。
 */

#include "utils/dsp282/Delay9.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Delay9::Delay9(QObject *parent)
    : QObject(parent)
{
    // Default tap pattern: quarter note + dotted eighth
    TapConfig t1, t2;
    t1.beatOffset = 1.0; t1.gain = 0.7; t1.pan = -0.7; t1.pingPong = true;
    t2.beatOffset = 1.5; t2.gain = 0.5; t2.pan = 0.7;  t2.pingPong = true;
    m_taps = {t1, t2};
    initBuffers();
}

Delay9::~Delay9() = default;

/* ---- Configuration ---- */

void Delay9::setSampleRate(int rate)
{
    m_sampleRate = qMax(8000, rate);
    m_stats.sampleRate = m_sampleRate;
    initBuffers();
}

void Delay9::setTempo(double bpm)
{
    m_bpm = qBound(20.0, bpm, 300.0);
    m_stats.bpm = m_bpm;
    initBuffers();
}

void Delay9::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Delay9::setMix(double mix) { m_mix = qBound(0.0, mix, 1.0); }

void Delay9::setTaps(const QVector<TapConfig>& taps)
{
    m_taps = taps.isEmpty() ? m_taps : taps;
    initBuffers();
}

/* ---- Beat offset to delay samples ---- */

int Delay9::beatToSamples(double beatOffset) const
{
    // beats per second = bpm / 60
    double bps = m_bpm / 60.0;
    double seconds = beatOffset / bps;
    return qMax(1, static_cast<int>(seconds * m_sampleRate));
}

/* ---- Pan gain: stereo spread ---- */

Delay9::StereoSample Delay9::panGain(double pan, double sample) const
{
    StereoSample out;
    // Constant-power pan law
    double angle = (pan + 1.0) * 0.25 * M_PI;  // Map [-1,1] to [0, pi/2]
    out.left = sample * qCos(angle);
    out.right = sample * qSin(angle);
    return out;
}

/* ---- Circular buffer read ---- */

double Delay9::readBuffer(int tapIdx, int offset) const
{
    if (tapIdx < 0 || tapIdx >= m_delayBuffers.size()) return 0.0;
    const auto& buf = m_delayBuffers[tapIdx];
    int sz = buf.size();
    int readPos = (m_writePos[tapIdx] - offset + sz) % sz;
    return buf[readPos];
}

/* ---- Circular buffer write ---- */

void Delay9::writeBuffer(int tapIdx, double value)
{
    if (tapIdx < 0 || tapIdx >= m_delayBuffers.size()) return;
    m_delayBuffers[tapIdx][m_writePos[tapIdx]] = value;
    m_writePos[tapIdx] = (m_writePos[tapIdx] + 1) % m_delayBuffers[tapIdx].size();
}

/* ---- Initialize delay buffers ---- */

void Delay9::initBuffers()
{
    int numTaps = m_taps.size();
    m_delayBuffers.resize(numTaps);
    m_writePos.resize(numTaps);

    for (int i = 0; i < numTaps; ++i) {
        int delaySamples = beatToSamples(m_taps[i].beatOffset);
        m_delayBuffers[i] = QVector<double>(delaySamples, 0.0);
        m_writePos[i] = 0;
    }
}

/* ---- Process mono to stereo with ping-pong routing ---- */

QVector<Delay9::StereoSample> Delay9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int frames = input.size();
    QVector<StereoSample> output(frames);
    double peak = 0.0;

    for (int i = 0; i < frames; ++i) {
        double dry = input[i];
        StereoSample mixed;
        mixed.left = dry * (1.0 - m_mix);
        mixed.right = dry * (1.0 - m_mix);

        // Sum contributions from each tap
        for (int t = 0; t < m_taps.size(); ++t) {
            int delaySamps = m_delayBuffers[t].size();

            // Read delayed signal
            double delayed = readBuffer(t, delaySamps - 1);

            // Ping-pong: alternate L/R emphasis
            double panValue = m_taps[t].pan;
            if (m_taps[t].pingPong) {
                // Flip pan based on feedback iteration (use write position as proxy)
                int phase = (m_writePos[t] / qMax(1, delaySamps / 2)) % 2;
                panValue = (phase == 0) ? m_taps[t].pan : -m_taps[t].pan;
            }

            StereoSample panned = panGain(panValue, delayed * m_taps[t].gain * m_mix);
            mixed.left += panned.left;
            mixed.right += panned.right;

            // Write input + feedback to buffer
            double fbSignal = dry + delayed * m_feedback;
            writeBuffer(t, fbSignal);
        }

        double level = qMax(qAbs(mixed.left), qAbs(mixed.right));
        peak = qMax(peak, level);
        output[i] = mixed;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processDone(frames, peak, elapsed);

    return output;
}

/* ---- Process stereo input with ping-pong routing ---- */

QVector<Delay9::StereoSample> Delay9::processStereo(const QVector<StereoSample>& input)
{
    QElapsedTimer timer;
    timer.start();

    int frames = input.size();
    QVector<StereoSample> output(frames);
    double peak = 0.0;

    for (int i = 0; i < frames; ++i) {
        double dryL = input[i].left;
        double dryR = input[i].right;
        double dryMono = (dryL + dryR) * 0.5;

        StereoSample mixed;
        mixed.left = dryL * (1.0 - m_mix);
        mixed.right = dryR * (1.0 - m_mix);

        for (int t = 0; t < m_taps.size(); ++t) {
            int delaySamps = m_delayBuffers[t].size();
            double delayed = readBuffer(t, delaySamps - 1);

            double panValue = m_taps[t].pan;
            if (m_taps[t].pingPong) {
                int phase = (m_writePos[t] / qMax(1, delaySamps / 2)) % 2;
                panValue = (phase == 0) ? m_taps[t].pan : -m_taps[t].pan;
            }

            StereoSample panned = panGain(panValue, delayed * m_taps[t].gain * m_mix);
            mixed.left += panned.left;
            mixed.right += panned.right;

            // Ping-pong feedback: cross-feed L/R into opposite buffer channel
            double fbSignal = dryMono + delayed * m_feedback;
            writeBuffer(t, fbSignal);
        }

        double level = qMax(qAbs(mixed.left), qAbs(mixed.right));
        peak = qMax(peak, level);
        output[i] = mixed;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processDone(frames, peak, elapsed);

    return output;
}

/* ---- Reset delay buffers ---- */

void Delay9::reset()
{
    for (auto& buf : m_delayBuffers)
        buf.fill(0.0);
    m_writePos.fill(0);
}

/* ---- Reset statistics ---- */

void Delay9::resetStatistics()
{
    m_stats = Stats{};
    m_stats.sampleRate = m_sampleRate;
    m_stats.bpm = m_bpm;
    m_timeSum = 0.0;
}
