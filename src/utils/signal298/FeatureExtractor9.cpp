/**
 * @file FeatureExtractor9.cpp
 * @brief FeatureExtractor9 实现
 *
 * 实现特征提取：MFCC梅尔频率倒谱系数计算与Delta/加速度导数实现音频指纹。
 */

#include "utils/signal298/FeatureExtractor9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FeatureExtractor9::FeatureExtractor9(QObject *parent)
    : QObject(parent)
{
    buildFilterBank();
    buildDCT();
}

FeatureExtractor9::~FeatureExtractor9() = default;

/* ---- Configuration ---- */

void FeatureExtractor9::setSampleRate(int rate) { m_sampleRate = qBound(8000, rate, 96000); buildFilterBank(); }
void FeatureExtractor9::setNumCoeffs(int n) { m_numCoeffs = qBound(1, n, 40); buildDCT(); }
void FeatureExtractor9::setNumFilters(int n) { m_numFilters = qBound(8, n, 80); buildFilterBank(); }

/* ---- Mel scale conversions ---- */

double FeatureExtractor9::freqToMel(double f) const { return 2595.0 * qLn(1.0 + f / 700.0) / qLn(2.0); }
double FeatureExtractor9::melToFreq(double m) const { return 700.0 * (qPow(2.0, m / 2595.0) - 1.0); }

/* ---- Build mel-spaced triangular filterbank ---- */

void FeatureExtractor9::buildFilterBank()
{
    int fftSize = 512;
    m_filterBank.resize(m_numFilters);
    double maxMel = freqToMel(m_sampleRate / 2.0);

    // Linearly spaced mel points
    QVector<double> melPoints(m_numFilters + 2);
    for (int i = 0; i < m_numFilters + 2; ++i)
        melPoints[i] = i * maxMel / (m_numFilters + 1);

    // Convert back to FFT bin indices
    QVector<int> binPoints(m_numFilters + 2);
    for (int i = 0; i < m_numFilters + 2; ++i)
        binPoints[i] = qFloor((fftSize + 1) * melToFreq(melPoints[i]) / m_sampleRate);

    for (int f = 0; f < m_numFilters; ++f) {
        m_filterBank[f].resize(fftSize / 2 + 1, 0.0);
        int start = binPoints[f];
        int center = binPoints[f + 1];
        int end = binPoints[f + 2];
        for (int i = start; i <= center && i < m_filterBank[f].size(); ++i) {
            if (i >= 0 && center > start)
                m_filterBank[f][i] = static_cast<double>(i - start) / (center - start);
        }
        for (int i = center; i <= end && i < m_filterBank[f].size(); ++i) {
            if (i >= 0 && end > center)
                m_filterBank[f][i] = static_cast<double>(end - i) / (end - center);
        }
    }
}

/* ---- Build DCT-II matrix ---- */

void FeatureExtractor9::buildDCT()
{
    m_dctMatrix.resize(m_numCoeffs);
    for (int i = 0; i < m_numCoeffs; ++i) {
        m_dctMatrix[i].resize(m_numFilters);
        for (int j = 0; j < m_numFilters; ++j)
            m_dctMatrix[i][j] = qCos(M_PI * i * (j + 0.5) / m_numFilters);
    }
}

/* ---- Apply Hamming window ---- */

QVector<double> FeatureExtractor9::applyWindow(const QVector<double>& frame) const
{
    int N = frame.size();
    QVector<double> windowed(N, 0.0);
    for (int i = 0; i < N; ++i) {
        double w = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (N - 1));
        windowed[i] = frame[i] * w;
    }
    return windowed;
}

/* ---- Compute power spectrum ---- */

QVector<double> FeatureExtractor9::powerSpectrum(const QVector<double>& frame) const
{
    int N = frame.size();
    int fftSize = 512;
    // DFT for first fftSize/2+1 bins
    QVector<double> psd(fftSize / 2 + 1, 0.0);
    for (int k = 0; k <= fftSize / 2; ++k) {
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N && n < fftSize; ++n) {
            double angle = 2.0 * M_PI * k * n / fftSize;
            real += frame[n] * qCos(angle);
            imag -= frame[n] * qSin(angle);
        }
        psd[k] = (real * real + imag * imag) / fftSize;
    }
    return psd;
}

