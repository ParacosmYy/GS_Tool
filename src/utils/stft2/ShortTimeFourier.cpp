/**
 * @file ShortTimeFourier.cpp
 * @brief 短时傅里叶变换(STFT)实现
 */

#include "ShortTimeFourier.h"
#include <QElapsedTimer>
#include <cmath>

ShortTimeFourier::ShortTimeFourier(QObject* parent)
    : QObject(parent)
    , m_fftSize(512)
    , m_hopSize(256)
    , m_window(Hanning)
    , m_timeSum(0.0)
{
}

void ShortTimeFourier::configure(int fftSize, int hopSize, Window window)
{
    m_fftSize = (fftSize > 0) ? fftSize : 512;
    m_hopSize = (hopSize > 0) ? hopSize : m_fftSize / 2;
    m_window = window;
}

QVector<QVector<double>> ShortTimeFourier::transform(
    const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    QVector<double> win = createWindow(m_fftSize);
    int halfBin = m_fftSize / 2 + 1;
    int numFrames = (N - m_fftSize) / m_hopSize + 1;
    if (numFrames < 1) numFrames = 1;

    QVector<QVector<double>> spectrogram(numFrames, QVector<double>(halfBin, 0.0));

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        /* 加窗 */
        QVector<double> real(m_fftSize, 0.0), imag(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && (start + i) < N; ++i) {
            real[i] = signal[start + i] * win[i];
        }

        fft(real, imag);

        for (int k = 0; k < halfBin; ++k) {
            spectrogram[f][k] = std::sqrt(real[k] * real[k] + imag[k] * imag[k]);
        }
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, halfBin);
    return spectrogram;
}

QVector<double> ShortTimeFourier::frequencyAxis(double sampleRate) const
{
    int halfBin = m_fftSize / 2 + 1;
    QVector<double> freqs(halfBin);
    for (int i = 0; i < halfBin; ++i)
        freqs[i] = static_cast<double>(i) * sampleRate / m_fftSize;
    return freqs;
}

QVector<double> ShortTimeFourier::timeAxis(int signalLength, double sampleRate) const
{
    int numFrames = (signalLength - m_fftSize) / m_hopSize + 1;
    if (numFrames < 1) numFrames = 1;
    QVector<double> times(numFrames);
    for (int i = 0; i < numFrames; ++i)
        times[i] = static_cast<double>(i * m_hopSize + m_fftSize / 2) / sampleRate;
    return times;
}

QVector<double> ShortTimeFourier::createWindow(int length) const
{
    QVector<double> w(length);
    for (int i = 0; i < length; ++i) {
        double n = static_cast<double>(i) / (length - 1);
        switch (m_window) {
        case Hanning: w[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * n)); break;
        case Hamming: w[i] = 0.54 - 0.46 * std::cos(2.0 * M_PI * n); break;
        case Blackman: w[i] = 0.42 - 0.5 * std::cos(2.0 * M_PI * n) + 0.08 * std::cos(4.0 * M_PI * n); break;
        case Rectangular: w[i] = 1.0; break;
        }
    }
    return w;
}

void ShortTimeFourier::fft(QVector<double>& real, QVector<double>& imag) const
{
    int N = real.size();
    if (N <= 1) return;
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(real[i], real[j]); std::swap(imag[i], imag[j]); }
    }
    for (int len = 2; len <= N; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wR = std::cos(angle), wI = std::sin(angle);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tR = cR * real[i + j + len / 2] - cI * imag[i + j + len / 2];
                double tI = cR * imag[i + j + len / 2] + cI * real[i + j + len / 2];
                real[i + j + len / 2] = real[i + j] - tR;
                imag[i + j + len / 2] = imag[i + j] - tI;
                real[i + j] += tR;
                imag[i + j] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nR;
            }
        }
    }
}

void ShortTimeFourier::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
