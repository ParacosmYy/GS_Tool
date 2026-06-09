/**
 * @file PitchDetector7.cpp
 * @brief PitchDetector7 实现
 *
 * 实现音高检测：自相关函数与YIN差分函数抛物线插值精化。
 */

#include "utils/signal246/PitchDetector7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PitchDetector7::PitchDetector7(QObject *parent) : QObject(parent) {}
PitchDetector7::~PitchDetector7() = default;

/* ---- Configuration ---- */

void PitchDetector7::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    m_stats.sampleRate = m_sampleRate;
}

void PitchDetector7::setMinFrequency(double freq) { m_minFreq = qMax(20.0, freq); }
void PitchDetector7::setMaxFrequency(double freq) { m_maxFreq = qMin(freq, m_sampleRate / 2.0); }
void PitchDetector7::setVoicedThreshold(double threshold) { m_voicedThreshold = qBound(0.0, threshold, 1.0); }

/* ---- Autocorrelation function ---- */

QVector<double> PitchDetector7::autocorrelation(const QVector<double>& samples) const
{
    int n = samples.size();
    int maxLag = n / 2;
    QVector<double> acf(maxLag, 0.0);

    for (int lag = 0; lag < maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i)
            sum += samples[i] * samples[i + lag];
        acf[lag] = sum;
    }

    // Normalize by zero-lag energy
    if (acf[0] > 0.0) {
        double norm = acf[0];
        for (auto& v : acf) v /= norm;
    }
    return acf;
}

/* ---- YIN difference function ---- */

QVector<double> PitchDetector7::yinDifference(const QVector<double>& samples) const
{
    int n = samples.size();
    int halfN = n / 2;
    QVector<double> diff(halfN, 0.0);

    // Step 2: Difference function d(τ) = Σ (x[j] - x[j+τ])²
    for (int tau = 0; tau < halfN; ++tau) {
        double sum = 0.0;
        for (int j = 0; j < halfN; ++j)
            sum += (samples[j] - samples[j + tau]) * (samples[j] - samples[j + tau]);
        diff[tau] = sum;
    }
    return diff;
}

/* ---- Cumulative mean normalized difference function ---- */

QVector<double> PitchDetector7::cumulativeMeanNormalized(const QVector<double>& diff) const
{
    int n = diff.size();
    QVector<double> cmndf(n, 0.0);
    cmndf[0] = 1.0;
    double runningSum = 0.0;

    for (int tau = 1; tau < n; ++tau) {
        runningSum += diff[tau];
        cmndf[tau] = (runningSum > 0.0) ? diff[tau] * tau / runningSum : 1.0;
    }
    return cmndf;
}

/* ---- Parabolic interpolation ---- */

double PitchDetector7::parabolicRefine(const QVector<double>& data, int minIdx) const
{
    if (minIdx <= 0 || minIdx >= data.size() - 1) return minIdx;

    double y0 = data[minIdx - 1];
    double y1 = data[minIdx];
    double y2 = data[minIdx + 1];

    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qFabs(denom) < 1e-30) return minIdx;

    double offset = (y0 - y2) / denom;
    return minIdx + offset;
}

/* ---- Find first minimum below threshold (YIN step 4) ---- */

int PitchDetector7::findFirstMinimum(const QVector<double>& cmndf, int minLag, int maxLag) const
{
    // Find the first tau where cmndf[tau] < threshold
    int bestTau = -1;
    double minVal = std::numeric_limits<double>::max();

    for (int tau = minLag; tau < maxLag; ++tau) {
        if (cmndf[tau] < m_voicedThreshold) {
            // Find the local minimum in this region
            if (cmndf[tau] < minVal) {
                minVal = cmndf[tau];
                bestTau = tau;
            }
        } else if (bestTau >= 0) {
            // We already found a dip below threshold and now it's above again
            break;
        }
    }
    return bestTau;
}

/* ---- Detect pitch ---- */

PitchDetector7::PitchResult PitchDetector7::detect(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;

    int n = samples.size();
    if (n < 64) {
        result.voiced = false;
        return result;
    }

    // Compute lag range from frequency limits
    int minLag = qMax(2, static_cast<int>(m_sampleRate / m_maxFreq));
    int maxLag = qMin(n / 2, static_cast<int>(m_sampleRate / m_minFreq));

    // YIN method
    QVector<double> diff = yinDifference(samples);
    QVector<double> cmndf = cumulativeMeanNormalized(diff);

    // Find period via absolute threshold
    int tau = findFirstMinimum(cmndf, minLag, maxLag);

    if (tau < 0) {
        // No pitch found below threshold; try autocorrelation fallback
        QVector<double> acf = autocorrelation(samples);
        double maxAcf = 0.0;
        for (int t = minLag; t < maxLag; ++t) {
            if (acf[t] > maxAcf) { maxAcf = acf[t]; tau = t; }
        }
        if (maxAcf < 0.3) {
            result.voiced = false;
            m_stats.numUnvoiced++;
            m_stats.blockSize = n;
            m_stats.totalOps++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
            return result;
        }
    }

    // Parabolic interpolation for sub-sample precision
    double refinedTau = parabolicRefine(cmndf, tau);
    if (refinedTau < 1.0) refinedTau = tau;

    double frequency = m_sampleRate / refinedTau;
    double confidence = 1.0 - (tau >= 0 && tau < cmndf.size() ? cmndf[tau] : 1.0);
    confidence = qBound(0.0, confidence, 1.0);

    result.frequency = frequency;
    result.confidence = confidence;
    result.period = tau;
    result.refinedPeriod = refinedTau;
    result.voiced = (frequency >= m_minFreq && frequency <= m_maxFreq);

    if (result.voiced) m_stats.numVoiced++;
    else m_stats.numUnvoiced++;

    m_stats.blockSize = n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit pitchDetected(result.frequency, result.confidence, result.voiced);
    return result;
}

/* ---- Reset ---- */

void PitchDetector7::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