/* ---- Compute delta coefficients ---- */

QVector<double> FeatureExtractor9::computeDelta(
    const QVector<QVector<double>>& features, int n) const
{
    if (features.size() < 2 * n + 1) return QVector<double>(features.isEmpty() ? 0 : features[0].size(), 0.0);
    int dim = features[0].size();
    QVector<double> delta(dim, 0.0);
    double sumW = 0.0;
    for (int t = -n; t <= n; ++t) sumW += t * t;
    for (int d = 0; d < dim; ++d) {
        for (int t = -n; t <= n; ++t) {
            int idx = qBound(0, static_cast<int>(features.size() / 2) + t,
                             features.size() - 1);
            delta[d] += t * features[idx][d];
        }
        delta[d] /= (2.0 * sumW);
    }
    return delta;
}

/* ---- Extract features from single frame ---- */

FeatureExtractor9::FeatureVector FeatureExtractor9::extract(
    const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    FeatureVector fv;

    // Step 1: Pre-emphasis and windowing
    auto windowed = applyWindow(frame);

    // Step 2: Power spectrum
    auto psd = powerSpectrum(windowed);

    // Step 3: Apply mel filterbank
    QVector<double> melEnergies(m_numFilters, 0.0);
    for (int f = 0; f < m_numFilters; ++f) {
        double sum = 0.0;
        for (int i = 0; i < qMin(psd.size(), m_filterBank[f].size()); ++i)
            sum += psd[i] * m_filterBank[f][i];
        melEnergies[f] = qMax(sum, 1e-10);  // Floor to avoid log(0)
    }

    // Step 4: Log mel energies
    QVector<double> logMel(m_numFilters);
    double energy = 0.0;
    for (int i = 0; i < m_numFilters; ++i) {
        logMel[i] = qLn(melEnergies[i]);
        energy += melEnergies[i];
    }
    fv.energy = energy;

    // Step 5: DCT to get MFCCs
    fv.mfcc.resize(m_numCoeffs);
    for (int i = 0; i < m_numCoeffs; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_numFilters; ++j)
            sum += m_dctMatrix[i][j] * logMel[j];
        fv.mfcc[i] = sum;
    }

    // Delta and acceleration require multi-frame context
    // For single frame, zero-filled
    fv.delta.resize(m_numCoeffs, 0.0);
    fv.acceleration.resize(m_numCoeffs, 0.0);

    double elapsed = timer.elapsed();
    m_stats.frameSize = frame.size();
    m_stats.numCoeffs = m_numCoeffs;
    m_stats.totalExtractions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalExtractions;

    emit extractionDone(m_numCoeffs, energy, elapsed);
    return fv;
}

/* ---- Extract features from multiple frames ---- */

QVector<FeatureExtractor9::FeatureVector> FeatureExtractor9::extractMulti(
    const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = frames.size();
    QVector<FeatureVector> result(numFrames);

    // Extract base MFCCs for all frames
    QVector<QVector<double>> allMfcc(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        result[f] = extract(frames[f]);
        allMfcc[f] = result[f].mfcc;
    }

    // Compute delta coefficients (N=2 regression)
    int N = qMin(2, numFrames / 2);
    for (int f = 0; f < numFrames; ++f) {
        QVector<QVector<double>> window;
        for (int t = -N; t <= N; ++t) {
            int idx = qBound(0, f + t, numFrames - 1);
            window.append(allMfcc[idx]);
        }
        result[f].delta = computeDelta(window, N);
    }

    // Compute acceleration (delta of delta)
    QVector<QVector<double>> allDelta(numFrames);
    for (int f = 0; f < numFrames; ++f)
        allDelta[f] = result[f].delta;

    for (int f = 0; f < numFrames; ++f) {
        QVector<QVector<double>> window;
        for (int t = -N; t <= N; ++t) {
            int idx = qBound(0, f + t, numFrames - 1);
            window.append(allDelta[idx]);
        }
        result[f].acceleration = computeDelta(window, N);
    }

    double elapsed = timer.elapsed();
    emit extractionDone(m_numCoeffs, 0.0, elapsed);
    return result;
}

/* ---- Reset ---- */

void FeatureExtractor9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
