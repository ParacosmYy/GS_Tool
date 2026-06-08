/**
 * @file Compressor4.cpp
 * @brief Compressor4 实现
 *
 * 实现动态范围压缩器：前馈增益检测、软拐点对数曲线、包络跟随。
 */

#include "utils/dsp215/Compressor4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Compressor4::Compressor4(QObject *parent) : QObject(parent)
{
    updateCoefficients();
}

Compressor4::~Compressor4() = default;

/* ---- Configuration ---- */

void Compressor4::setParameters(double thresholdDb, double ratio,
                                 double kneeWidthDb, double attackMs,
                                 double releaseMs, int sampleRate,
                                 double makeupGainDb)
{
    m_thresholdDb = thresholdDb;
    m_ratio = qMax(1.0, ratio);
    m_kneeWidthDb = qMax(0.0, kneeWidthDb);
    m_attackMs = qMax(0.01, attackMs);
    m_releaseMs = qMax(0.01, releaseMs);
    m_sampleRate = qMax(8000, sampleRate);
    m_makeupGainDb = makeupGainDb;

    m_stats.thresholdDb = m_thresholdDb;
    m_stats.ratio = m_ratio;
    m_stats.kneeWidthDb = m_kneeWidthDb;
    m_stats.sampleRate = m_sampleRate;

    updateCoefficients();
}

/* ---- Update envelope coefficients ---- */

void Compressor4::updateCoefficients()
{
    // Time constant: tau = -T / ln(1 - alpha) => alpha = 1 - exp(-1/(tau*fs))
    double attackTau = m_attackMs / 1000.0;
    double releaseTau = m_releaseMs / 1000.0;
    double sr = static_cast<double>(m_sampleRate);
    m_attackCoeff = (attackTau > 0.0)
        ? 1.0 - qExp(-1.0 / (attackTau * sr)) : 1.0;
    m_releaseCoeff = (releaseTau > 0.0)
        ? 1.0 - qExp(-1.0 / (releaseTau * sr)) : 1.0;
}

/* ---- Amplitude <-> dB conversions ---- */

double Compressor4::ampToDb(double amp)
{
    return 20.0 * qLn(qMax(1e-10, qAbs(amp))) / qLn(10.0);
}

double Compressor4::dbToAmp(double db)
{
    return qPow(10.0, db / 20.0);
}

/* ---- Soft-knee transfer function ---- */

double Compressor4::transferFunction(double inputDb) const
{
    double T = m_thresholdDb;
    double W = m_kneeWidthDb;
    double R = m_ratio;

    if (W <= 0.0) {
        // Hard knee
        return (inputDb <= T) ? inputDb
                              : T + (inputDb - T) / R;
    }

    double halfW = W / 2.0;
    if (inputDb < T - halfW) {
        // Below knee: unity gain
        return inputDb;
    } else if (inputDb > T + halfW) {
        // Above knee: compressed
        return T + (inputDb - T) / R;
    } else {
        // Inside soft-knee region: quadratic interpolation
        double x = inputDb - T + halfW;
        return inputDb + ((1.0 / R) - 1.0) * x * x / (2.0 * W);
    }
}

/* ---- Compute gain reduction ---- */

double Compressor4::computeGainReduction(double inputLevelDb) const
{
    double outputDb = transferFunction(inputLevelDb);
    return outputDb - inputLevelDb;  // Negative = gain reduction
}

/* ---- Process single sample ---- */

double Compressor4::processOne(double sample)
{
    // Feed-forward: detect input level
    double inputDb = ampToDb(sample);
    if (inputDb < -120.0) inputDb = -120.0;

    // Envelope follower (smoothed level detection)
    double coeff = (inputDb > m_envelope) ? m_attackCoeff : m_releaseCoeff;
    m_envelope += coeff * (inputDb - m_envelope);

    // Compute gain via transfer function
    double outputDb = transferFunction(m_envelope);
    double gainDb = outputDb - m_envelope + m_makeupGainDb;

    return sample * dbToAmp(gainDb);
}

/* ---- Process block ---- */

QVector<double> Compressor4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(input.size());

    double maxGainReduction = 0.0;
    for (int i = 0; i < input.size(); ++i) {
        double inputDb = ampToDb(input[i]);
        if (inputDb < -120.0) inputDb = -120.0;

        double coeff = (inputDb > m_envelope) ? m_attackCoeff : m_releaseCoeff;
        m_envelope += coeff * (inputDb - m_envelope);

        double outputDb = transferFunction(m_envelope);
        double gainDb = outputDb - m_envelope + m_makeupGainDb;
        double gr = m_envelope - outputDb;
        if (gr > maxGainReduction) maxGainReduction = gr;

        output.append(input[i] * dbToAmp(gainDb));
    }

    m_stats.blockSize = input.size();
    m_stats.gainReductionDb = maxGainReduction;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(input.size(), maxGainReduction, timer.elapsed());

    return output;
}

/* ---- Envelope level ---- */

double Compressor4::envelopeLevel() const { return m_envelope; }

/* ---- Reset ---- */

void Compressor4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = -120.0;
    updateCoefficients();
}
