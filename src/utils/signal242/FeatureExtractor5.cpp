/**
 * @file FeatureExtractor5.cpp
 * @brief FeatureExtractor5 实现
 *
 * 实现特征提取：时域统计描述符与频谱质心信号表征。
 */

#include "utils/signal242/FeatureExtractor5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>

/* ---- Construction / Destruction ---- */

FeatureExtractor5::FeatureExtractor5(QObject *parent) : QObject(parent) {}
FeatureExtractor5::~FeatureExtractor5() = default;

/* ---- Configuration ---- */

void FeatureExtractor5::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); }

/* ---- Time-domain statistical features ---- */

void FeatureExtractor5::computeTimeDomain(const QVector<double>& frame, FeatureVector& fv) const
{
    int N = frame.size();
    if (N == 0) return;

    // Mean
    double sum = 0.0;
    for (double s : frame) sum += s;
    fv.mean = sum / N;

    // Variance, RMS, peak, peak-to-peak
    double sumSq = 0.0;
    double minVal = frame[0], maxVal = frame[0];
    for (double s : frame) {
        double diff = s - fv.mean;
        sumSq += diff * diff;
        if (s < minVal) minVal = s;
        if (s > maxVal) maxVal = s;
    }
    fv.variance = sumSq / N;
    fv.stdDev = qSqrt(fv.variance);

    // RMS
    double sumSqRaw = 0.0;
    for (double s : frame) sumSqRaw += s * s;
    fv.rms = qSqrt(sumSqRaw / N);

    // Peak and peak-to-peak
    fv.peak = qMax(qAbs(maxVal), qAbs(minVal));
    fv.peakToPeak = maxVal - minVal;

    // Crest factor: peak / RMS
    fv.crestFactor = (fv.rms > 1e-15) ? fv.peak / fv.rms : 0.0;

    // Zero crossing rate
    int crossings = 0;
    for (int i = 1; i < N; ++i) {
        if ((frame[i] >= 0.0) != (frame[i - 1] >= 0.0))
            crossings++;
    }
    fv.zeroCrossRate = static_cast<double>(crossings) / qMax(1, N - 1);

    // Skewness (3rd central moment / std^3)
    double sum3 = 0.0;
    for (double s : frame) {
        double d = s - fv.mean;
        sum3 += d * d * d;
    }
    fv.skewness = (fv.stdDev > 1e-15) ? (sum3 / N) / (fv.stdDev * fv.stdDev * fv.stdDev) : 0.0;

    // Kurtosis (4th central moment / std^4 - 3)
    double sum4 = 0.0;
    for (double s : frame) {
        double d = s - fv.mean;
        double d2 = d * d;
        sum4 += d2 * d2;
    }
    fv.kurtosis = (fv.stdDev > 1e-15)
        ? (sum4 / N) / (fv.stdDev * fv.stdDev * fv.stdDev * fv.stdDev) - 3.0
        : 0.0;

    // Energy
    fv.energy = sumSqRaw;
}

/* ---- Magnitude spectrum (DFT) ---- */

QVector<double> FeatureExtractor5::magnitudeSpectrum(const QVector<double>& frame) const
{
    int N = frame.size();
    int halfN = N / 2 + 1;
    QVector<double> mag(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
    return mag;
}

/* ---- Spectral features ---- */

void FeatureExtractor5::computeSpectral(const QVector<double>& frame, FeatureVector& fv) const
{
    QVector<double> mag = magnitudeSpectrum(frame);
    int halfN = mag.size();
    if (halfN == 0) return;

    // Total magnitude for normalization
    double totalMag = 0.0;
    for (double m : mag) totalMag += m;
    if (totalMag < 1e-15) return;

    // Spectral centroid: weighted mean frequency
    double binWidth = static_cast<double>(m_sampleRate) / frame.size();
    double centroid = 0.0;
    for (int k = 0; k < halfN; ++k)
        centroid += k * binWidth * mag[k];
    fv.spectralCentroid = centroid / totalMag;

    // Spectral spread: weighted std dev of frequency
    double spread = 0.0;
    for (int k = 0; k < halfN; ++k) {
        double diff = k * binWidth - fv.spectralCentroid;
        spread += mag[k] * diff * diff;
    }
    fv.spectralSpread = qSqrt(spread / totalMag);

    // Spectral rolloff: frequency below which 85% of energy is concentrated
    double threshold = 0.85 * totalMag;
    double cumSum = 0.0;
    fv.spectralRolloff = 0.0;
    for (int k = 0; k < halfN; ++k) {
        cumSum += mag[k];
        if (cumSum >= threshold) {
            fv.spectralRolloff = k * binWidth;
            break;
        }
    }
}

/* ---- Extract features from one frame ---- */

FeatureExtractor5::FeatureVector FeatureExtractor5::extract(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    FeatureVector fv;
    if (frame.isEmpty()) return fv;

    computeTimeDomain(frame, fv);
    computeSpectral(frame, fv);

    m_stats.frameSize = frame.size();
    m_stats.numExtractions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit extractionCompleted(frame.size(), timer.elapsed());
    return fv;
}

/* ---- Batch extraction ---- */

QVector<FeatureExtractor5::FeatureVector> FeatureExtractor5::extractBatch(
    const QVector<QVector<double>>& frames)
{
    QVector<FeatureVector> results;
    results.reserve(frames.size());
    for (const auto& frame : frames)
        results.append(extract(frame));
    return results;
}

/* ---- Flatten feature vector ---- */

QVector<double> FeatureExtractor5::flatten(const FeatureVector& fv)
{
    return { fv.mean, fv.variance, fv.stdDev, fv.skewness, fv.kurtosis,
             fv.rms, fv.peak, fv.peakToPeak, fv.crestFactor, fv.zeroCrossRate,
             fv.spectralCentroid, fv.spectralSpread, fv.spectralRolloff, fv.energy };
}

/* ---- Reset ---- */

void FeatureExtractor5::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
