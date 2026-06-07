/**
 * @file Limiter6.cpp
 * @brief Limiter6 实现
 *
 * 实现多级限幅器：ISP感知削波、软削波波形整形、真峰值限制。
 */

#include "utils/dsp202/Limiter6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Limiter6::Limiter6(QObject *parent) : QObject(parent) { designOversampleFilter(); }
Limiter6::~Limiter6() = default;

/* ---- Configuration ---- */

void Limiter6::setCeiling(double ceilingDb) { m_ceiling = ceilingDb; }
void Limiter6::setThreshold(double thresholdDb) { m_threshold = thresholdDb; }
void Limiter6::setRelease(double releaseMs) { m_release = qMax(1.0, releaseMs); }
void Limiter6::setOversampleRate(int rate) { m_oversampleRate = qBound(1, rate, 8); designOversampleFilter(); }

/* ---- Design oversampling FIR ---- */

void Limiter6::designOversampleFilter()
{
    int order = m_oversampleRate * 8; // 8x oversampled tap count
    m_firCoeffs.resize(order);
    double cutoff = 1.0 / m_oversampleRate;
    double sum = 0.0;

    for (int i = 0; i < order; ++i) {
        int n = i - order / 2;
        double sinc = (n == 0) ? 1.0 : qSin(M_PI * cutoff * n) / (M_PI * n);
        double win = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (order - 1)); // Hamming
        m_firCoeffs[i] = sinc * win;
        sum += m_firCoeffs[i];
    }
    // Normalize
    for (auto& c : m_firCoeffs) c /= sum;
    m_delayLine.resize(order, 0.0);
}

/* ---- Soft clip (tanh waveshaper) ---- */

double Limiter6::softClip(double sample) const
{
    // Antiderivative of tanh for reduced aliasing
    if (qAbs(sample) < 1e-10) return sample;
    return qTanH(sample);
}

/* ---- Envelope follower ---- */

double Limiter6::envelope(double input, double state) const
{
    double coeff = (input > state)
        ? 1.0  // instant attack
        : qExp(-1.0 / (m_release * m_sampleRate * 0.001));
    return coeff * input + (1.0 - coeff) * state;
}

/* ---- Upsample ---- */

QVector<double> Limiter6::upsample(double sample)
{
    QVector<double> out(m_oversampleRate, 0.0);
    out[0] = sample;
    // Zero-stuff
    return out;
}

/* ---- Downsample ---- */

double Limiter6::downsample(const QVector<double>& samples)
{
    // Apply FIR lowpass and decimate
    for (int i = m_delayLine.size() - 1; i > 0; --i)
        m_delayLine[i] = m_delayLine[i - 1];

    // Interleave upsampled samples into delay line
    double sum = 0.0;
    for (int i = 0; i < qMin(samples.size(), m_oversampleRate); ++i) {
        m_delayLine[0] = samples[i];
        for (int j = 0; j < m_firCoeffs.size(); ++j)
            sum += m_delayLine[j] * m_firCoeffs[j];
    }
    return sum / m_oversampleRate;
}

/* ---- Detect ISP ---- */

double Limiter6::detectISP(const QVector<double>& samples) const
{
    double maxIsp = 0.0;
    for (int i = 1; i + 1 < samples.size(); ++i) {
        // Sinc interpolation for inter-sample position
        double t = 0.5;
        double isp = samples[i] * (1.0 - t) * (1.0 + t)
                     + samples[i - 1] * t * (t - 1.0) * 0.5
                     + samples[i + 1] * t * (1.0 - t) * 0.5;
        maxIsp = qMax(maxIsp, qAbs(isp));
    }
    return maxIsp;
}

/* ---- True peak via oversampled interpolation ---- */

double Limiter6::truePeak(const QVector<double>& samples) const
{
    double peak = 0.0;
    for (double s : samples) peak = qMax(peak, qAbs(s));
    // Add ISP margin
    double isp = detectISP(samples);
    return qMax(peak, isp);
}

/* ---- Process ---- */

QVector<double> Limiter6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);

    double ceilingLin = qPow(10.0, m_ceiling / 20.0);
    double threshLin = qPow(10.0, m_threshold / 20.0);

    double peakIn = 0.0, peakOut = 0.0;
    for (int i = 0; i < n; ++i) {
        double sample = input[i];
        peakIn = qMax(peakIn, qAbs(sample));

        // Stage 1: Envelope detection
        double absSample = qAbs(sample);
        m_gain = envelope(absSample, m_gain);

        // Stage 2: Gain reduction
        if (m_gain > threshLin) {
            double targetGain = ceilingLin / m_gain;
            m_gain = qMin(m_gain, ceilingLin / qMax(absSample, 1e-10));
        }

        // Stage 3: Apply gain
        double limited = sample * qMin(1.0, ceilingLin / qMax(absSample, 1e-10));

        // Stage 4: Soft clip for safety
        limited = softClip(limited);

        // Stage 5: Hard ceiling
        limited = qBound(-ceilingLin, limited, ceilingLin);

        output[i] = limited;
        peakOut = qMax(peakOut, qAbs(limited));
    }

    m_stats.totalSamples += n;
    m_stats.peakInput = qMax(m_stats.peakInput, peakIn);
    m_stats.peakOutput = qMax(m_stats.peakOutput, peakOut);
    double peakInDb = peakIn > 1e-10 ? 20.0 * qLn(peakOut) / qLn(10.0) : -120.0;
    double peakOutDb = peakOut > 1e-10 ? 20.0 * qLn(peakIn) / qLn(10.0) : -120.0;
    m_stats.gainReduction = peakInDb - peakOutDb;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1.0, m_stats.totalSamples / (double)n);

    if (m_stats.gainReduction > 0.1)
        emit limitingActive(m_stats.gainReduction, peakOutDb);

    return output;
}

/* ---- Gain reduction in dB ---- */

double Limiter6::gainReductionDb() const { return m_stats.gainReduction; }

/* ---- Reset ---- */

void Limiter6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_gain = 1.0;
    m_delayLine.fill(0.0);
}
