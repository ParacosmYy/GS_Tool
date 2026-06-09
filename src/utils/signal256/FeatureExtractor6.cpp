/**
 * @file FeatureExtractor6.cpp
 * @brief FeatureExtractor6 实现
 *
 * 实现特征提取：MFCC梅尔频率倒谱系数与delta/delta-delta导数语音特征。
 */

#include "utils/signal256/FeatureExtractor6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FeatureExtractor6::FeatureExtractor6(int sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(sampleRate)
{
    m_stats.sampleRate = sampleRate;
    buildMelFilterbank();
}
FeatureExtractor6::~FeatureExtractor6() = default;

/* ---- Configuration ---- */

void FeatureExtractor6::setFftSize(int size)
{
    m_fftSize = qMax(64, size);
    m_stats.fftSize = m_fftSize;
    buildMelFilterbank();
}

void FeatureExtractor6::setMelBins(int bins)
{
    m_melBins = qMax(4, bins);
    m_stats.numMelBins = m_melBins;
    buildMelFilterbank();
}

void FeatureExtractor6::setMfccCount(int count)
{
    m_mfccCount = qMax(1, count);
    m_stats.numMfcc = m_mfccCount;
}

/* ---- Hz <-> Mel conversion ---- */

double FeatureExtractor6::hzToMel(double hz) const
{
    return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0);
}

double FeatureExtractor6::melToHz(double mel) const
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/* ---- Build mel filter bank ---- */

void FeatureExtractor6::buildMelFilterbank()
{
    int fftBins = m_fftSize / 2 + 1;
    double maxFreq = m_sampleRate / 2.0;
    double maxMel = hzToMel(maxFreq);

    // Mel-spaced center frequencies
    QVector<double> melPoints(m_melBins + 2);
    for (int i = 0; i < m_melBins + 2; ++i)
        melPoints[i] = i * maxMel / (m_melBins + 1);

    // Convert to FFT bin indices
    QVector<int> binPoints(m_melBins + 2);
    for (int i = 0; i < m_melBins + 2; ++i) {
        double hz = melToHz(melPoints[i]);
        binPoints[i] = qBound(0, static_cast<int>(hz / maxFreq * (fftBins - 1)),
                               fftBins - 1);
    }

    // Build triangular filters
    m_melFilterbank.resize(m_melBins);
    for (int m = 0; m < m_melBins; ++m) {
        m_melFilterbank[m].resize(fftBins, 0.0);
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];

        // Rising slope
        for (int k = left; k <= center; ++k) {
            if (center > left)
                m_melFilterbank[m][k] = static_cast<double>(k - left) / (center - left);
        }
        // Falling slope
        for (int k = center; k <= right; ++k) {
            if (right > center)
                m_melFilterbank[m][k] = static_cast<double>(right - k) / (right - center);
        }
    }
}

/* ---- Hamming window ---- */

QVector<double> FeatureExtractor6::hammingWindow(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i)
        windowed[i] = frame[i] * (0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1)));
    return windowed;
}

/* ---- Power spectrum ---- */

QVector<double> FeatureExtractor6::powerSpectrum(const QVector<double>& frame) const
{
    int n = frame.size();
    int half = n / 2 + 1;
    QVector<double> power(half, 0.0);

    // Simple DFT for power spectrum (magnitude squared)
    for (int k = 0; k < half; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            re += frame[i] * qCos(angle);
            im -= frame[i] * qSin(angle);
        }
        power[k] = (re * re + im * im) / n;
    }
    return power;
}

/* ---- Apply mel filter bank ---- */

QVector<double> FeatureExtractor6::applyFilterbank(const QVector<double>& spectrum) const
{
    QVector<double> melEnergies(m_melBins, 0.0);
    for (int m = 0; m < m_melBins; ++m) {
        double sum = 0.0;
        for (int k = 0; k < spectrum.size(); ++k)
            sum += spectrum[k] * m_melFilterbank[m][k];
        melEnergies[m] = qMax(sum, 1e-10);  // Floor to avoid log(0)
    }
    return melEnergies;
}

/* ---- DCT-II ---- */

QVector<double> FeatureExtractor6::dctII(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> output(m_mfccCount, 0.0);
    for (int k = 0; k < m_mfccCount; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            sum += input[i] * qCos(M_PI * k * (2 * i + 1) / (2.0 * n));
        output[k] = sum;
    }
    return output;
}

/* ---- Extract MFCC features ---- */

QVector<QVector<double>> FeatureExtractor6::extract(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    int frameSize = m_fftSize;
    int hopSize = frameSize / 2;
    if (audio.size() < frameSize) return {};

    // Framing
    QVector<QVector<double>> features;
    for (int start = 0; start + frameSize <= audio.size(); start += hopSize) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i)
            frame[i] = audio[start + i];

        // Pre-emphasis
        for (int i = frameSize - 1; i > 0; --i)
            frame[i] -= 0.97 * frame[i - 1];

        // Windowing
        QVector<double> windowed = hammingWindow(frame);

        // Power spectrum
        QVector<double> power = powerSpectrum(windowed);

        // Mel filter bank
        QVector<double> melEnergies = applyFilterbank(power);

        // Log mel energies
        for (auto& v : melEnergies) v = qLn(v);

        // DCT -> MFCC
        QVector<double> mfcc = dctII(melEnergies);
        features.append(mfcc);
    }

    m_stats.numFrames = features.size();
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit extractionCompleted(features.size(), m_mfccCount, elapsed);
    return features;
}

/* ---- Delta features ---- */

QVector<QVector<double>> FeatureExtractor6::delta(
    const QVector<QVector<double>>& features, int n) const
{
    if (features.isEmpty()) return {};
    int numFrames = features.size();
    int numCoeffs = features[0].size();
    QVector<QVector<double>> deltaFeatures(numFrames, QVector<double>(numCoeffs, 0.0));

    for (int t = 0; t < numFrames; ++t) {
        for (int c = 0; c < numCoeffs; ++c) {
            double sum = 0.0;
            double norm = 0.0;
            for (int k = -n; k <= n; ++k) {
                int idx = qBound(0, t + k, numFrames - 1);
                sum += k * features[idx][c];
                norm += k * k;
            }
            deltaFeatures[t][c] = (norm > 0) ? sum / norm : 0.0;
        }
    }
    return deltaFeatures;
}

/* ---- Reset ---- */

void FeatureExtractor6::resetStatistics()
{
    m_melFilterbank.clear();
    m_stats = Stats{};
    m_stats.sampleRate = m_sampleRate;
    m_timeSum = 0.0;
    buildMelFilterbank();
}
