/**
 * @file Compressor6.cpp
 * @brief Compressor6 实现
 *
 * 实现动态范围压缩器：软拐点特性曲线与RMS/峰值双检测模式。
 */

#include "utils/dsp243/Compressor6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Compressor6::Compressor6(QObject *parent) : QObject(parent) {}
Compressor6::~Compressor6() = default;

/* ---- Configuration ---- */

void Compressor6::setThreshold(double thresholdDb) { m_thresholdDb = thresholdDb; }
void Compressor6::setRatio(double ratio) { m_ratio = qMax(1.0, ratio); }
void Compressor6::setKneeWidth(double widthDb) { m_kneeWidthDb = qMax(0.0, widthDb); }
void Compressor6::setAttackTime(double ms) { m_attackMs = qMax(0.01, ms); }
void Compressor6::setReleaseTime(double ms) { m_releaseMs = qMax(0.01, ms); }
void Compressor6::setDetectionMode(DetectionMode mode) { m_mode = mode; }
void Compressor6::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }

/* ---- dB conversion helpers ---- */

double Compressor6::toDb(double amp) { return 20.0 * qLn(qMax(qAbs(amp), 1e-10)) / M_LN2 / 6.0; }
double Compressor6::fromDb(double db) { return qExp(db * M_LN2 * 6.0 / 20.0); }

/* ---- Compute gain reduction with soft knee ---- */

double Compressor6::computeGainReduction(double inputDb) const
{
    double T = m_thresholdDb;
    double W = m_kneeWidthDb;
    double R = m_ratio;

    if (W <= 0.0) {
        // Hard knee
        if (inputDb <= T) return 0.0;
        return (inputDb - T) * (1.0 / R - 1.0);
    }

    // Soft knee: quadratic transition region [T - W/2, T + W/2]
    double halfW = W / 2.0;
    if (inputDb < T - halfW) {
        // Below knee: no compression
        return 0.0;
    } else if (inputDb > T + halfW) {
        // Above knee: full compression
        return (inputDb - T) * (1.0 / R - 1.0);
    } else {
        // Inside soft knee: quadratic interpolation
        double x = inputDb - T + halfW;
        return (1.0 / R - 1.0) * x * x / (2.0 * W);
    }
}

/* ---- Process ---- */

QVector<double> Compressor6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    m_gainReduction.resize(n);

    // Envelope coefficients
    double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * m_sampleRate));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * m_sampleRate));

    // RMS window length (samples)
    int rmsWindow = qMax(1, static_cast<int>(m_sampleRate * 0.01));
    double rmsSum = 0.0;

    double inputPeak = 0.0;
    double outputPeak = 0.0;
    double totalGR = 0.0;

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(input[i]);

        // Detection: compute input level
        double detected;
        if (m_mode == RMS) {
            rmsSum += input[i] * input[i];
            if (i >= rmsWindow) {
                double old = input[i - rmsWindow];
                rmsSum -= old * old;
            }
            int winLen = qMin(i + 1, rmsWindow);
            detected = qSqrt(qMax(rmsSum, 0.0) / winLen);
        } else {
            detected = absSample;
        }

        // Envelope follower
        double coeff = (detected > m_envelope) ? attackCoeff : releaseCoeff;
        m_envelope = detected + coeff * (m_envelope - detected);
        m_envelope = qMax(m_envelope, 0.0);

        // Compute gain reduction
        double envDb = toDb(m_envelope);
        double grDb = computeGainReduction(envDb);
        m_gainReduction[i] = grDb;

        // Apply gain
        double gain = fromDb(grDb);
        output[i] = input[i] * gain;

        inputPeak = qMax(inputPeak, absSample);
        outputPeak = qMax(outputPeak, qAbs(output[i]));
        totalGR += grDb;
    }

    m_stats.numSamples = n;
    m_stats.inputPeakDb = toDb(inputPeak);
    m_stats.outputPeakDb = toDb(outputPeak);
    m_stats.avgGainReductionDb = (n > 0) ? totalGR / n : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit compressionCompleted(n, m_stats.avgGainReductionDb, timer.elapsed());
    return output;
}

/* ---- Accessors ---- */

QVector<double> Compressor6::gainReduction() const { return m_gainReduction; }

/* ---- Reset ---- */

void Compressor6::resetStatistics()
{
    m_envelope = 0.0;
    m_gainReduction.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
