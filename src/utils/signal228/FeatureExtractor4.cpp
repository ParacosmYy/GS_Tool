/**
 * @file FeatureExtractor4.cpp
 * @brief FeatureExtractor4 实现
 *
 * 实现频谱质心/滚降/通量特征提取、统计矩管线与z-score归一化。
 */

#include "utils/signal228/FeatureExtractor4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FeatureExtractor4::FeatureExtractor4(QObject *parent) : QObject(parent) {}
FeatureExtractor4::~FeatureExtractor4() = default;

/* ---- Configuration ---- */

void FeatureExtractor4::setParameters(int sampleRate, int frameSize)
{
    m_sampleRate = qMax(8000, sampleRate);
    m_frameSize = qMax(64, frameSize);
    m_stats.sampleRate = m_sampleRate;
    m_stats.frameSize = m_frameSize;
}

/* ---- Hann window ---- */

QVector<double> FeatureExtractor4::hannWindow(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        windowed[i] = frame[i] * w;
    }
    return windowed;
}

/* ---- Magnitude spectrum via DFT ---- */

QVector<double> FeatureExtractor4::magnitudeSpectrum(
    const QVector<double>& windowed) const
{
    int n = windowed.size();
    int halfN = n / 2 + 1;
    QVector<double> mag(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            re += windowed[i] * qCos(angle);
            im -= windowed[i] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
    return mag;
}

/* ---- Frequency bin to Hz ---- */

double FeatureExtractor4::binToHz(int bin) const
{
    return static_cast<double>(bin) * m_sampleRate / m_frameSize;
}

/* ---- Compute spectral features ---- */

FeatureExtractor4::SpectralFeatures FeatureExtractor4::computeSpectral(
    const QVector<double>& magnitude,
    const QVector<double>& prevMagnitude) const
{
    SpectralFeatures feat;
    int halfN = magnitude.size();
    if (halfN < 2) return feat;

    // Total energy for weighting
    double totalEnergy = 0.0;
    for (int k = 0; k < halfN; ++k)
        totalEnergy += magnitude[k] * magnitude[k];
    if (totalEnergy < 1e-15) return feat;

    // Spectral centroid: weighted mean frequency
    double weightedSum = 0.0;
    for (int k = 0; k < halfN; ++k)
        weightedSum += binToHz(k) * magnitude[k] * magnitude[k];
    feat.centroid = weightedSum / totalEnergy;

    // Spectral spread
    double spreadSum = 0.0;
    for (int k = 0; k < halfN; ++k) {
        double f = binToHz(k);
        spreadSum += (f - feat.centroid) * (f - feat.centroid) * magnitude[k] * magnitude[k];
    }
    feat.spread = qSqrt(spreadSum / totalEnergy);

    // Spectral rolloff (85% energy threshold)
    double threshold = 0.85 * totalEnergy;
    double cumEnergy = 0.0;
    for (int k = 0; k < halfN; ++k) {
        cumEnergy += magnitude[k] * magnitude[k];
        if (cumEnergy >= threshold) {
            feat.rolloff = binToHz(k);
            break;
        }
    }

    // Spectral flux (half-wave rectified difference)
    if (prevMagnitude.size() == halfN) {
        double fluxSum = 0.0;
        for (int k = 0; k < halfN; ++k) {
            double diff = magnitude[k] - prevMagnitude[k];
            fluxSum += diff * diff * (diff > 0 ? 1.0 : 0.0);
        }
        feat.flux = qSqrt(fluxSum) / halfN;
    }

    // Spectral flatness (geometric mean / arithmetic mean)
    double logSum = 0.0;
    double arithSum = 0.0;
    int nonzero = 0;
    for (int k = 1; k < halfN; ++k) {
        if (magnitude[k] > 1e-15) {
            logSum += qLn(magnitude[k]);
            arithSum += magnitude[k];
            nonzero++;
        }
    }
    if (nonzero > 0) {
        double geoMean = qExp(logSum / nonzero);
        double ariMean = arithSum / nonzero;
        feat.flatness = (ariMean > 1e-15) ? geoMean / ariMean : 0.0;
    }

    return feat;
}

/* ---- Compute statistical moments ---- */

FeatureExtractor4::StatMoments FeatureExtractor4::computeMoments(
    const QVector<double>& data) const
{
    StatMoments m;
    int n = data.size();
    if (n < 2) return m;

    // Mean
    double sum = 0.0;
    for (double v : data) sum += v;
    m.mean = sum / n;

    // Variance
    double varSum = 0.0;
    for (double v : data) {
        double d = v - m.mean;
        varSum += d * d;
    }
    m.variance = varSum / n;
    m.stdDev = qSqrt(m.variance);

    if (m.stdDev < 1e-15) return m;

    // Skewness (3rd moment)
    double skewSum = 0.0;
    double kurtSum = 0.0;
    for (double v : data) {
        double d = (v - m.mean) / m.stdDev;
        double d2 = d * d;
        skewSum += d2 * d;
        kurtSum += d2 * d2;
    }
    m.skewness = skewSum / n;
    m.kurtosis = kurtSum / n - 3.0; // excess kurtosis

    return m;
}

/* ---- Compute RMS ---- */

double FeatureExtractor4::computeRMS(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : frame) sum += v * v;
    return qSqrt(sum / frame.size());
}

