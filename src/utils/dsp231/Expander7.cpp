/**
 * @file Expander7.cpp
 * @brief Expander7 实现
 *
 * 实现下行扩展器：程序依赖释放与对数增益曲线自然门行为。
 */

#include "utils/dsp231/Expander7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander7::Expander7(QObject *parent) : QObject(parent) {}
Expander7::~Expander7() = default;

/* ---- Configuration ---- */

void Expander7::setParameters(double threshold, double ratio, double attack,
                                double release, double knee, double sampleRate)
{
    m_threshold = qBound(-96.0, threshold, 0.0);
    m_ratio = qMax(1.0, ratio);
    m_attack = qMax(0.01, attack);
    m_release = qMax(0.1, release);
    m_knee = qBound(0.0, knee, 24.0);
    m_sampleRate = qMax(8000.0, sampleRate);
    m_range = -80.0;
}

/* ---- dB conversion ---- */

double Expander7::dbToLinear(double db)
{
    return qPow(10.0, db / 20.0);
}

double Expander7::linearToDb(double linear)
{
    return 20.0 * qLn(qMax(1e-10, linear)) / qLn(10.0);
}

/* ---- Logarithmic gain curve ---- */

double Expander7::computeGainCurve(double inputDb) const
{
    // Logarithmic gain curve for natural gate behavior
    if (inputDb >= m_threshold + m_knee / 2.0) {
        // Above threshold: unity gain
        return 0.0;
    } else if (inputDb <= m_threshold - m_knee / 2.0) {
        // Below threshold: logarithmic expansion
        double diff = m_threshold - inputDb;
        // Logarithmic curve: gain = -diff * (ratio - 1) / ratio * log_factor
        double logFactor = 1.0 - 1.0 / (1.0 + diff / 20.0);
        double expansion = diff * (m_ratio - 1.0) / m_ratio;
        double gainReduction = -expansion * logFactor;
        return qMax(m_range, gainReduction);
    } else {
        // Soft knee: quadratic interpolation
        double halfKnee = m_knee / 2.0;
        double x = inputDb - (m_threshold - halfKnee);
        double t = x / m_knee;
        double diff = m_threshold - inputDb;
        double logFactor = 1.0 - 1.0 / (1.0 + diff / 20.0);
        double expansion = diff * (m_ratio - 1.0) / m_ratio;
        return qMax(m_range, -expansion * logFactor * t * t);
    }
}

/* ---- Program-dependent release ---- */

double Expander7::programRelease(double inputDb) const
{
    // Louder signals get faster release for natural behavior
    double levelAboveThreshold = inputDb - m_threshold;
    if (levelAboveThreshold > 0.0) return m_release * 0.25;

    double depth = qAbs(levelAboveThreshold);
    // Deeper below threshold -> slower release
    double factor = 0.5 + 0.5 * qLn(1.0 + depth / 20.0) / qLn(2.0);
    return m_release * factor;
}

/* ---- Smooth envelope ---- */

double Expander7::smoothEnvelope(double target, double current, double coeff) const
{
    // One-pole smoothing
    return current + coeff * (target - current);
}

/* ---- Process single frame ---- */

QVector<double> Expander7::processFrame(const QVector<double>& frame)
{
    int channels = frame.size();
    if (m_gainEnvelope.size() != channels) {
        m_gainEnvelope.fill(1.0, channels);
        m_gainReduction.fill(0.0, channels);
    }

    QVector<double> output(channels);
    for (int ch = 0; ch < channels; ++ch) {
        double absSample = qAbs(frame[ch]);
        double inputDb = linearToDb(qMax(1e-10, absSample));

        // Compute target gain from logarithmic curve
        double targetGainDb = computeGainCurve(inputDb);
        double targetGainLin = dbToLinear(targetGainDb);

        // Attack/release coefficients
        double effectiveRelease = programRelease(inputDb);
        double attackCoeff = 1.0 - qExp(-1.0 / (m_attack * 0.001 * m_sampleRate));
        double releaseCoeff = 1.0 - qExp(-1.0 / (effectiveRelease * 0.001 * m_sampleRate));

        // Smooth gain envelope
        double coeff = (targetGainLin < m_gainEnvelope[ch]) ? attackCoeff : releaseCoeff;
        m_gainEnvelope[ch] = smoothEnvelope(targetGainLin, m_gainEnvelope[ch], coeff);

        // Apply gain
        output[ch] = frame[ch] * m_gainEnvelope[ch];

        m_gainReduction[ch] = linearToDb(m_gainEnvelope[ch]);
        double prevReduction = m_gainReduction[ch];
        Q_UNUSED(prevReduction)
    }

    m_stats.numFrames++;
    m_stats.totalOps++;
    return output;
}

/* ---- Process block ---- */

QVector<double> Expander7::processBlock(const QVector<double>& data, int channels)
{
    QElapsedTimer timer;
    timer.start();

    int frames = data.size() / channels;
    if (frames <= 0) return data;

    QVector<double> output(data.size());
    for (int f = 0; f < frames; ++f) {
        QVector<double> frame(channels);
        for (int ch = 0; ch < channels; ++ch)
            frame[ch] = data[f * channels + ch];

        QVector<double> processed = processFrame(frame);
        for (int ch = 0; ch < channels; ++ch)
            output[f * channels + ch] = processed[ch];
    }

    // Update average gain reduction
    double sumGR = 0.0;
    for (double gr : m_gainReduction) sumGR += gr;
    m_stats.avgGainReduction = sumGR / qMax(1, m_gainReduction.size());
    double peakGR = 0.0;
    for (double gr : m_gainReduction) peakGR = qMin(peakGR, gr);
    m_stats.peakGainReduction = peakGR;
    m_stats.numChannels = channels;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return output;
}

/* ---- Current gain ---- */

QVector<double> Expander7::currentGain() const { return m_gainEnvelope; }

/* ---- Gain reduction dB ---- */

QVector<double> Expander7::gainReductionDb() const { return m_gainReduction; }

/* ---- Reset ---- */

void Expander7::resetStatistics()
{
    m_gainEnvelope.clear();
    m_gainReduction.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
