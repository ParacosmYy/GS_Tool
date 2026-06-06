/**
 * @file FeatureExtractor.cpp
 * @brief FeatureExtractor 实现
 *
 * 实现音频特征提取：MFCC、频谱质心/滚降/通量、过零率、RMS。
 */

#include "utils/signal175/FeatureExtractor.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FeatureExtractor::FeatureExtractor(QObject *parent)
    : QObject(parent)
{
}

FeatureExtractor::~FeatureExtractor() = default;

/* ---- Configuration ---- */

void FeatureExtractor::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void FeatureExtractor::setMfccBins(int bins) { m_mfccBins = qMax(4, bins); }
void FeatureExtractor::setMfccCoeffs(int coeffs) { m_mfccCoeffs = qMax(1, coeffs); }

/* ---- RMS energy ---- */

double FeatureExtractor::computeRms(const QVector<double>& signal)
{
    if (signal.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : signal)
        sum += s * s;
    return qSqrt(sum / signal.size());
}

/* ---- Zero-crossing rate ---- */

double FeatureExtractor::computeZeroCrossingRate(const QVector<double>& signal)
{
    if (signal.size() < 2) return 0.0;
    int crossings = 0;
    for (int i = 1; i < signal.size(); ++i) {
        if ((signal[i] >= 0.0) != (signal[i - 1] >= 0.0))
            crossings++;
    }
    return static_cast<double>(crossings) / (signal.size() - 1);
}

/* ---- Spectral centroid ---- */

double FeatureExtractor::computeSpectralCentroid(const QVector<double>& magnitude,
                                                  double sampleRate)
{
    int n = magnitude.size();
    if (n == 0) return 0.0;
    double sumMag = 0.0, sumWeighted = 0.0;
    for (int i = 0; i < n; ++i) {
        double freq = i * sampleRate / (2.0 * n);
        sumWeighted += freq * magnitude[i];
        sumMag += magnitude[i];
    }
    return (sumMag > 0.0) ? sumWeighted / sumMag : 0.0;
}

/* ---- Spectral rolloff ---- */

double FeatureExtractor::computeSpectralRolloff(const QVector<double>& magnitude,
                                                 double sampleRate, double threshold)
{
    int n = magnitude.size();
    if (n == 0) return 0.0;
    double total = 0.0;
    for (int i = 0; i < n; ++i) total += magnitude[i];
    double cumSum = 0.0;
    for (int i = 0; i < n; ++i) {
        cumSum += magnitude[i];
        if (cumSum >= threshold * total)
            return i * sampleRate / (2.0 * n);
    }
    return 0.0;
}

/* ---- Spectral flux ---- */

double FeatureExtractor::computeSpectralFlux(const QVector<double>& magnitude,
                                              const QVector<double>& prevMagnitude)
{
    int n = qMin(magnitude.size(), prevMagnitude.size());
    if (n == 0) return 0.0;
    double flux = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = magnitude[i] - prevMagnitude[i];
        flux += diff * diff;
    }
    return qSqrt(flux / n);
}

/* ---- Mel scale ---- */

double FeatureExtractor::hzToMel(double hz) { return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0); }
double FeatureExtractor::melToHz(double mel) { return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0); }

/* ---- Pre-emphasis ---- */

QVector<double> FeatureExtractor::preEmphasis(const QVector<double>& signal, double alpha)
{
    QVector<double> out(signal.size());
    out[0] = signal[0];
    for (int i = 1; i < signal.size(); ++i)
        out[i] = signal[i] - alpha * signal[i - 1];
    return out;
}

/* ---- Hanning window ---- */

QVector<double> FeatureExtractor::hanningWindow(int n)
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
    return w;
}

/* ---- Build Mel filter bank ---- */

QVector<QVector<double>> FeatureExtractor::buildMelFilterBank(int fftSize) const
{
    double maxHz = m_sampleRate / 2.0;
    double maxMel = hzToMel(maxHz);
    int numBins = fftSize / 2 + 1;

    /* Linearly spaced Mel points */
    QVector<double> melPoints(m_mfccBins + 2);
    for (int i = 0; i < m_mfccBins + 2; ++i)
        melPoints[i] = i * maxMel / (m_mfccBins + 1);

    /* Convert back to Hz and then to FFT bin indices */
    QVector<int> binPoints(m_mfccBins + 2);
    for (int i = 0; i < m_mfccBins + 2; ++i) {
        double hz = melToHz(melPoints[i]);
        binPoints[i] = qFloor(fftSize * hz / m_sampleRate);
    }

    QVector<QVector<double>> filterBank(m_mfccBins, QVector<double>(numBins, 0.0));
    for (int m = 0; m < m_mfccBins; ++m) {
        int fLeft = binPoints[m];
        int fCenter = binPoints[m + 1];
        int fRight = binPoints[m + 2];
        for (int k = fLeft; k <= fCenter; ++k) {
            if (k >= 0 && k < numBins)
                filterBank[m][k] = static_cast<double>(k - fLeft) / qMax(fCenter - fLeft, 1);
        }
        for (int k = fCenter; k <= fRight; ++k) {
            if (k >= 0 && k < numBins)
                filterBank[m][k] = static_cast<double>(fRight - k) / qMax(fRight - fCenter, 1);
        }
    }
    return filterBank;
}

