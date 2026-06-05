/**
 * @file SpectralWhitener.cpp
 * @brief 频谱白化 — FFT频谱平坦化处理
 */

#include "SpectralWhitener.h"
#include <QElapsedTimer>
#include <cmath>

SpectralWhitener::SpectralWhitener(QObject* parent)
    : QObject(parent)
    , m_fftSize(1024)
    , m_timeSum(0.0)
{
}

QVector<double> SpectralWhitener::whiten(const QVector<double>& input, double strength)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_fftSize;
    if (input.size() < N) N = input.size();

    /* 补零到2的幂 */
    int fftN = 1;
    while (fftN < N) fftN *= 2;

    QVector<double> real(fftN, 0.0), imag(fftN, 0.0);
    for (int i = 0; i < qMin(input.size(), fftN); ++i) {
        real[i] = input[i];
    }

    /* FFT */
    fft(real, imag);

    /* 计算幅度谱 */
    QVector<double> mag(fftN / 2);
    for (int i = 0; i < fftN / 2; ++i) {
        mag[i] = std::sqrt(real[i] * real[i] + imag[i] * imag[i]);
    }

    /* 计算平均幅度 */
    double avgMag = 0.0;
    for (int i = 1; i < fftN / 2; ++i) avgMag += mag[i];
    avgMag /= (fftN / 2 - 1);

    /* 白化: 缩放频谱使幅度趋向平均值 */
    for (int i = 0; i < fftN / 2; ++i) {
        double currentMag = std::max(mag[i], 1e-10);
        double targetMag = avgMag * (1.0 - strength) + currentMag * strength;
        double scale = avgMag / currentMag;
        double whitenedScale = 1.0 + (scale - 1.0) * strength;

        real[i] *= whitenedScale;
        imag[i] *= whitenedScale;

        /* 共轭对称 */
        if (i > 0 && i < fftN / 2) {
            real[fftN - i] = real[i];
            imag[fftN - i] = -imag[i];
        }
    }

    /* IFFT */
    ifft(real, imag);

    QVector<double> result(input.size());
    for (int i = 0; i < input.size(); ++i) {
        result[i] = real[i] / fftN;
    }

    /* 计算平坦度 */
    double geoMean = 0.0, arithMean = 0.0;
    for (int i = 1; i < fftN / 2; ++i) {
        double m = std::max(mag[i], 1e-10);
        geoMean += std::log(m);
        arithMean += m;
    }
    geoMean = std::exp(geoMean / (fftN / 2 - 1));
    arithMean /= (fftN / 2 - 1);
    double flatness = (arithMean > 0) ? geoMean / arithMean : 0.0;

    m_stats.totalWhitening++;
    m_stats.totalSamples += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalWhitening;

    emit whiteningCompleted(input.size(), flatness);
    return result;
}

QVector<double> SpectralWhitener::magnitudeSpectrum(const QVector<double>& input) const
{
    int fftN = 1;
    while (fftN < input.size()) fftN *= 2;

    QVector<double> real(fftN, 0.0), imag(fftN, 0.0);
    for (int i = 0; i < input.size(); ++i) real[i] = input[i];

    const_cast<SpectralWhitener*>(this)->fft(real, imag);

    QVector<double> mag(fftN / 2);
    for (int i = 0; i < fftN / 2; ++i) {
        mag[i] = std::sqrt(real[i] * real[i] + imag[i] * imag[i]);
    }
    return mag;
}

QVector<double> SpectralWhitener::powerSpectrum(const QVector<double>& input) const
{
    QVector<double> mag = magnitudeSpectrum(input);
    for (double& m : mag) m = m * m;
    return mag;
}

void SpectralWhitener::setFFTSize(int size)
{
    m_fftSize = (size > 0) ? size : 1024;
}

void SpectralWhitener::fft(QVector<double>& real, QVector<double>& imag) const
{
    int N = real.size();
    if (N <= 1) return;

    /* 位反转排列 */
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= N; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wReal = std::cos(angle), wImag = std::sin(angle);
        for (int i = 0; i < N; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tReal = curReal * real[i + j + len / 2] - curImag * imag[i + j + len / 2];
                double tImag = curReal * imag[i + j + len / 2] + curImag * real[i + j + len / 2];
                real[i + j + len / 2] = real[i + j] - tReal;
                imag[i + j + len / 2] = imag[i + j] - tImag;
                real[i + j] += tReal;
                imag[i + j] += tImag;
                double newCurReal = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
            }
        }
    }
}

void SpectralWhitener::ifft(QVector<double>& real, QVector<double>& imag) const
{
    int N = real.size();
    /* 共轭 */
    for (int i = 0; i < N; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < N; ++i) {
        real[i] /= N;
        imag[i] = -imag[i] / N;
    }
}

void SpectralWhitener::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
