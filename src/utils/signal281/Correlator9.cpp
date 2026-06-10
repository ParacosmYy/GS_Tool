/**
 * @file Correlator9.cpp
 * @brief Correlator9 实现
 *
 * 实现相关器：最大似然时延估计与Cramer-Rao界约束的最优统计时延估计。
 */

#include "utils/signal281/Correlator9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Correlator9::Correlator9(QObject *parent)
    : QObject(parent) {}

Correlator9::~Correlator9() = default;

/* ---- Configuration ---- */

void Correlator9::setSampleRate(double rate) { m_sampleRate = qBound(1.0, rate, 1e9); }
void Correlator9::setSearchRange(int minLag, int maxLag)
{
    m_minLag = qMax(0, minLag);
    m_maxLag = qMax(m_minLag + 1, maxLag);
}
void Correlator9::setInterpolationOrder(int order) { m_interpOrder = qBound(1, order, 5); }

/* ---- Apply Hann window ---- */

QVector<double> Correlator9::applyWindow(const QVector<double>& data) const
{
    int n = data.size();
    QVector<double> w(n, 0.0);
    for (int i = 0; i < n; ++i)
        w[i] = data[i] * (0.5 * (1.0 - qCos(2.0 * M_PI * i / n)));
    return w;
}

/* ---- Compute signal bandwidth for CRLB ---- */

double Correlator9::signalBandwidth(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < 4) return m_sampleRate / 2.0;

    // Compute energy spectral centroid as bandwidth estimate
    double totalEnergy = 0.0, weightedSum = 0.0;
    QVector<double> w = applyWindow(signal);

    for (int i = 0; i < n - 1; ++i) {
        double diff = w[i + 1] - w[i];
        double energy = diff * diff;
        double freq = i * m_sampleRate / n;
        totalEnergy += energy;
        weightedSum += freq * energy;
    }
    return (totalEnergy > 1e-300) ? weightedSum / totalEnergy : m_sampleRate / 4.0;
}

/* ---- Estimate SNR ---- */

double Correlator9::estimateSNR(const QVector<double>& signal, double noiseFloor) const
{
    double sigPow = 0.0;
    for (int i = 0; i < signal.size(); ++i) sigPow += signal[i] * signal[i];
    sigPow /= signal.size();
    return (noiseFloor > 1e-300) ? 10.0 * qLn(sigPow / noiseFloor) / qLn(10.0) : 30.0;
}

/* ---- Cross-correlation ---- */

Correlator9::CorrelationResult Correlator9::crossCorrelate(
    const QVector<double>& a, const QVector<double>& b) const
{
    CorrelationResult result;
    int n = a.size();
    int m = b.size();
    if (n < 1 || m < 1) return result;

    int maxLag = qMin(n, m_maxLag);
    result.lagMax = 0;
    result.peakValue = 0.0;
    result.correlation.resize(maxLag, 0.0);

    // Normalized cross-correlation for lags 0..maxLag-1
    double normA = 0.0;
    for (int i = 0; i < n; ++i) normA += a[i] * a[i];
    normA = qSqrt(normA);

    for (int lag = 0; lag < maxLag; ++lag) {
        double sum = 0.0;
        int count = qMin(n, m - lag);
        double normB = 0.0;
        for (int i = 0; i < count; ++i) {
            sum += a[i] * b[i + lag];
            normB += b[i + lag] * b[i + lag];
        }
        normB = qSqrt(normB);
        double norm = normA * normB;
        result.correlation[lag] = (norm > 1e-300) ? sum / norm : 0.0;

        if (result.correlation[lag] > result.peakValue) {
            result.peakValue = result.correlation[lag];
            result.lagMax = lag;
        }
    }
    result.normalizedPeak = result.peakValue;
    return result;
}

/* ---- Parabolic interpolation ---- */

