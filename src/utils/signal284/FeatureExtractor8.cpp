/**
 * @file FeatureExtractor8.cpp
 * @brief FeatureExtractor8 实现
 *
 * 实现特征提取器：谱质心与谱通量特征及过零率的音频内容分类。
 */

#include "utils/signal284/FeatureExtractor8.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

FeatureExtractor8::FeatureExtractor8(QObject *parent)
    : QObject(parent)
{
    precomputeWindow();
}

FeatureExtractor8::~FeatureExtractor8() = default;

/* ---- Configuration ---- */

void FeatureExtractor8::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
    m_stats.frameSize = m_frameSize;
    precomputeWindow();
}

void FeatureExtractor8::setSampleRate(int rate)
{
    m_sampleRate = qMax(8000, rate);
    m_stats.sampleRate = m_sampleRate;
}

void FeatureExtractor8::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/* ---- Precompute Hann window ---- */

void FeatureExtractor8::precomputeWindow()
{
    m_window.resize(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_frameSize - 1)));
}

/* ---- Compute magnitude spectrum via DFT ---- */

QVector<double> FeatureExtractor8::computeMagnitude(const QVector<double>& frame) const
{
    int n = frame.size();
    int halfN = n / 2 + 1;
    QVector<double> magnitude(halfN, 0.0);

    // DFT for real input: compute only positive frequencies
    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            re += frame[i] * qCos(angle);
            im -= frame[i] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
    }
    return magnitude;
}

/* ---- Spectral centroid ---- */

double FeatureExtractor8::spectralCentroid(const QVector<double>& magnitude) const
{
    int n = magnitude.size();
    double weightedSum = 0.0;
    double totalMag = 0.0;

    for (int k = 0; k < n; ++k) {
        double freq = k * m_sampleRate / (2.0 * (n - 1));
        weightedSum += freq * magnitude[k];
        totalMag += magnitude[k];
    }
    return (totalMag > 1e-15) ? weightedSum / totalMag : 0.0;
}

/* ---- Spectral flux ---- */

double FeatureExtractor8::spectralFlux(const QVector<double>& magCurr,
                                         const QVector<double>& magPrev) const
{
    int n = qMin(magCurr.size(), magPrev.size());
    double flux = 0.0;
    for (int k = 0; k < n; ++k) {
        double diff = magCurr[k] - magPrev[k];
        flux += diff * diff;  // Half-wave rectified: only positive changes
    }
    return qSqrt(flux / n);
}

/* ---- Zero-crossing rate ---- */

double FeatureExtractor8::zeroCrossingRate(const QVector<double>& frame) const
{
    int n = frame.size();
    if (n < 2) return 0.0;

    int crossings = 0;
    for (int i = 1; i < n; ++i) {
        if ((frame[i] >= 0.0 && frame[i - 1] < 0.0) ||
            (frame[i] < 0.0 && frame[i - 1] >= 0.0))
            crossings++;
    }
    return static_cast<double>(crossings) / (n - 1);
}

/* ---- RMS energy ---- */

double FeatureExtractor8::rmsEnergy(const QVector<double>& frame) const
{
    int n = frame.size();
    if (n == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < n; ++i) sum += frame[i] * frame[i];
    return qSqrt(sum / n);
}

/* ---- Spectral rolloff ---- */

double FeatureExtractor8::spectralRolloff(const QVector<double>& magnitude) const
{
    int n = magnitude.size();
    double totalEnergy = 0.0;
    for (int k = 0; k < n; ++k) totalEnergy += magnitude[k] * magnitude[k];

    double threshold = 0.85 * totalEnergy;
    double cumEnergy = 0.0;
    for (int k = 0; k < n; ++k) {
        cumEnergy += magnitude[k] * magnitude[k];
        if (cumEnergy >= threshold)
            return k * m_sampleRate / (2.0 * (n - 1));
    }
    return m_sampleRate / 2.0;
}

/* ---- Spectral bandwidth ---- */

double FeatureExtractor8::spectralBandwidth(const QVector<double>& magnitude,
                                               double centroid) const
{
    int n = magnitude.size();
    double totalMag = 0.0;
    double weightedVar = 0.0;

    for (int k = 0; k < n; ++k) {
        double freq = k * m_sampleRate / (2.0 * (n - 1));
        double diff = freq - centroid;
        weightedVar += magnitude[k] * diff * diff;
        totalMag += magnitude[k];
    }
    return (totalMag > 1e-15) ? qSqrt(weightedVar / totalMag) : 0.0;
}

/* ---- Extract features from entire audio signal ---- */

FeatureExtractor8::ExtractionResult FeatureExtractor8::extract(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    ExtractionResult result;
    int totalSamples = audio.size();
    if (totalSamples < m_frameSize) return result;

    int numFrames = (totalSamples - m_frameSize) / m_hopSize + 1;
    QVector<double> prevMagnitude;

    FeatureVector globalSum;
    int validFrames = 0;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        // Extract and window frame
        QVector<double> frame(m_frameSize, 0.0);
        for (int i = 0; i < m_frameSize && start + i < totalSamples; ++i)
            frame[i] = audio[start + i] * m_window[i];

        // Compute magnitude spectrum
        QVector<double> magnitude = computeMagnitude(frame);

        // Extract features
        FeatureVector feat;
        feat.spectralCentroid = spectralCentroid(magnitude);
        feat.spectralFlux = prevMagnitude.isEmpty() ? 0.0
                             : spectralFlux(magnitude, prevMagnitude);
        feat.zeroCrossingRate = zeroCrossingRate(frame);
        feat.rmsEnergy = rmsEnergy(frame);
        feat.spectralRolloff = spectralRolloff(magnitude);
        feat.bandwidth = spectralBandwidth(magnitude, feat.spectralCentroid);

        result.frameFeatures.append(feat);
        prevMagnitude = magnitude;

        // Accumulate for global features
        globalSum.spectralCentroid += feat.spectralCentroid;
        globalSum.spectralFlux += feat.spectralFlux;
        globalSum.zeroCrossingRate += feat.zeroCrossingRate;
        globalSum.rmsEnergy += feat.rmsEnergy;
        globalSum.spectralRolloff += feat.spectralRolloff;
        globalSum.bandwidth += feat.bandwidth;
        validFrames++;

        double elapsed = timer.elapsed();
        emit frameDone(f, feat.spectralCentroid, feat.zeroCrossingRate, elapsed);
    }

    // Compute global averages
    if (validFrames > 0) {
        result.globalFeatures.spectralCentroid = globalSum.spectralCentroid / validFrames;
        result.globalFeatures.spectralFlux = globalSum.spectralFlux / validFrames;
        result.globalFeatures.zeroCrossingRate = globalSum.zeroCrossingRate / validFrames;
        result.globalFeatures.rmsEnergy = globalSum.rmsEnergy / validFrames;
        result.globalFeatures.spectralRolloff = globalSum.spectralRolloff / validFrames;
        result.globalFeatures.bandwidth = globalSum.bandwidth / validFrames;
    }
    result.numFrames = validFrames;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit extractionDone(validFrames, result.globalFeatures.spectralCentroid, elapsed);

    return result;
}

/* ---- Reset ---- */

void FeatureExtractor8::resetStatistics()
{
    m_stats = Stats{};
    m_stats.frameSize = m_frameSize;
    m_stats.sampleRate = m_sampleRate;
    m_timeSum = 0.0;
}
