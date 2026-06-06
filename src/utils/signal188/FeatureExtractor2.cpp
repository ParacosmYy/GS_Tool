/**
 * @file FeatureExtractor2.cpp
 * @brief FeatureExtractor2 实现
 *
 * 实现MFCC特征提取：梅尔滤波器组、DCT倒谱、Delta/Delta-Delta差分特征。
 */

#include "utils/signal188/FeatureExtractor2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FeatureExtractor2::FeatureExtractor2(QObject *parent) : QObject(parent)
{
    buildMelFilterBank();
}

FeatureExtractor2::~FeatureExtractor2() = default;

/* ---- Configuration ---- */

void FeatureExtractor2::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); buildMelFilterBank(); }
void FeatureExtractor2::setNumCoeffs(int n) { m_numCoeffs = qMax(1, n); }
void FeatureExtractor2::setNumMelBins(int n) { m_numMelBins = qMax(1, n); buildMelFilterBank(); }
void FeatureExtractor2::setFrameSize(int size) { m_frameSize = qMax(4, size); buildMelFilterBank(); }
void FeatureExtractor2::setHopSize(int hop) { m_hopSize = qMax(1, hop); }

/* ---- Hz <-> Mel conversions ---- */

double FeatureExtractor2::hzToMel(double hz) const { return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0); }
double FeatureExtractor2::melToHz(double mel) const { return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0); }

/* ---- Build mel filter bank ---- */

void FeatureExtractor2::buildMelFilterBank()
{
    int fftSize = m_frameSize / 2 + 1;
    m_melWeights = QVector<QVector<double>>(m_numMelBins, QVector<double>(fftSize, 0.0));

    double melLow = hzToMel(0.0);
    double melHigh = hzToMel(m_sampleRate / 2.0);

    // Linearly space mel frequencies
    QVector<double> melPoints(m_numMelBins + 2);
    for (int i = 0; i < m_numMelBins + 2; ++i)
        melPoints[i] = melLow + (melHigh - melLow) * i / (m_numMelBins + 1);

    // Convert back to Hz -> FFT bin indices
    QVector<int> binPoints(m_numMelBins + 2);
    for (int i = 0; i < m_numMelBins + 2; ++i) {
        double hz = melToHz(melPoints[i]);
        binPoints[i] = qBound(0, (int)(hz * m_frameSize / m_sampleRate), fftSize - 1);
    }

    // Build triangular filters
    for (int m = 0; m < m_numMelBins; ++m) {
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];
        if (center == left) center = left + 1;
        if (right == center) right = center + 1;

        for (int k = left; k < center && k < fftSize; ++k) {
            if (center > left)
                m_melWeights[m][k] = (double)(k - left) / (center - left);
        }
        for (int k = center; k <= right && k < fftSize; ++k) {
            if (right > center)
                m_melWeights[m][k] = (double)(right - k) / (right - center);
        }
    }
}

/* ---- Hamming window ---- */

QVector<double> FeatureExtractor2::hammingWindow(const QVector<double>& frame) const
{
    int N = frame.size();
    QVector<double> windowed(N);
    for (int i = 0; i < N; ++i)
        windowed[i] = frame[i] * (0.54 - 0.46 * qCos(2.0 * M_PI * i / (N - 1)));
    return windowed;
}

/* ---- In-place radix-2 FFT ---- */

void FeatureExtractor2::fft(QVector<double>& re, QVector<double>& im) const
{
    int N = re.size();
    if (N <= 1) return;
    int log2N = 0;
    while ((1 << log2N) < N) ++log2N;

    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b) j = (j << 1) | ((i >> b) & 1);
        if (j > i) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    for (int len = 2; len <= N; len *= 2) {
        double ang = -2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tR = cR * re[o] - cI * im[o];
                double tI = cR * im[o] + cI * re[o];
                re[o] = re[e] - tR; im[o] = im[e] - tI;
                re[e] += tR; im[e] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR; cR = nR;
            }
        }
    }
}

/* ---- Power spectrum from frame ---- */

QVector<double> FeatureExtractor2::powerSpectrum(const QVector<double>& frame) const
{
    int N = frame.size();
    QVector<double> re(N), im(N, 0.0);
    for (int i = 0; i < N; ++i) re[i] = frame[i];
    fft(re, im);

    QVector<double> psd(N / 2 + 1);
    for (int i = 0; i <= N / 2; ++i)
        psd[i] = (re[i] * re[i] + im[i] * im[i]) / N;
    return psd;
}

/* ---- Mel filter bank energies ---- */

QVector<double> FeatureExtractor2::melFilterBank(const QVector<double>& powerSpectrum) const
{
    int numBins = qMin(m_numMelBins, m_melWeights.size());
    QVector<double> energies(numBins, 0.0);
    int psdSize = powerSpectrum.size();

    for (int m = 0; m < numBins; ++m) {
        int wtSize = qMin(m_melWeights[m].size(), psdSize);
        for (int k = 0; k < wtSize; ++k)
            energies[m] += m_melWeights[m][k] * powerSpectrum[k];
        energies[m] = qMax(energies[m], 1e-10); // Floor to avoid log(0)
    }
    return energies;
}

/* ---- DCT-II for cepstral coefficients ---- */

QVector<double> FeatureExtractor2::dctCepstral(const QVector<double>& logMelEnergies) const
{
    int M = logMelEnergies.size();
    int numC = qMin(m_numCoeffs, M);
    QVector<double> coeffs(numC);

    for (int k = 0; k < numC; ++k) {
        double sum = 0.0;
        for (int m = 0; m < M; ++m)
            sum += logMelEnergies[m] * qCos(M_PI * k * (2 * m + 1) / (2.0 * M));
        coeffs[k] = sum;
    }
    return coeffs;
}

/* ---- Delta features ---- */

QVector<QVector<double>> FeatureExtractor2::computeDelta(
    const QVector<QVector<double>>& features, int N) const
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

/* ---- Main extraction ---- */

FeatureExtractor2::FeatureSet FeatureExtractor2::extract(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    FeatureSet result;
    int n = audio.size();
    if (n < m_frameSize) return result;

    int numFrames = (n - m_frameSize) / m_hopSize + 1;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        // Extract frame
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize && start + i < n; ++i)
            frame[i] = audio[start + i];

        // Apply Hamming window
        frame = hammingWindow(frame);

        // Compute power spectrum
        auto psd = powerSpectrum(frame);

        // Mel filter bank energies
        auto melE = melFilterBank(psd);

        // Log mel energies
        QVector<double> logMel(melE.size());
        double frameEnergy = 0.0;
        for (int i = 0; i < psd.size(); ++i) frameEnergy += psd[i];
        for (int i = 0; i < melE.size(); ++i)
            logMel[i] = qLn(melE[i]);

        // DCT -> MFCC
        auto mfcc = dctCepstral(logMel);
        result.mfcc.append(mfcc);
        result.energy.append(qLn(qMax(frameEnergy, 1e-10)));
    }

    // Compute delta and delta-delta
    result.delta = computeDelta(result.mfcc, 2);
    result.deltaDelta = computeDelta(result.delta, 2);

    m_stats.totalExtractions++;
    m_stats.frameSize = m_frameSize;
    m_stats.numCoeffs = m_numCoeffs;
    m_stats.numMelBins = m_numMelBins;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalExtractions;

    emit extractionCompleted(numFrames, m_numCoeffs);
    return result;
}

/* ---- Reset ---- */

void FeatureExtractor2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
