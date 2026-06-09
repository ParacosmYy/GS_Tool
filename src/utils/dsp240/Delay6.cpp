/**
 * @file Delay6.cpp
 * @brief Delay6 实现
 *
 * 实现多抽头延迟：节拍同步抽头间距、反馈滤波与低/高切音色控制。
 */

#include "utils/dsp240/Delay6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstring>

/* ---- Construction / Destruction ---- */

Delay6::Delay6(QObject *parent) : QObject(parent) {}
Delay6::~Delay6() = default;

/* ---- Configuration ---- */

void Delay6::setSampleRate(int sr) { m_stats.sampleRate = qMax(8000, sr); updateFilterCoeffs(); }
void Delay6::setBPM(double bpm) { m_stats.bpm = qBound(20.0, bpm, 300.0); }
void Delay6::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Delay6::setLowCut(double freq) { m_lowCut = qBound(20.0, freq, 20000.0); updateFilterCoeffs(); }
void Delay6::setHighCut(double freq) { m_highCut = qBound(20.0, freq, 20000.0); updateFilterCoeffs(); }

void Delay6::setTaps(const QVector<TapConfig>& taps)
{
    m_taps = taps;
    m_stats.numTaps = taps.size();

    // Allocate delay lines (max 2 whole notes = 8 beats worth of samples)
    int maxDelay = beatToSamples(8.0);
    m_delayLines.resize(taps.size());
    m_writePos.resize(taps.size(), 0);
    for (int i = 0; i < taps.size(); ++i) {
        int delay = beatToSamples(taps[i].beatFraction);
        delay = qMax(1, delay);
        m_delayLines[i].resize(delay, 0.0);
        m_writePos[i] = 0;
    }
}

/* ---- Beat to samples ---- */

int Delay6::beatToSamples(double beatFraction) const
{
    double beatDuration = 60.0 / m_stats.bpm;
    double delaySec = beatDuration * beatFraction;
    return qMax(1, static_cast<int>(delaySec * m_stats.sampleRate));
}

/* ---- Update feedback filter coefficients (biquad) ---- */

void Delay6::updateFilterCoeffs()
{
    double sr = m_stats.sampleRate;

    // Low-cut: high-pass biquad
    double lcW = 2.0 * M_PI * m_lowCut / sr;
    double lcSin = qSin(lcW);
    double lcCos = qCos(lcW);
    double lcAlpha = lcSin / (2.0 * 0.707);
    m_lcb[0] = (1.0 + lcCos) / 2.0;
    m_lcb[1] = -(1.0 + lcCos);
    m_lcb[2] = m_lcb[0];
    double lca0 = 1.0 + lcAlpha;
    m_lcb[0] /= lca0; m_lcb[1] /= lca0; m_lcb[2] /= lca0;
    // a1, a2 stored as negatives for direct form II
    double lca1 = -2.0 * lcCos / lca0;
    double lca2 = (1.0 - lcAlpha) / lca0;
    m_lcx[0] = m_lcx[1] = m_lcx[2] = 0;
    m_lcy[0] = m_lcy[1] = m_lcy[2] = 0;
    // Store a1, a2 negated for difference equation y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2
    m_lcy[0] = lca1; m_lcy[1] = lca2;

    // High-cut: low-pass biquad
    double hcW = 2.0 * M_PI * m_highCut / sr;
    double hcSin = qSin(hcW);
    double hcCos = qCos(hcW);
    double hcAlpha = hcSin / (2.0 * 0.707);
    m_hcb[0] = (1.0 - hcCos) / 2.0;
    m_hcb[1] = 1.0 - hcCos;
    m_hcb[2] = m_hcb[0];
    double hca0 = 1.0 + hcAlpha;
    m_hcb[0] /= hca0; m_hcb[1] /= hca0; m_hcb[2] /= hca0;
    double hca1 = -2.0 * hcCos / hca0;
    double hca2 = (1.0 - hcAlpha) / hca0;
    m_hcx[0] = m_hcx[1] = m_hcx[2] = 0;
    m_hcy[0] = hca1; m_hcy[1] = hca2;
}

/* ---- Apply feedback tone filter ---- */

double Delay6::applyFeedbackFilter(double sample)
{
    // High-pass (low-cut)
    double x2 = m_lcx[1], x1 = m_lcx[0];
    m_lcx[0] = sample;
    double yHP = m_lcb[0] * m_lcx[0] + m_lcb[1] * x1 + m_lcb[2] * x2
                 - m_lcy[0] * m_lcy[2] - m_lcy[1] * m_lcy[2];
    // Simplified: use single-pole for stability
    yHP = sample; // passthrough for robustness; filter applied via simple 1st order
    double alpha = 1.0 - qExp(-2.0 * M_PI * m_lowCut / m_stats.sampleRate);
    static double hpState = 0.0;
    hpState = alpha * (hpState + sample - x1);
    yHP = hpState;

    // Low-pass (high-cut)
    alpha = 1.0 - qExp(-2.0 * M_PI * m_highCut / m_stats.sampleRate);
    static double lpState = 0.0;
    lpState += alpha * (yHP - lpState);

    return lpState;
}

/* ---- Process ---- */

void Delay6::process(QVector<double>& samples, int numFrames)
{
    QElapsedTimer timer;
    timer.start();

    if (m_taps.isEmpty() || m_delayLines.isEmpty()) return;

    for (int f = 0; f < numFrames; ++f) {
        double input = samples[f];
        double output = input;

        for (int t = 0; t < m_taps.size(); ++t) {
            int delayLen = m_delayLines[t].size();
            if (delayLen < 1) continue;

            int readPos = (m_writePos[t] - delayLen / 2 + delayLen) % delayLen;
            double delayed = m_delayLines[t][readPos];

            // Mix tap output
            output += delayed * m_taps[t].gain * m_wetMix;

            // Write input + feedback into delay line
            double feedback = applyFeedbackFilter(delayed * m_feedback);
            m_delayLines[t][m_writePos[t]] = input + feedback;
            m_writePos[t] = (m_writePos[t] + 1) % delayLen;
        }

        samples[f] = output;
    }

    m_stats.framesProcessed += numFrames;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processCompleted(numFrames, timer.elapsed());
}

/* ---- Reset ---- */

void Delay6::reset()
{
    for (auto& dl : m_delayLines)
        std::fill(dl.begin(), dl.end(), 0.0);
    std::fill(m_writePos.begin(), m_writePos.end(), 0);
}

void Delay6::resetStatistics()
{
    reset();
    m_stats = Stats{}; m_timeSum = 0.0;
}
