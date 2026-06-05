/**
 * @file MelFrequencyCepstrum.cpp
 * @brief MFCC梅尔频率倒谱系数实现
 */

#include "MelFrequencyCepstrum.h"
#include <QElapsedTimer>
#include <cmath>

MelFrequencyCepstrum::MelFrequencyCepstrum(int numFilters, int numCoeffs,
                                               QObject* parent)
    : QObject(parent)
    , m_numFilters(numFilters > 0 ? numFilters : 26)
    , m_numCoeffs(numCoeffs > 0 ? numCoeffs : 13)
    , m_timeSum(0.0)
{
}

QVector<QVector<double>> MelFrequencyCepstrum::compute(
    const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    int fftSize = 1;
    while (fftSize < N) fftSize *= 2;

    /* 加Hanning窗 */
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < N; ++i) {
        double w = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
        real[i] = signal[i] * w;
    }

    /* FFT */
    for (int i = 1, j = 0; i < fftSize; ++i) {
        int bit = fftSize >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(real[i], real[j]); std::swap(imag[i], imag[j]); }
    }
    for (int len = 2; len <= fftSize; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wR = std::cos(angle), wI = std::sin(angle);
        for (int i = 0; i < fftSize; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tR = cR * real[i + j + len / 2] - cI * imag[i + j + len / 2];
                double tI = cR * imag[i + j + len / 2] + cI * real[i + j + len / 2];
                real[i + j + len / 2] = real[i + j] - tR;
                imag[i + j + len / 2] = imag[i + j] - tI;
                real[i + j] += tR;
                imag[i + j] += tI;
                double nR = cR * wR - cI * wI; cI = cR * wI + cI * wR; cR = nR;
            }
        }
    }

    /* 功率谱 */
    int halfBin = fftSize / 2 + 1;
    QVector<double> powerSpectrum(halfBin);
    for (int i = 0; i < halfBin; ++i)
        powerSpectrum[i] = (real[i] * real[i] + imag[i] * imag[i]) / fftSize;

    /* 梅尔滤波器组 */
    QVector<double> melEnergies = applyMelFilterBank(powerSpectrum, sampleRate);

    /* 取对数 */
    for (double& v : melEnergies) v = std::log(std::max(v, 1e-10));

    /* DCT */
    QVector<double> mfcc = dct(melEnergies, m_numCoeffs);

    QVector<QVector<double>> result(1, mfcc);

    m_stats.totalComputations++;
    m_stats.totalFrames++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(1, m_numCoeffs);
    return result;
}

QVector<double> MelFrequencyCepstrum::applyMelFilterBank(
    const QVector<double>& spectrum, double sampleRate) const
{
    int N = spectrum.size();
    double maxMel = hzToMel(sampleRate / 2.0);

    QVector<double> melPoints(m_numFilters + 2);
    for (int i = 0; i < m_numFilters + 2; ++i)
        melPoints[i] = i * maxMel / (m_numFilters + 1);

    QVector<int> binPoints(m_numFilters + 2);
    for (int i = 0; i < m_numFilters + 2; ++i)
        binPoints[i] = static_cast<int>(melToHz(melPoints[i]) / sampleRate * 2 * (N - 1));

    QVector<double> energies(m_numFilters, 0.0);
    for (int m = 0; m < m_numFilters; ++m) {
        int start = binPoints[m];
        int center = binPoints[m + 1];
        int end = binPoints[m + 2];

        for (int k = start; k <= center && k < N; ++k) {
            if (center > start)
                energies[m] += spectrum[k] * static_cast<double>(k - start) / (center - start);
        }
        for (int k = center; k <= end && k < N; ++k) {
            if (end > center)
                energies[m] += spectrum[k] * static_cast<double>(end - k) / (end - center);
        }
    }
    return energies;
}

QVector<double> MelFrequencyCepstrum::dct(const QVector<double>& input,
                                            int numCoeffs) const
{
    int N = input.size();
    QVector<double> coeffs(numCoeffs);
    for (int k = 0; k < numCoeffs; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n)
            sum += input[n] * std::cos(M_PI * k * (2 * n + 1) / (2 * N));
        coeffs[k] = sum;
    }
    return coeffs;
}

double MelFrequencyCepstrum::hzToMel(double hz) const { return 2595.0 * std::log10(1.0 + hz / 700.0); }
double MelFrequencyCepstrum::melToHz(double mel) const { return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0); }

void MelFrequencyCepstrum::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
