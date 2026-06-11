/**
 * @file Compressor10.cpp
 * @brief Compressor10 实现
 *
 * 实现动态压缩器：并行信号混合与自动增益补偿实现RMS/峰值检测模式切换的透明动态控制。
 */

#include "utils/dsp299/Compressor10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Compressor10::Compressor10(QObject *parent)
    : QObject(parent) {}

Compressor10::~Compressor10() = default;

/* ---- Configuration ---- */

void Compressor10::setParameters(const Parameters& params)
{
    m_params = params;
    m_params.thresholdDb = qBound(-120.0, params.thresholdDb, 0.0);
    m_params.ratio = qBound(1.0, params.ratio, 100.0);
    m_params.kneeDb = qBound(0.0, params.kneeDb, 24.0);
    m_params.attackMs = qBound(0.01, params.attackMs, 500.0);
    m_params.releaseMs = qBound(1.0, params.releaseMs, 5000.0);
    m_params.makeupGainDb = qBound(-60.0, params.makeupGainDb, 60.0);
    m_params.mixPercent = qBound(0.0, params.mixPercent, 100.0);
}

void Compressor10::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); }

/* ---- Utility conversions ---- */

double Compressor10::linearToDb(double linear) const
{
    return 20.0 * qLn(qMax(1e-10, qFabs(linear))) / M_LN10;
}

double Compressor10::dbToLinear(double db) const
{
    return qPow(10.0, db / 20.0);
}

/* ---- Compute gain reduction with soft knee ---- */

double Compressor10::computeGainReduction(double inputDb) const
{
    double t = m_params.thresholdDb;
    double r = m_params.ratio;
    double w = m_params.kneeDb;

    double halfW = w / 2.0;
    double diff = inputDb - t;

    if (w <= 0.0) {
        // Hard knee
        if (diff <= 0.0) return 0.0;
        return -diff * (1.0 - 1.0 / r);
    }

    // Soft knee interpolation
    if (diff < -halfW) return 0.0;
    if (diff > halfW) return -diff * (1.0 - 1.0 / r);

    // Quadratic interpolation across knee region
    double x = diff + halfW;
    double gainRed = -(x * x) / (2.0 * w) * (1.0 - 1.0 / r);
    return gainRed;
}

/* ---- Update envelope with ballistics ---- */

double Compressor10::updateEnvelope(double sample, double dt)
{
    double absSample = qFabs(sample);
    double alpha = 0.0;

    if (m_params.mode == DetectionMode::RMS) {
        // RMS ballistics: smooth squared envelope
        double attackCoeff = qExp(-dt / (m_params.attackMs * 0.001));
        double releaseCoeff = qExp(-dt / (m_params.releaseMs * 0.001));
        alpha = (absSample * absSample > m_envelope) ? attackCoeff : releaseCoeff;
        m_envelope = alpha * m_envelope + (1.0 - alpha) * absSample * absSample;
        return qSqrt(qMax(0.0, m_envelope));
    }

    // Peak detection
    double attackCoeff = qExp(-dt / (m_params.attackMs * 0.001));
    double releaseCoeff = qExp(-dt / (m_params.releaseMs * 0.001));
    alpha = (absSample > m_envelope) ? attackCoeff : releaseCoeff;
    m_envelope = alpha * m_envelope + (1.0 - alpha) * absSample;
    return m_envelope;
}

/* ---- Compute auto makeup gain ---- */

double Compressor10::autoMakeupGain() const
{
    if (m_params.makeupGainDb != 0.0) return m_params.makeupGainDb;
    // Auto: compensate for half the compression at threshold
    double compressionAtThreshold = (m_params.thresholdDb) * (1.0 - 1.0 / m_params.ratio) * 0.5;
    return -compressionAtThreshold * 0.5;
}

/* ---- RMS level over window ---- */

double Compressor10::rmsLevel(const QVector<double>& window) const
{
    double sumSq = 0.0;
    for (double s : window) sumSq += s * s;
    return qSqrt(sumSq / qMax(1, window.size()));
}

/* ---- Main process ---- */

QVector<double> Compressor10::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double dt = 1.0 / m_sampleRate;
    double mixWet = m_params.mixPercent / 100.0;
    double mixDry = 1.0 - mixWet;
    double makeup = autoMakeupGain();
    double makeupLin = dbToLinear(makeup);

    double maxReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double dry = input[i];
        double env = updateEnvelope(dry, dt);
        double envDb = linearToDb(env);
        double gainRedDb = computeGainReduction(envDb);
        double gainLin = dbToLinear(gainRedDb) * makeupLin;

        // Compressed signal
        double compressed = dry * gainLin;

        // Parallel blending: mix dry and wet
        output[i] = mixDry * dry + mixWet * compressed;

        double redDb = -gainRedDb;
        if (redDb > maxReduction) maxReduction = redDb;
    }

    m_lastGR.inputLevelDb = (n > 0) ? linearToDb(rmsLevel(input)) : -120.0;
    m_lastGR.outputLevelDb = (n > 0) ? linearToDb(rmsLevel(output)) : -120.0;
    m_lastGR.reductionDb = maxReduction;
    m_lastGR.gainDb = makeup;

    m_stats.totalBlocks++;
    m_stats.peakReductionDb = qMax(m_stats.peakReductionDb, maxReduction);
    m_reductionSum += maxReduction;
    m_stats.avgReductionDb = m_reductionSum / m_stats.totalBlocks;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;

    emit processDone(n, maxReduction, elapsed);
    return output;
}

/* ---- Reset ---- */

void Compressor10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reductionSum = 0.0;
    m_envelope = 0.0;
}
