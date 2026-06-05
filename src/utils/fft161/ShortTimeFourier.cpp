/**
 * @file ShortTimeFourier.cpp
 * @brief 短时傅里叶变换(STFT)实现
 *
 * 实现完整的STFT/ISTFT：窗函数生成、分帧加窗、基2 FFT/IFFT、
 * overlap-add重建。
 */

#include "utils/fft161/ShortTimeFourier.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
ShortTimeFourier::ShortTimeFourier(QObject* parent)
    : QObject(parent)
{
}

void ShortTimeFourier::setWindowType(WindowType type)
{
    m_windowType = type;
}

void ShortTimeFourier::setFrameSize(int size)
{
    m_frameSize = qMax(2, size);
}

void ShortTimeFourier::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

void ShortTimeFourier::setFftSize(int nfft)
{
    m_nfft = qMax(2, nfft);
}

void ShortTimeFourier::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 生成窗函数
 */
QVector<double> ShortTimeFourier::generateWindow(int size) const
{
    QVector<double> window(size);
    const double pi = M_PI;

    switch (m_windowType) {
    case WindowType::Hanning:
        for (int i = 0; i < size; ++i)
            window[i] = 0.5 * (1.0 - qCos(2.0 * pi * i / (size - 1)));
        break;
    case WindowType::Hamming:
        for (int i = 0; i < size; ++i)
            window[i] = 0.54 - 0.46 * qCos(2.0 * pi * i / (size - 1));
        break;
    case WindowType::Blackman:
        for (int i = 0; i < size; ++i)
            window[i] = 0.42 - 0.5 * qCos(2.0 * pi * i / (size - 1))
                       + 0.08 * qCos(4.0 * pi * i / (size - 1));
        break;
    case WindowType::Rectangular:
        for (int i = 0; i < size; ++i)
            window[i] = 1.0;
        break;
    }
    return window;
}

/**
 * @brief 基2 FFT(就地Cooley-Tukey)
 */
void ShortTimeFourier::fft(QVector<Complex>& x) const
{
    int n = x.size();
    if (n <= 1) return;

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        Complex wlen{ qCos(angle), qSin(angle) };
        for (int i = 0; i < n; i += len) {
            Complex w{ 1.0, 0.0 };
            for (int j = 0; j < len / 2; ++j) {
                Complex u = x[i + j];
                Complex t{ w.real * x[i + j + len / 2].real - w.imag * x[i + j + len / 2].imag,
                            w.real * x[i + j + len / 2].imag + w.imag * x[i + j + len / 2].real };
                x[i + j] = { u.real + t.real, u.imag + t.imag };
                x[i + j + len / 2] = { u.real - t.real, u.imag - t.imag };
                double newWr = w.real * wlen.real - w.imag * wlen.imag;
                double newWi = w.real * wlen.imag + w.imag * wlen.real;
                w = { newWr, newWi };
            }
        }
    }
}

/**
 * @brief 基2 IFFT
 */
void ShortTimeFourier::ifft(QVector<Complex>& x) const
{
    int n = x.size();
    /* 共轭 */
    for (auto& c : x) c.imag = -c.imag;
    fft(x);
    double invN = 1.0 / n;
    for (auto& c : x) {
        c.real *= invN;
        c.imag = -c.imag * invN;
    }
}

/**
 * @brief 执行STFT变换
 *
 * 分帧 → 加窗 → 补零到FFT点数 → FFT → 输出复数频谱矩阵
 */
QVector<QVector<ShortTimeFourier::Complex>> ShortTimeFourier::transform(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int sigLen = signal.size();
    if (sigLen < m_frameSize) return QVector<QVector<Complex>>();

    QVector<double> window = generateWindow(m_frameSize);
    QVector<QVector<Complex>> spectrogram;
    double globalPeakMag = 0.0;
    int peakBin = 0;

    for (int start = 0; start + m_frameSize <= sigLen; start += m_hopSize) {
        /* 加窗并补零 */
        QVector<Complex> frame(m_nfft);
        for (int i = 0; i < m_frameSize; ++i) {
            frame[i].real = signal[start + i] * window[i];
            frame[i].imag = 0.0;
        }
        for (int i = m_frameSize; i < m_nfft; ++i) {
            frame[i] = { 0.0, 0.0 };
        }

        fft(frame);
        spectrogram.append(frame);

        /* 峰值频率追踪 */
        for (int i = 0; i < m_nfft / 2; ++i) {
            double mag = frame[i].magnitude();
            if (mag > globalPeakMag) {
                globalPeakMag = mag;
                peakBin = i;
            }
        }

        m_stats.totalFrames++;
    }

    m_stats.peakFrequency = (m_nfft > 0) ? peakBin * m_sampleRate / m_nfft : 0.0;

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(spectrogram.size(), m_nfft / 2);
    return spectrogram;
}

/**
 * @brief 逆STFT(ISTFT)，overlap-add重建
 */
QVector<double> ShortTimeFourier::inverseTransform(
    const QVector<QVector<Complex>>& spectrogram)
{
    if (spectrogram.isEmpty()) return QVector<double>();

    const int numFrames = spectrogram.size();
    const int nfft = spectrogram[0].size();
    QVector<double> window = generateWindow(m_frameSize);

    /* 输出信号长度估计 */
    int outputLen = m_frameSize + (numFrames - 1) * m_hopSize;
    QVector<double> output(outputLen, 0.0);
    QVector<double> winSum(outputLen, 0.0);

    for (int f = 0; f < numFrames; ++f) {
        QVector<Complex> frame = spectrogram[f];
        ifft(frame);

        int start = f * m_hopSize;
        for (int i = 0; i < m_frameSize && start + i < outputLen; ++i) {
            output[start + i] += frame[i].real * window[i];
            winSum[start + i] += window[i] * window[i];
        }
    }

    /* 归一化 */
    for (int i = 0; i < outputLen; ++i) {
        if (winSum[i] > 1e-10) {
            output[i] /= winSum[i];
        }
    }

    return output;
}

/**
 * @brief 获取幅度谱矩阵
 */
QVector<QVector<double>> ShortTimeFourier::magnitudeSpectrum(
    const QVector<QVector<Complex>>& spectrogram) const
{
    QVector<QVector<double>> mag;
    mag.reserve(spectrogram.size());
    for (const auto& frame : spectrogram) {
        QVector<double> frameMag;
        frameMag.reserve(frame.size() / 2);
        for (int i = 0; i < frame.size() / 2; ++i) {
            frameMag.append(frame[i].magnitude());
        }
        mag.append(frameMag);
    }
    return mag;
}

void ShortTimeFourier::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