/* ---- DCT-II ---- */

QVector<double> FeatureExtractor::dctII(const QVector<double>& input)
{
    int n = input.size();
    QVector<double> output(n);
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            sum += input[i] * qCos(M_PI * k * (2 * i + 1) / (2.0 * n));
        output[k] = sum;
    }
    return output;
}

/* ---- Simple FFT (radix-2, in-place) ---- */

void FeatureExtractor::simpleFFT(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    /* Bit-reversal */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            qSwap(real[i], real[j]);
            qSwap(imag[i], imag[j]);
        }
    }
    /* Butterfly */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < len / 2; ++j) {
                double w = angle * j;
                double cs = qCos(w), sn = qSin(w);
                int u = i + j, v = i + j + len / 2;
                double tr = cs * real[v] - sn * imag[v];
                double ti = cs * imag[v] + sn * real[v];
                real[v] = real[u] - tr;
                imag[v] = imag[u] - ti;
                real[u] += tr;
                imag[u] += ti;
            }
        }
    }
}

/* ---- Compute MFCC ---- */

QVector<double> FeatureExtractor::computeMfcc(const QVector<double>& signal)
{
    if (signal.size() < 64) return QVector<double>(m_mfccCoeffs, 0.0);

    /* Pre-emphasis */
    QVector<double> emphasized = preEmphasis(signal);

    /* Determine FFT size (next power of 2) */
    int frameSize = emphasized.size();
    int fftSize = 1;
    while (fftSize < frameSize) fftSize <<= 1;

    /* Window */
    QVector<double> window = hanningWindow(frameSize);
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < frameSize; ++i)
        real[i] = emphasized[i] * window[i];

    /* FFT */
    simpleFFT(real, imag);

    /* Power spectrum */
    int numBins = fftSize / 2 + 1;
    QVector<double> power(numBins);
    for (int i = 0; i < numBins; ++i)
        power[i] = (real[i] * real[i] + imag[i] * imag[i]) / fftSize;

    /* Apply Mel filter bank */
    QVector<QVector<double>> filterBank = buildMelFilterBank(fftSize);
    QVector<double> melEnergies(m_mfccBins);
    for (int m = 0; m < m_mfccBins; ++m) {
        double sum = 0.0;
        for (int k = 0; k < numBins; ++k)
            sum += power[k] * filterBank[m][k];
        melEnergies[m] = qLn(qMax(sum, 1e-30));
    }

    /* DCT to get MFCC */
    QVector<double> dct = dctII(melEnergies);
    QVector<double> mfcc(m_mfccCoeffs);
    for (int i = 0; i < m_mfccCoeffs && i < dct.size(); ++i)
        mfcc[i] = dct[i];
    return mfcc;
}

/* ---- Main extraction ---- */

FeatureExtractor::Features FeatureExtractor::extract(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    Features f;
    if (signal.isEmpty()) return f;

    /* Time-domain features */
    f.rms = computeRms(signal);
    f.zeroCrossingRate = computeZeroCrossingRate(signal);

    /* Compute magnitude spectrum */
    int n = signal.size();
    int fftSize = 1;
    while (fftSize < n) fftSize <<= 1;

    QVector<double> window = hanningWindow(n);
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i)
        real[i] = signal[i] * window[i];

    simpleFFT(real, imag);

    int numBins = fftSize / 2 + 1;
    QVector<double> magnitude(numBins);
    for (int i = 0; i < numBins; ++i)
        magnitude[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]) / fftSize;

    /* Spectral features */
    f.spectralCentroid = computeSpectralCentroid(magnitude, m_sampleRate);
    f.spectralRolloff = computeSpectralRolloff(magnitude, m_sampleRate);
    f.spectralFlux = 0.0; /* Needs previous frame */

    /* MFCC */
    f.mfcc = computeMfcc(signal);

    m_stats.totalExtractions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalExtractions;

    emit extractionCompleted(m_mfccCoeffs + 5);
    return f;
}

/* ---- Statistics ---- */

void FeatureExtractor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
