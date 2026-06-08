/**
 * @file FeatureExtractor3.cpp
 * @brief FeatureExtractor3 实现
 *
 * 实现Mel频率特征提取：Mel滤波器组、Delta-Delta加速度系数流水线。
 */

#include "utils/signal214/FeatureExtractor3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FeatureExtractor3::FeatureExtractor3(QObject *parent) : QObject(parent) {}
FeatureExtractor3::~FeatureExtractor3() = default;

/* ---- Hz/Mel conversion ---- */

double FeatureExtractor3::hzToMel(double hz)
{
    return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0);
    // Using 2595 * log10(1 + hz/700)
}

double FeatureExtractor3::melToHz(double mel)
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/* ---- Initialize ---- */

void FeatureExtractor3::init(double sampleRate, int fftSize, int numMelBands)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_fftSize = qMax(4, fftSize);
    m_numMelBands = qMax(2, numMelBands);

    buildMelFilterBank();

    m_stats.fftSize = m_fftSize;
    m_stats.numMelBands = m_numMelBands;
    m_stats.numFeatures = m_numCoeffs * 3;  // MFCC + delta + delta-delta
}

/* ---- Build Mel filter bank ---- */

void FeatureExtractor3::buildMelFilterBank()
{
    double maxFreq = m_sampleRate / 2.0;
    double maxMel = hzToMel(maxFreq);

    // Mel points: numMelBands + 2 endpoints
    int numPoints = m_numMelBands + 2;
    QVector<double> melPoints(numPoints);
    for (int i = 0; i < numPoints; ++i)
        melPoints[i] = i * maxMel / (numPoints - 1);

    // Convert to FFT bin indices
    QVector<int> binPoints(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double hz = melToHz(melPoints[i]);
        binPoints[i] = qBound(0, static_cast<int>(hz / maxFreq * (m_fftSize / 2)),
                               m_fftSize / 2);
    }

    // Build triangular filters
    m_melFilters.resize(m_numMelBands);
    m_filterStart.resize(m_numMelBands);
    m_filterEnd.resize(m_numMelBands);
    int halfFFT = m_fftSize / 2 + 1;

    for (int m = 0; m < m_numMelBands; ++m) {
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];

        m_filterStart[m] = left;
        m_filterEnd[m] = right;

        m_melFilters[m].fill(0.0, halfFFT);

        // Rising slope: left -> center
        if (center > left) {
            for (int k = left; k <= center && k < halfFFT; ++k)
                m_melFilters[m][k] = static_cast<double>(k - left) / (center - left);
        }
        // Falling slope: center -> right
        if (right > center) {
            for (int k = center; k <= right && k < halfFFT; ++k)
                m_melFilters[m][k] = static_cast<double>(right - k) / (right - center);
        }
    }
}

/* ---- Hamming window ---- */

QVector<double> FeatureExtractor3::applyHamming(const QVector<double>& frame)
{
    int N = frame.size();
    QVector<double> windowed(N);
    for (int n = 0; n < N; ++n)
        windowed[n] = frame[n] * (0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1)));
    return windowed;
}

/* ---- Power spectrum ---- */

QVector<double> FeatureExtractor3::powerSpectrum(const QVector<double>& frame) const
{
    int N = qMin(frame.size(), m_fftSize);
    auto windowed = applyHamming(frame);

    // Simple DFT magnitude squared (no FFT optimization for clarity)
    int halfN = m_fftSize / 2 + 1;
    QVector<double> power(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / m_fftSize;
            re += windowed[n] * qCos(angle);
            im -= windowed[n] * qSin(angle);
        }
        power[k] = (re * re + im * im) / m_fftSize;
    }
    return power;
}

/* ---- Mel filter energies ---- */

QVector<double> FeatureExtractor3::melFilterEnergies(const QVector<double>& powerSpec) const
{
    QVector<double> energies(m_numMelBands, 0.0);
    int halfN = qMin(powerSpec.size(), m_fftSize / 2 + 1);

    for (int m = 0; m < m_numMelBands; ++m) {
        double sum = 0.0;
        int len = qMin(m_melFilters[m].size(), halfN);
        for (int k = 0; k < len; ++k)
            sum += powerSpec[k] * m_melFilters[m][k];
        energies[m] = qMax(sum, 1e-10);  // Floor to avoid log(0)
    }
    return energies;
}

