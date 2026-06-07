/**
 * @file SignalSynchronizer3.cpp
 * @brief SignalSynchronizer3 实现
 *
 * 实现导频音相关信号同步：载波频偏估计、频偏校正、定时同步。
 */

#include "utils/signal203/SignalSynchronizer3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalSynchronizer3::SignalSynchronizer3(QObject *parent) : QObject(parent) {}
SignalSynchronizer3::~SignalSynchronizer3() = default;

/* ---- Configuration ---- */

void SignalSynchronizer3::setPilotTone(double freqHz, double sampleRate)
{
    m_pilotFreqHz = freqHz;
    m_sampleRate = qMax(1.0, sampleRate);
}

void SignalSynchronizer3::setSearchRange(double maxFreqOffsetHz) { m_maxFreqOffsetHz = qMax(1.0, maxFreqOffsetHz); }
void SignalSynchronizer3::setCorrelationThreshold(double threshold) { m_corrThreshold = qBound(0.0, threshold, 1.0); }

/* ---- Generate pilot tone ---- */

QVector<double> SignalSynchronizer3::generatePilot(int length) const
{
    QVector<double> pilot(length);
    for (int i = 0; i < length; ++i)
        pilot[i] = qSin(2.0 * M_PI * m_pilotFreqHz * i / m_sampleRate);
    return pilot;
}

/* ---- Parabolic interpolation ---- */

double SignalSynchronizer3::parabolicInterp(double y0, double y1, double y2) const
{
    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-15) return 0.0;
    return (y0 - y2) / denom;
}

/* ---- Cross-correlate ---- */

QVector<double> SignalSynchronizer3::crossCorrelate(const QVector<double>& signal,
                                                       const QVector<double>& pilot) const
{
    int n = signal.size();
    int m = pilot.size();
    int corrLen = n - m + 1;
    if (corrLen <= 0) return {};

    QVector<double> correlation(corrLen, 0.0);
    for (int lag = 0; lag < corrLen; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < m; ++i)
            sum += signal[lag + i] * pilot[i];
        correlation[lag] = sum;
    }
    return correlation;
}

/* ---- Correlate at specific frequency ---- */

double SignalSynchronizer3::correlateAtFreq(const QVector<double>& signal, double freq) const
{
    double sumI = 0.0, sumQ = 0.0;
    for (int i = 0; i < signal.size(); ++i) {
        double phase = 2.0 * M_PI * freq * i / m_sampleRate;
        sumI += signal[i] * qCos(phase);
        sumQ += signal[i] * qSin(phase);
    }
    return qSqrt(sumI * sumI + sumQ * sumQ);
}

/* ---- Estimate CFO ---- */

double SignalSynchronizer3::estimateCFO(const QVector<double>& signal) const
{
    // Coarse search: sweep frequency offset range
    double bestFreq = 0.0;
    double bestCorr = 0.0;
    double stepHz = m_maxFreqOffsetHz / 50.0;

    for (double f = -m_maxFreqOffsetHz; f <= m_maxFreqOffsetHz; f += stepHz) {
        double testFreq = m_pilotFreqHz + f;
        double corr = correlateAtFreq(signal, testFreq);
        if (corr > bestCorr) { bestCorr = corr; bestFreq = f; }
    }

    // Fine search: refine around best frequency
    double fineStep = stepHz / 10.0;
    for (double f = bestFreq - stepHz; f <= bestFreq + stepHz; f += fineStep) {
        double testFreq = m_pilotFreqHz + f;
        double corr = correlateAtFreq(signal, testFreq);
        if (corr > bestCorr) { bestCorr = corr; bestFreq = f; }
    }

    return bestFreq;
}

/* ---- Correct CFO ---- */

QVector<double> SignalSynchronizer3::correctCFO(const QVector<double>& signal, double cfo) const
{
    QVector<double> corrected(signal.size());
    for (int i = 0; i < signal.size(); ++i) {
        double phase = -2.0 * M_PI * cfo * i / m_sampleRate;
        corrected[i] = signal[i] * qCos(phase);
    }
    return corrected;
}

/* ---- Find timing offset ---- */

int SignalSynchronizer3::findTimingOffset(const QVector<double>& correlation) const
{
    if (correlation.isEmpty()) return 0;
    double maxVal = correlation[0];
    int maxIdx = 0;
    for (int i = 1; i < correlation.size(); ++i) {
        if (correlation[i] > maxVal) { maxVal = correlation[i]; maxIdx = i; }
    }
    return maxIdx;
}

/* ---- Synchronize ---- */

QVector<double> SignalSynchronizer3::synchronize(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) return signal;

    // Estimate carrier frequency offset
    double cfo = estimateCFO(signal);

    // Correct CFO
    QVector<double> corrected = correctCFO(signal, cfo);

    // Generate pilot and find timing
    QVector<double> pilot = generatePilot(qMin(n, 256));
    QVector<double> corr = crossCorrelate(corrected, pilot);
    int timingOffset = findTimingOffset(corr);

    // Shift signal by timing offset
    QVector<double> synced(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int src = i + timingOffset;
        if (src >= 0 && src < n) synced[i] = corrected[src];
    }

    double peakCorr = corr.isEmpty() ? 0.0 :
        corr[qBound(0, timingOffset, corr.size() - 1)];

    m_stats.totalSyncs++;
    m_stats.signalLength = n;
    m_stats.estimatedCFO = cfo;
    m_stats.correlationPeak = peakCorr;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSyncs;

    emit syncCompleted(cfo, timingOffset, timer.elapsed());
    return synced;
}

/* ---- Reset ---- */

void SignalSynchronizer3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
