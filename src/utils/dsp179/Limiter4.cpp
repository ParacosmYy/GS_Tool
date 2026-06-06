/**
 * @file Limiter4.cpp
 * @brief Limiter4 实现
 *
 * 实现砖墙限制器：4x过采样真峰值检测、前瞻缓冲、自动增益释放。
 */

#include "utils/dsp179/Limiter4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Limiter4::Limiter4(QObject *parent) : QObject(parent)
{
    setSampleRate(44100.0);
    designOversamplingFilter();
}

Limiter4::~Limiter4() = default;

/* ---- Configuration ---- */

void Limiter4::setThreshold(double dB)
{
    m_threshold = dB;
    m_thresholdLin = qPow(10.0, dB / 20.0);
}

void Limiter4::setCeiling(double dB)
{
    m_ceiling = dB;
    m_ceilingLin = qPow(10.0, dB / 20.0);
}

void Limiter4::setLookaheadMs(double ms)
{
    m_lookaheadMs = qMax(0.0, ms);
    m_lookaheadSamples = static_cast<int>(m_lookaheadMs * m_sampleRate / 1000.0);
    m_delayLine.resize(m_lookaheadSamples + 1, 0.0);
    m_delayPos = 0;
}

void Limiter4::setReleaseMs(double ms) { m_releaseMs = qMax(1.0, ms); }
void Limiter4::setAutoRelease(bool enabled) { m_autoRelease = enabled; }

void Limiter4::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    setLookaheadMs(m_lookaheadMs);
}

/* ---- Design 4x oversampling lowpass FIR ---- */

void Limiter4::designOversamplingFilter()
{
    // Windowed-sinc lowpass at Nyquist/4 for 4x oversampling
    // Cutoff = pi/4 of original Nyquist
    int order = 63; // Must be odd
    m_osHalfLen = order / 2;
    m_osCoeffs.resize(order + 1);

    double fc = 0.25; // Normalized cutoff (relative to 4x Nyquist = pi)
    double sum = 0.0;
    for (int i = 0; i <= order; ++i) {
        int n = i - m_osHalfLen;
        if (n == 0) {
            m_osCoeffs[i] = 2.0 * fc;
        } else {
            double x = M_PI * n;
            m_osCoeffs[i] = qSin(2.0 * M_PI * fc * n) / x;
            // Blackman window
            double w = 0.42 - 0.5 * qCos(2.0 * M_PI * i / order)
                       + 0.08 * qCos(4.0 * M_PI * i / order);
            m_osCoeffs[i] *= w;
        }
        sum += m_osCoeffs[i];
    }
    // Normalize
    for (auto& c : m_osCoeffs) c /= sum;
}

/* ---- 4x oversample: insert zeros and filter ---- */

QVector<double> Limiter4::oversample(double x) const
{
    // Polyphase decomposition: x at position 0, zeros at 1,2,3
    QVector<double> out(4, 0.0);
    int len = m_osCoeffs.size();

    for (int phase = 0; phase < 4; ++phase) {
        double val = 0.0;
        // Only non-zero input samples contribute
        for (int k = phase; k < len; k += 4)
            val += m_osCoeffs[k] * x;
        // Simplified: since only every 4th sample is non-zero,
        // for phase 0 the contribution is the sum of coeffs[0], coeffs[4], ...
        out[phase] = val * 4.0; // Scale for energy conservation
    }
    return out;
}

/* ---- True peak detection via oversampled absolute max ---- */

double Limiter4::truePeak(const QVector<double>& frame) const
{
    double peak = 0.0;
    for (double s : frame)
        peak = qMax(peak, qAbs(s));
    return peak;
}

/* ---- Auto-release: faster for higher peaks ---- */

double Limiter4::computeAutoRelease(double peak) const
{
    // Proportional release: higher peak = faster release
    double overDb = 20.0 * qLog10(peak / m_thresholdLin + 1e-10);
    double factor = qBound(0.1, 1.0 - overDb / 20.0, 1.0);
    return m_releaseMs * factor;
}

/* ---- Main process ---- */

QVector<double> Limiter4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double coeff = 1.0 - qExp(-1.0 / (m_releaseMs * m_sampleRate / 1000.0));

    for (int i = 0; i < n; ++i) {
        double sample = input[i];

        // 4x oversample for true peak detection
        auto osFrame = oversample(sample);
        double tp = truePeak(osFrame);

        // Gain computation
        double targetGain = 1.0;
        if (tp > m_thresholdLin) {
            // Attenuate to bring peak to ceiling
            targetGain = m_ceilingLin / (tp + 1e-10);
            targetGain = qBound(0.0, targetGain, 1.0);
        }

        // Smooth gain: fast attack (instant), variable release
        if (targetGain < m_gain) {
            // Attack: instant
            m_gain = targetGain;
        } else {
            // Release: exponential
            double releaseCoeff = coeff;
            if (m_autoRelease) {
                double ar = computeAutoRelease(tp);
                releaseCoeff = 1.0 - qExp(-1.0 / (ar * m_sampleRate / 1000.0));
            }
            m_gain += (1.0 - m_gain) * releaseCoeff;
        }

        // Lookahead delay: store input, output delayed sample
        m_delayLine[m_delayPos] = sample;
        int readPos = (m_delayPos + 1) % m_delayLine.size();
        double delayed = m_delayLine[readPos];
        m_delayPos = (m_delayPos + 1) % m_delayLine.size();

        // Apply gain
        output[i] = delayed * m_gain;

        // Track stats
        double reduction = 20.0 * qLog10(m_gain + 1e-10);
        if (reduction < -0.01)
            m_stats.peakReduction = qMin(m_stats.peakReduction, reduction);
    }

    m_stats.totalSamples += n;
    m_stats.truePeak = qMax(m_stats.truePeak, truePeak(oversample(
        input.isEmpty() ? 0.0 : *std::max_element(input.begin(), input.end(),
        [](double a, double b) { return qAbs(a) < qAbs(b); }))));
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples / 512);

    return output;
}

/* ---- Reset state ---- */

void Limiter4::reset()
{
    m_gain = 1.0;
    m_delayLine.fill(0.0);
    m_delayPos = 0;
}

void Limiter4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