double Correlator9::parabolicInterpolation(const QVector<double>& corr, int peakIdx) const
{
    if (peakIdx <= 0 || peakIdx >= corr.size() - 1) return static_cast<double>(peakIdx);
    double y0 = corr[peakIdx - 1];
    double y1 = corr[peakIdx];
    double y2 = corr[peakIdx + 1];
    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-300) return static_cast<double>(peakIdx);
    return static_cast<double>(peakIdx) + (y0 - y2) / denom;
}

/* ---- Gaussian interpolation ---- */

double Correlator9::gaussianInterpolation(const QVector<double>& corr, int peakIdx) const
{
    if (peakIdx <= 0 || peakIdx >= corr.size() - 1) return static_cast<double>(peakIdx);
    double y0 = qLn(qMax(corr[peakIdx - 1], 1e-300));
    double y1 = qLn(qMax(corr[peakIdx], 1e-300));
    double y2 = qLn(qMax(corr[peakIdx + 1], 1e-300));
    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-300) return static_cast<double>(peakIdx);
    return static_cast<double>(peakIdx) + (y0 - y2) / denom;
}

/* ---- Cramer-Rao lower bound ---- */

double Correlator9::cramerRaoBound(const QVector<double>& signal, double snr) const
{
    // CRLB for time delay: var(tau) >= 1 / (SNR * (2*pi*BW)^2 * N)
    int n = signal.size();
    double bw = signalBandwidth(signal);
    double snrLin = qPow(10.0, snr / 10.0);
    double omegaBW = 2.0 * M_PI * bw;
    double crlb = 1.0 / (snrLin * omegaBW * omegaBW * n);
    return crlb;
}

/* ---- Estimate delay via ML ---- */

Correlator9::DelayResult Correlator9::estimateDelay(
    const QVector<double>& ref, const QVector<double>& observed)
{
    QElapsedTimer timer;
    timer.start();

    DelayResult result;
    if (ref.size() < 4 || observed.size() < 4) return result;

    // Step 1: Cross-correlate within search range
    auto corrResult = crossCorrelate(ref, observed);
    if (corrResult.correlation.isEmpty()) return result;

    // Step 2: Find peak in correlation
    int peakIdx = corrResult.lagMax;
    if (peakIdx < m_minLag || peakIdx >= m_maxLag) {
        // Search within specified range
        double bestVal = -1e30;
        for (int i = m_minLag; i < qMin(m_maxLag, corrResult.correlation.size()); ++i) {
            if (corrResult.correlation[i] > bestVal) {
                bestVal = corrResult.correlation[i];
                peakIdx = i;
            }
        }
    }

    // Step 3: Sub-sample refinement via interpolation
    double fracDelay = parabolicInterpolation(corrResult.correlation, peakIdx);

    // Step 4: Estimate SNR
    double noiseFloor = 0.0;
    for (int i = 0; i < observed.size(); ++i)
        noiseFloor += (observed[i] - ref[i % ref.size()]) * (observed[i] - ref[i % ref.size()]);
    noiseFloor /= observed.size();
    double snr = estimateSNR(ref, noiseFloor);

    // Step 5: Compute CRLB
    double crlb = cramerRaoBound(ref, snr);

    result.delaySamples = fracDelay;
    result.delaySeconds = fracDelay / m_sampleRate;
    result.correlationPeak = corrResult.peakValue;
    result.snr = snr;
    result.cramerRaoBound = qSqrt(qMax(crlb, 0.0));
    result.searchIndex = peakIdx;
    result.confidence = qMin(1.0, corrResult.peakValue * qSqrt(snr));
    result.valid = true;

    double elapsed = timer.elapsed();
    m_stats.signalLength = ref.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    m_snrSum += snr;
    m_stats.avgSnr = m_snrSum / m_stats.totalOps;
    emit delayEstimated(result.delaySeconds, result.cramerRaoBound, elapsed);

    return result;
}

/* ---- Reset ---- */

void Correlator9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_snrSum = 0.0;
}