/* ---- Compute ZCR ---- */

double FeatureExtractor4::computeZCR(const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;
    int crossings = 0;
    for (int i = 1; i < frame.size(); ++i)
        if ((frame[i] >= 0) != (frame[i - 1] >= 0)) crossings++;
    return static_cast<double>(crossings) / (frame.size() - 1);
}

/* ---- Extract single frame ---- */

FeatureExtractor4::FrameFeatures FeatureExtractor4::extractFrame(
    const QVector<double>& frame,
    const QVector<double>& prevSpectrum) const
{
    FrameFeatures ff;

    // Time-domain features
    ff.rms = computeRMS(frame);
    ff.zcr = computeZCR(frame);
    ff.timeDomain = computeMoments(frame);

    // Windowed DFT
    QVector<double> windowed = hannWindow(frame);
    QVector<double> mag = magnitudeSpectrum(windowed);

    // Spectral features
    ff.spectral = computeSpectral(mag, prevSpectrum);
    ff.freqDomain = computeMoments(mag);

    return ff;
}

/* ---- Extract all frames ---- */

QVector<FeatureExtractor4::FrameFeatures> FeatureExtractor4::extractAll(
    const QVector<double>& signal) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<FrameFeatures> results;
    int hopSize = m_frameSize / 2; // 50% overlap
    QVector<double> prevSpectrum;

    for (int start = 0; start + m_frameSize <= signal.size(); start += hopSize) {
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] = signal[start + i];

        FrameFeatures ff = extractFrame(frame, prevSpectrum);

        // Update prev spectrum
        QVector<double> windowed = hannWindow(frame);
        prevSpectrum = magnitudeSpectrum(windowed);

        results.append(ff);
    }

    m_stats.numFrames = results.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit extractionCompleted(results.size(), timer.elapsed());
    return results;
}

/* ---- Z-score normalize ---- */

QVector<QVector<double>> FeatureExtractor4::zscoreNormalize(
    const QVector<QVector<double>>& featureMatrix) const
{
    if (featureMatrix.isEmpty()) return {};
    int rows = featureMatrix.size();
    int cols = featureMatrix[0].size();

    // Compute per-column mean and std
    QVector<double> mean(cols, 0.0), stdDev(cols, 0.0);
    for (int j = 0; j < cols; ++j) {
        for (int i = 0; i < rows; ++i)
            mean[j] += featureMatrix[i][j];
        mean[j] /= rows;
    }
    for (int j = 0; j < cols; ++j) {
        for (int i = 0; i < rows; ++i) {
            double d = featureMatrix[i][j] - mean[j];
            stdDev[j] += d * d;
        }
        stdDev[j] = qSqrt(stdDev[j] / rows);
        if (stdDev[j] < 1e-15) stdDev[j] = 1.0;
    }

    QVector<QVector<double>> normalized(rows, QVector<double>(cols));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            normalized[i][j] = (featureMatrix[i][j] - mean[j]) / stdDev[j];

    return normalized;
}

/* ---- Reset ---- */

void FeatureExtractor4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
