/**
 * @file ShortTimeFFT.cpp
 * @brief 短时傅里叶变换(STFT)实现
 */

#include "ShortTimeFFT.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

ShortTimeFFT::ShortTimeFFT(int fftSize, int hopSize, WindowType windowType,
                           QObject* parent)
    : QObject(parent)
    , m_fftSize(fftSize)
    , m_hopSize(hopSize)
    , m_windowType(windowType)
    , m_timeSum(0.0)
{
    generateWindow();
}

void ShortTimeFFT::generateWindow()
{
    m_window.resize(m_fftSize);
    int N = m_fftSize;

    switch (m_windowType) {
    case Rectangular:
        for (int i = 0; i < N; ++i)
            m_window[i] = 1.0;
        break;

    case Hann:
        for (int i = 0; i < N; ++i)
            m_window[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
        break;

    case Hamming:
        for (int i = 0; i < N; ++i)
            m_window[i] = 0.54 - 0.46 * std::cos(2.0 * M_PI * i / (N - 1));
        break;

    case Blackman:
        for (int i = 0; i < N; ++i)
            m_window[i] = 0.42 - 0.5 * std::cos(2.0 * M_PI * i / (N - 1))
                         + 0.08 * std::cos(4.0 * M_PI * i / (N - 1));
        break;

    case Kaiser: {
        double beta = 6.0;
        double denom = 1.0;
        for (int k = 1; k <= 10; ++k)
            denom *= k;
        for (int i = 0; i < N; ++i) {
            double alpha = 2.0 * i / (N - 1) - 1.0;
            double x = beta * std::sqrt(1.0 - alpha * alpha);
            double numer = 1.0;
            double bessel = 1.0;
            double term = 1.0;
            for (int k = 1; k <= 10; ++k) {
                term *= (x / (2.0 * k));
                bessel += term * term;
            }
            m_window[i] = bessel / denom;
        }
        double maxW = *std::max_element(m_window.begin(), m_window.end());
        for (int i = 0; i < N; ++i)
            m_window[i] /= maxW;
        break;
    }
    }
}

QVector<QVector<QPair<double,double>>> ShortTimeFFT::transform(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = qMax(1, (signal.size() - m_fftSize) / m_hopSize + 1);
    int halfN = m_fftSize / 2 + 1;
    QVector<QVector<QPair<double,double>>> result(numFrames);

    QVector<double> real(m_fftSize), imag(m_fftSize);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        /* 加窗 */
        for (int i = 0; i < m_fftSize; ++i) {
            int idx = start + i;
            real[i] = (idx < signal.size()) ? signal[idx] * m_window[i] : 0.0;
            imag[i] = 0.0;
        }

        /* FFT */
        fft(real, imag, false);

        /* 存储复数结果(仅正频率) */
        result[f].resize(halfN);
        for (int i = 0; i < halfN; ++i)
            result[f][i] = {real[i], imag[i]};
    }

    m_stats.totalTransforms++;
    m_stats.totalFramesProcessed += numFrames;
    m_stats.totalSamplesProcessed += signal.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, halfN);
    return result;
}

QVector<QVector<double>> ShortTimeFFT::magnitudeSpectrum(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = qMax(1, (signal.size() - m_fftSize) / m_hopSize + 1);
    int halfN = m_fftSize / 2 + 1;
    QVector<QVector<double>> mag(numFrames, QVector<double>(halfN));

    QVector<double> real(m_fftSize), imag(m_fftSize);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        for (int i = 0; i < m_fftSize; ++i) {
            int idx = start + i;
            real[i] = (idx < signal.size()) ? signal[idx] * m_window[i] : 0.0;
            imag[i] = 0.0;
        }
        fft(real, imag, false);

        for (int i = 0; i < halfN; ++i)
            mag[f][i] = std::sqrt(real[i] * real[i] + imag[i] * imag[i]);
    }

    m_stats.totalTransforms++;
    m_stats.totalFramesProcessed += numFrames;
    m_stats.totalSamplesProcessed += signal.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, halfN);
    return mag;
}

QVector<QVector<double>> ShortTimeFFT::powerSpectrum(
    const QVector<double>& signal)
{
    auto mag = magnitudeSpectrum(signal);
    for (auto& frame : mag)
        for (int i = 0; i < frame.size(); ++i)
            frame[i] = frame[i] * frame[i];
    return mag;
}

QVector<double> ShortTimeFFT::inverseTransform(
    const QVector<QVector<QPair<double,double>>>& stftFrames,
    int totalSamples)
{
    if (stftFrames.isEmpty()) return {};

    QVector<double> output(totalSamples, 0.0);
    QVector<double> windowSum(totalSamples, 0.0);
    int halfN = m_fftSize / 2 + 1;

    for (int f = 0; f < stftFrames.size(); ++f) {
        int start = f * m_hopSize;

        /* 重建完整频谱(共轭对称) */
        QVector<double> real(m_fftSize, 0.0), imag(m_fftSize, 0.0);
        for (int i = 0; i < halfN && i < stftFrames[f].size(); ++i) {
            real[i] = stftFrames[f][i].first;
            imag[i] = stftFrames[f][i].second;
        }
        for (int i = halfN; i < m_fftSize; ++i) {
            int mirror = m_fftSize - i;
            real[i] = real[mirror];
            imag[i] = -imag[mirror];
        }

        /* 逆FFT */
        fft(real, imag, true);

        /* 重叠相加 */
        for (int i = 0; i < m_fftSize && (start + i) < totalSamples; ++i) {
            output[start + i] += real[i] * m_window[i];
            windowSum[start + i] += m_window[i] * m_window[i];
        }
    }

    /* 归一化 */
    for (int i = 0; i < totalSamples; ++i) {
        if (windowSum[i] > 1e-10)
            output[i] /= windowSum[i];
    }

    return output;
}

QVector<double> ShortTimeFFT::frequencyAxis(double sampleRate) const
{
    int halfN = m_fftSize / 2 + 1;
    QVector<double> freqs(halfN);
    for (int i = 0; i < halfN; ++i)
        freqs[i] = i * sampleRate / m_fftSize;
    return freqs;
}

QVector<double> ShortTimeFFT::timeAxis(double sampleRate, int totalSamples) const
{
    int numFrames = qMax(1, (totalSamples - m_fftSize) / m_hopSize + 1);
    QVector<double> times(numFrames);
    for (int i = 0; i < numFrames; ++i)
        times[i] = (i * m_hopSize + m_fftSize / 2.0) / sampleRate;
    return times;
}

void ShortTimeFFT::fft(QVector<double>& real, QVector<double>& imag,
                        bool inverse) const
{
    int n = real.size();

    /* 位反转置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        double wReal = std::cos(angle);
        double wImag = std::sin(angle);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];
                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;
                double newReal = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = newReal;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

void ShortTimeFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
