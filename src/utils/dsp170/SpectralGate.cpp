/**
 * @file SpectralGate.cpp
 * @brief SpectralGate 实现
 *
 * 实现频谱门限：噪声底估计、阈值曲线计算、增益平滑、帧处理。
 */

#include "utils/dsp170/SpectralGate.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction ---- */

SpectralGate::SpectralGate(int fftSize, QObject *parent)
    : QObject(parent), m_fftSize(qMax(16, fftSize))
{
    m_noiseFloorDb.fill(-80.0, m_fftSize / 2 + 1);
    m_gainState.fill(1.0, m_fftSize / 2 + 1);

    /* Compute attack/release coefficients (assuming 44100 Hz sample rate) */
    double sampleRate = 44100.0;
    double frameDur = m_fftSize / sampleRate;
    m_attackCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_attackMs / 1000.0));
    m_releaseCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_releaseMs / 1000.0));
}

SpectralGate::~SpectralGate() = default;

/* ---- Configuration ---- */

void SpectralGate::setThreshold(double threshDb) { m_thresholdDb = threshDb; }

void SpectralGate::setRatio(double ratio) { m_ratio = qMax(1.0, ratio); }

void SpectralGate::setAttack(double ms)
{
    m_attackMs = qMax(0.1, ms);
    double sampleRate = 44100.0;
    double frameDur = m_fftSize / sampleRate;
    m_attackCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_attackMs / 1000.0));
}

void SpectralGate::setRelease(double ms)
{
    m_releaseMs = qMax(0.1, ms);
    double sampleRate = 44100.0;
    double frameDur = m_fftSize / sampleRate;
    m_releaseCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_releaseMs / 1000.0));
}

void SpectralGate::setThresholdCurve(ThresholdCurve curve) { m_curveType = curve; }

void SpectralGate::setCustomCurve(const QVector<double>& curveDb)
{
    m_customCurveDb = curveDb;
}

/* ---- dB helpers ---- */

double SpectralGate::toDb(double linear) { return 20.0 * qLog10(qMax(1e-30, linear)); }
double SpectralGate::fromDb(double db) { return qPow(10.0, db / 20.0); }

/* ---- Noise floor estimation ---- */

void SpectralGate::estimateNoiseFloor(const QVector<QVector<double>>& noiseFrames)
{
    if (noiseFrames.isEmpty()) return;
    int bins = noiseFrames[0].size();
    m_noiseFloorDb.resize(bins);

    /* Average magnitude across frames then convert to dB */
    for (int b = 0; b < bins; ++b) {
        double sum = 0.0;
        for (const auto& frame : noiseFrames)
            sum += frame[b];
        double avg = sum / noiseFrames.size();
        m_noiseFloorDb[b] = toDb(avg);
    }

    m_stats.lastNoiseFloor = 0.0;
    for (double v : m_noiseFloorDb) m_stats.lastNoiseFloor += v;
    m_stats.lastNoiseFloor /= bins;
}

void SpectralGate::setNoiseFloor(const QVector<double>& floorDb)
{
    m_noiseFloorDb = floorDb;
}

QVector<double> SpectralGate::noiseFloor() const { return m_noiseFloorDb; }

/* ---- Bin threshold ---- */

double SpectralGate::binThreshold(int bin) const
{
    int bins = m_noiseFloorDb.size();
    if (bins == 0) return m_thresholdDb;

    double noiseOffset = (bin < bins) ? m_noiseFloorDb[bin] : -80.0;
    double curveOffset = 0.0;

    switch (m_curveType) {
    case Flat:
        curveOffset = 0.0;
        break;
    case Linear:
        curveOffset = m_thresholdDb * bin / qMax(1, bins - 1);
        break;
    case Logarithmic:
        curveOffset = m_thresholdDb * qLn(bin + 1) / qLn(bins);
        break;
    case Custom:
        curveOffset = (bin < m_customCurveDb.size()) ? m_customCurveDb[bin] : 0.0;
        break;
    }

    return noiseOffset + m_thresholdDb + curveOffset;
}

/* ---- Process frame ---- */

QVector<double> SpectralGate::process(const QVector<double>& magnitude)
{
    QElapsedTimer timer;
    timer.start();

    int bins = magnitude.size();
    QVector<double> output(bins);

    double totalReduction = 0.0;

    for (int b = 0; b < bins; ++b) {
        double sigDb = toDb(magnitude[b]);
        double threshDb = binThreshold(b);

        /* Compute target gain */
        double gain;
        if (sigDb >= threshDb) {
            /* Above threshold: pass through */
            gain = 1.0;
        } else {
            /* Below threshold: apply expansion */
            double diff = threshDb - sigDb;
            double gainDb = -diff * (m_ratio - 1.0) / m_ratio;
            gain = fromDb(qMax(-120.0, gainDb));
        }

        /* Smooth gain with attack/release */
        double& state = m_gainState[b];
        if (gain < state)
            state += m_attackCoeff * (gain - state);
        else
            state += m_releaseCoeff * (gain - state);

        output[b] = magnitude[b] * state;
        totalReduction += toDb(qMax(1e-30, state));
    }

    m_stats.totalFrames++;
    m_stats.avgReduction = (bins > 0) ? totalReduction / bins : 0.0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit frameProcessed(m_stats.totalFrames);
    return output;
}

/* ---- Statistics ---- */

void SpectralGate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
