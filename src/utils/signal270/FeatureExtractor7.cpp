/**
 * @file FeatureExtractor7.cpp
 * @brief FeatureExtractor7 实现
 *
 * 实现特征提取：MFCC梅尔频率倒谱系数与Delta/Delta-Delta导数音频指纹。
 */

#include "utils/signal270/FeatureExtractor7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FeatureExtractor7::FeatureExtractor7(QObject *parent)
    : QObject(parent) {}

FeatureExtractor7::~FeatureExtractor7() = default;

/* ---- Configuration ---- */

void FeatureExtractor7::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
}

void FeatureExtractor7::setNumCoeffs(int num)
{
    m_numCoeffs = qMax(1, num);
}

void FeatureExtractor7::setNumMelBins(int num)
{
    m_numMelBins = qMax(4, num);
}

void FeatureExtractor7::setSampleRate(int rate)
{
    m_sampleRate = qMax(8000, rate);
}

/* ---- Hz <-> Mel conversions ---- */

double FeatureExtractor7::hzToMel(double hz)
{
    return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0);
}

double FeatureExtractor7::melToHz(double mel)
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/* ---- Hamming window ---- */

QVector<double> FeatureExtractor7::hammingWindow(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i) {
        double w = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1));
        windowed[i] = frame[i] * w;
    }
    return windowed;
}

/* ---- Power spectrum via DFT ---- */

QVector<double> FeatureExtractor7::powerSpectrum(const QVector<double>& frame) const
{
    int N = frame.size();
    int halfN = N / 2 + 1;
    QVector<double> power(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im -= frame[n] * qSin(angle);
        }
        power[k] = (re * re + im * im) / N;
    }
    return power;
}

/* ---- Build Mel filter bank ---- */

QVector<QVector<double>> FeatureExtractor7::buildMelFilterBank(int fftSize) const
{
    int halfN = fftSize / 2 + 1;
    double maxFreq = static_cast<double>(m_sampleRate) / 2.0;
    double maxMel = hzToMel(maxFreq);

    // Mel bin edges
    QVector<double> melEdges(m_numMelBins + 2);
    for (int i = 0; i < m_numMelBins + 2; ++i)
        melEdges[i] = melToHz(maxMel * i / (m_numMelBins + 1));

    // Build triangular filters
    QVector<QVector<double>> filterBank(m_numMelBins,
                                         QVector<double>(halfN, 0.0));
    for (int m = 0; m < m_numMelBins; ++m) {
        double fLeft = melEdges[m];
        double fCenter = melEdges[m + 1];
        double fRight = melEdges[m + 2];

        for (int k = 0; k < halfN; ++k) {
            double freq = static_cast<double>(k) * m_sampleRate / fftSize;
            if (freq >= fLeft && freq <= fCenter && fCenter > fLeft)
                filterBank[m][k] = (freq - fLeft) / (fCenter - fLeft);
            else if (freq > fCenter && freq <= fRight && fRight > fCenter)
                filterBank[m][k] = (fRight - freq) / (fRight - fCenter);
        }
    }
    return filterBank;
}

/* ---- DCT-II ---- */

QVector<double> FeatureExtractor7::dctII(const QVector<double>& input) const
{
    int N = input.size();
    int numOut = qMin(m_numCoeffs, N);
    QVector<double> result(numOut);

    for (int k = 0; k < numOut; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += input[n] * qCos(M_PI * k * (2 * n + 1) / (2.0 * N));
        }
        result[k] = sum;
    }
    return result;
}

/* ---- Frame audio into overlapping windows ---- */

QVector<QVector<double>> FeatureExtractor7::frameAudio(
    const QVector<double>& audio, int hopSize) const
{
    QVector<QVector<double>> frames;
    int pos = 0;
    while (pos + m_frameSize <= audio.size()) {
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] = audio[pos + i];
        frames.append(frame);
        pos += hopSize;
    }
    return frames;
}

/* ---- Extract MFCC ---- */

QVector<QVector<double>> FeatureExtractor7::extractMFCC(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    int hopSize = m_frameSize / 2;
    QVector<QVector<double>> frames = frameAudio(audio, hopSize);
    auto melBank = buildMelFilterBank(m_frameSize);

    QVector<QVector<double>> mfcc;
    for (const auto& rawFrame : frames) {
        // Apply Hamming window
        QVector<double> windowed = hammingWindow(rawFrame);

        // Compute power spectrum
        QVector<double> power = powerSpectrum(windowed);

        // Apply Mel filter bank
        QVector<double> melEnergies(m_numMelBins, 0.0);
        for (int m = 0; m < m_numMelBins; ++m) {
            for (int k = 0; k < power.size(); ++k)
                melEnergies[m] += power[k] * melBank[m][k];
            // Log energy (add epsilon to avoid log(0))
            melEnergies[m] = qLn(qMax(melEnergies[m], 1e-10));
        }

        // DCT to get cepstral coefficients
        QVector<double> coeffs = dctII(melEnergies);
        mfcc.append(coeffs);
    }

    double elapsed = timer.elapsed();
    m_stats.frameSize = m_frameSize;
    m_stats.numFrames = mfcc.size();
    m_stats.numCoeffs = m_numCoeffs;
    m_stats.numMelBins = m_numMelBins;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit extractionCompleted(mfcc.size(), m_numCoeffs, elapsed);

    return mfcc;
}

/* ---- Compute delta features ---- */

QVector<QVector<double>> FeatureExtractor7::computeDelta(
    const QVector<QVector<double>>& features, int n) const
{
    int numFrames = features.size();
    if (numFrames == 0) return {};

    int numCoeffs = features[0].size();
    QVector<QVector<double>> delta(numFrames, QVector<double>(numCoeffs, 0.0));

    double norm = 0.0;
    for (int k = 1; k <= n; ++k) norm += 2.0 * k * k;
    if (norm > 0.0) norm = 1.0 / norm;

    for (int t = 0; t < numFrames; ++t) {
        for (int c = 0; c < numCoeffs; ++c) {
            double sum = 0.0;
            for (int k = 1; k <= n; ++k) {
                int prev = qMax(0, t - k);
                int next = qMin(numFrames - 1, t + k);
                sum += k * (features[next][c] - features[prev][c]);
            }
            delta[t][c] = sum * norm;
        }
    }
    return delta;
}

/* ---- Compute delta-delta features ---- */

QVector<QVector<double>> FeatureExtractor7::computeDeltaDelta(
    const QVector<QVector<double>>& features, int n) const
{
    return computeDelta(computeDelta(features, n), n);
}

/* ---- Full features: MFCC + delta + delta-delta ---- */

QVector<QVector<double>> FeatureExtractor7::extractFullFeatures(const QVector<double>& audio)
{
    QVector<QVector<double>> mfcc = extractMFCC(audio);
    QVector<QVector<double>> delta = computeDelta(mfcc);
    QVector<QVector<double>> dd = computeDeltaDelta(mfcc);

    // Concatenate: [mfcc, delta, delta-delta]
    int numFrames = mfcc.size();
    if (numFrames == 0) return {};

    int dim = mfcc[0].size();
    QVector<QVector<double>> full(numFrames, QVector<double>(dim * 3));
    for (int t = 0; t < numFrames; ++t) {
        for (int c = 0; c < dim; ++c) {
            full[t][c] = mfcc[t][c];
            full[t][c + dim] = (t < delta.size()) ? delta[t][c] : 0.0;
            full[t][c + 2 * dim] = (t < dd.size()) ? dd[t][c] : 0.0;
        }
    }
    return full;
}

/* ---- Reset ---- */

void FeatureExtractor7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