/* ---- DCT of Mel energies (MFCC) ---- */

QVector<double> FeatureExtractor3::dctMelCoefficients(const QVector<double>& melEnergies,
                                                        int numCoeffs) const
{
    int N = melEnergies.size();
    QVector<double> logEnergies(N);
    for (int i = 0; i < N; ++i)
        logEnergies[i] = qLn(melEnergies[i]);

    int nc = qMin(numCoeffs, N);
    QVector<double> coeffs(nc, 0.0);

    // Type-II DCT
    for (int k = 0; k < nc; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n)
            sum += logEnergies[n] * qCos(M_PI * k * (2 * n + 1) / (2.0 * N));
        coeffs[k] = sum;
    }
    return coeffs;
}

/* ---- Delta computation ---- */

QVector<QVector<double>> FeatureExtractor3::computeDelta(
    const QVector<QVector<double>>& features, int N)
{
    int numFrames = features.size();
    if (numFrames == 0) return {};
    int dim = features[0].size();
    QVector<QVector<double>> delta(numFrames, QVector<double>(dim, 0.0));

    double norm = 0.0;
    for (int n = 1; n <= N; ++n) norm += n * n;
    norm *= 2.0;

    for (int t = 0; t < numFrames; ++t) {
        for (int d = 0; d < dim; ++d) {
            double sum = 0.0;
            for (int n = 1; n <= N; ++n) {
                int tp = qMin(t + n, numFrames - 1);
                int tm = qMax(t - n, 0);
                sum += n * (features[tp][d] - features[tm][d]);
            }
            delta[t][d] = sum / norm;
        }
    }
    return delta;
}

/* ---- Extract single frame ---- */

FeatureExtractor3::FeatureFrame FeatureExtractor3::extractFrame(
    const QVector<double>& frame) const
{
    QElapsedTimer timer;
    timer.start();

    FeatureFrame ff;
    auto ps = powerSpectrum(frame);
    ff.melEnergies = melFilterEnergies(ps);
    ff.melCoefficients = dctMelCoefficients(ff.melEnergies, m_numCoeffs);
    // Delta/delta-delta require sequence context, leave empty for single frame

    auto self = const_cast<FeatureExtractor3*>(this);
    self->m_stats.totalFrames++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    return ff;
}

/* ---- Extract sequence ---- */

QVector<FeatureExtractor3::FeatureFrame> FeatureExtractor3::extractSequence(
    const QVector<QVector<double>>& frames) const
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = frames.size();
    QVector<FeatureFrame> result(numFrames);

    // Step 1: Compute Mel energies and MFCC for each frame
    QVector<QVector<double>> allCoeffs(numFrames);
    for (int t = 0; t < numFrames; ++t) {
        auto ps = powerSpectrum(frames[t]);
        result[t].melEnergies = melFilterEnergies(ps);
        result[t].melCoefficients = dctMelCoefficients(result[t].melEnergies, m_numCoeffs);
        allCoeffs[t] = result[t].melCoefficients;
    }

    // Step 2: Compute delta coefficients
    auto deltas = computeDelta(allCoeffs, 2);
    auto deltaDeltas = computeDelta(deltas, 2);

    for (int t = 0; t < numFrames; ++t) {
        result[t].delta = deltas[t];
        result[t].deltaDelta = deltaDeltas[t];
    }

    auto self = const_cast<FeatureExtractor3*>(this);
    self->m_stats.totalFrames += numFrames;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;
    self->emit extractionCompleted(numFrames, m_numCoeffs * 3, timer.elapsed());

    return result;
}

/* ---- Get Mel filter bank ---- */

QVector<QVector<double>> FeatureExtractor3::melFilterBank() const
{
    return m_melFilters;
}

/* ---- Reset ---- */

void FeatureExtractor3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_melFilters.clear();
    m_filterStart.clear();
    m_filterEnd.clear();
}
