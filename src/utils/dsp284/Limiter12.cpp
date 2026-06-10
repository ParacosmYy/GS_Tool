/**
 * @file Limiter12.cpp
 * @brief Limiter12 实现
 *
 * 实现限幅器：多频段前瞻与过采样帧间真峰值检测的广播合规处理。
 */

#include "utils/dsp284/Limiter12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Limiter12::Limiter12(QObject *parent)
    : QObject(parent)
{
    designOversampleFilter();
}

Limiter12::~Limiter12() = default;

/* ---- Configuration ---- */

void Limiter12::setConfig(const LimiterConfig& cfg)
{
    m_config = cfg;
    m_config.numBands = qBound(1, cfg.numBands, 8);
    m_config.lookaheadSamples = qBound(0, cfg.lookaheadSamples, 512);
    m_config.oversampleFactor = qBound(2, cfg.oversampleFactor, 8);

    m_delayLine.resize(m_config.lookaheadSamples, 0.0);
    m_bandBuffers.resize(m_config.numBands);
    m_smoother.resize(m_config.numBands, 1.0);
    m_delayPos = 0;

    designOversampleFilter();
}

/* ---- Design oversampling filter ---- */

void Limiter12::designOversampleFilter()
{
    int order = 63;
    int factor = m_config.oversampleFactor;
    m_osFilter.resize(order + 1);

    double cutoff = 1.0 / static_cast<double>(factor);
    double sum = 0.0;

    for (int i = 0; i <= order; ++i) {
        int center = order / 2;
        double n = static_cast<double>(i - center);
        double x = M_PI * cutoff * n;

        // Sinc function
        double sinc = (qAbs(n) < 1e-10) ? 1.0 : qSin(x) / x;

        // Blackman window
        double w = 0.42 - 0.5 * qCos(2.0 * M_PI * i / order)
                   + 0.08 * qCos(4.0 * M_PI * i / order);

        m_osFilter[i] = sinc * w;
        sum += m_osFilter[i];
    }

    // Normalize
    for (auto& c : m_osFilter)
        c /= sum;
}

/* ---- Oversample a single sample ---- */

QVector<double> Limiter12::oversample(double sample) const
{
    int factor = m_config.oversampleFactor;
    int filterLen = m_osFilter.size();
    QVector<double> os(factor * 2, 0.0);

    // Zero-stuff and filter (simplified: polyphase)
    for (int p = 0; p < factor; ++p) {
        double acc = 0.0;
        for (int k = 0; k < filterLen / factor; ++k) {
            int idx = p + k * factor;
            if (idx < filterLen)
                acc += m_osFilter[idx];
        }
        os[p] = (p == 0) ? sample * acc : 0.0;
    }
    return os;
}

/* ---- True peak detection ---- */

double Limiter12::truePeak(const QVector<double>& frame) const
{
    double maxPeak = 0.0;
    for (double s : frame) {
        QVector<double> os = oversample(s);
        for (double v : os)
            if (qAbs(v) > maxPeak) maxPeak = qAbs(v);
    }
    return maxPeak;
}

/* ---- Compute gain reduction ---- */

double Limiter12::computeGain(double peak) const
{
    double ceilingLin = qPow(10.0, m_config.ceiling / 20.0);
    double threshLin = qPow(10.0, m_config.threshold / 20.0);

    if (peak <= threshLin) return 1.0;
    if (peak < 1e-30) return 1.0;

    return qMin(1.0, ceilingLin / peak);
}

/* ---- Crossover split ---- */

void Limiter12::crossover(const QVector<double>& input)
{
    int n = input.size();
    int bands = m_config.numBands;

    for (int b = 0; b < bands; ++b) {
        m_bandBuffers[b].resize(n);
        double freq = 100.0 * qPow(10.0, static_cast<double>(b) / bands * 3.0);
        double alpha = qExp(-2.0 * M_PI * freq / 44100.0);

        for (int i = 0; i < n; ++i) {
            if (i == 0)
                m_bandBuffers[b][i] = input[i] * (1.0 - alpha);
            else
                m_bandBuffers[b][i] = alpha * m_bandBuffers[b][i - 1]
                                      + (1.0 - alpha) * input[i];
        }
    }
}

/* ---- Smooth gain with release ballistics ---- */

double Limiter12::smoothGain(double target)
{
    double coeff = (target < m_currentGain)
                   ? 1.0 - qExp(-1.0 / (m_config.release * 44.1))
                   : 1.0;

    m_currentGain = m_currentGain + coeff * (target - m_currentGain);
    return m_currentGain;
}

/* ---- Process ---- */

Limiter12::LimiterResult Limiter12::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    LimiterResult result;
    int n = input.size();
    if (n == 0) return result;

    result.output.resize(n);

    // Detect true peak in input
    double peakIn = truePeak(input);
    result.peakIn = peakIn;

    // Multi-band crossover
    crossover(input);

    // Lookahead gain computation
    int la = m_config.lookaheadSamples;
    if (m_delayLine.size() != la)
        m_delayLine.resize(la, 0.0);

    double maxGR = 0.0;
    double peakOut = 0.0;
    bool clipped = false;

    for (int i = 0; i < n; ++i) {
        // Detect peak across all bands
        double bandPeak = 0.0;
        for (int b = 0; b < m_config.numBands; ++b) {
            double bp = qAbs(m_bandBuffers[b][i]);
            if (bp > bandPeak) bandPeak = bp;
        }

        // True peak via oversampling
        QVector<double> os = oversample(input[i]);
        for (double v : os) {
            double av = qAbs(v);
            if (av > bandPeak) bandPeak = av;
        }

        // Gain reduction
        double targetGain = computeGain(bandPeak);
        double gain = smoothGain(targetGain);

        double gr = 20.0 * qLog10(qMax(gain, 1e-30));
        if (gr < maxGR) maxGR = gr;

        // Lookahead delay: read output from delay line
        double delayed = m_delayLine[m_delayPos];
        m_delayLine[m_delayPos] = input[i];
        m_delayPos = (m_delayPos + 1) % qMax(la, 1);

        // Apply gain
        double out = (la > 0) ? delayed * gain : input[i] * gain;
        result.output[i] = out;

        double absOut = qAbs(out);
        if (absOut > peakOut) peakOut = absOut;

        double ceilingLin = qPow(10.0, m_config.ceiling / 20.0);
        if (absOut > ceilingLin * 1.01) clipped = true;
    }

    result.peakOut = peakOut;
    result.gainReduction = maxGR;
    result.clippingOccurred = clipped;

    double elapsed = timer.elapsed();
    m_stats.frameSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processDone(n, peakOut, maxGR, elapsed);

    return result;
}

/* ---- Reset ---- */

void Limiter12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_delayLine.fill(0.0);
    m_delayPos = 0;
    m_currentGain = 1.0;
}
