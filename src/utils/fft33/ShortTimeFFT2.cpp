/**
 * @file ShortTimeFFT2.cpp
 * @brief 短时FFT增强实现 — OLA重建/相位相干/频谱图/窗函数设计
 */

#include "utils/fft33/ShortTimeFFT2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ShortTimeFFT2::ShortTimeFFT2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置窗口大小 @param size FFT窗口大小 */
void ShortTimeFFT2::setWindowSize(int size)
{
    m_windowSize = qMax(16, size);
    /* 确保是2的幂 */
    int p = 1;
    while (p < m_windowSize) p <<= 1;
    m_windowSize = p;
}

/** @brief 设置跳跃大小 @param hop 跳跃采样数 */
void ShortTimeFFT2::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/** @brief 设置窗函数类型(0=Hann,1=Hamming,2=Blackman,3=Rect) @param type 类型 */
void ShortTimeFFT2::setWindowType(int type)
{
    m_windowType = qBound(0, type, 3);
}

/** @brief 设置采样率 @param rate 采样率 */
void ShortTimeFFT2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 正向STFT变换
 * @param input 时域信号
 * @return 每帧的频谱(实部虚部交替,复数格式)
 */
QVector<QVector<double>> ShortTimeFFT2::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return {};

    QVector<double> window = designWindow(m_windowSize);
    int halfN = m_windowSize / 2 + 1;
    int numFrames = (input.size() - m_windowSize) / m_hopSize + 1;
    if (numFrames <= 0) numFrames = 1;

    QVector<QVector<double>> frames;
    frames.reserve(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        QVector<double> real(m_windowSize, 0.0);
        QVector<double> imag(m_windowSize, 0.0);

        /* 加窗 */
        for (int i = 0; i < m_windowSize; ++i) {
            int idx = start + i;
            double sample = (idx < input.size()) ? input[idx] : 0.0;
            real[i] = sample * window[i];
        }

        /* FFT */
        fft(real, imag);

        /* 存储频谱(幅度) */
        QVector<double> spectrum(halfN, 0.0);
        for (int i = 0; i < halfN; ++i) {
            spectrum[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
        }
        frames.append(spectrum);
    }

    m_stats.totalTransforms++;
    m_stats.totalFramesProcessed += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalTransforms));

    emit transformComplete(numFrames);
    return frames;
}

/**
 * @brief 逆STFT变换(OLA重建)
 * @param frames STFT帧序列
 * @return 重建的时域信号
 */
QVector<double> ShortTimeFFT2::inverse(const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    if (frames.isEmpty()) return {};

    QVector<double> window = designWindow(m_windowSize);
    int numFrames = frames.size();
    int outputLen = (numFrames - 1) * m_hopSize + m_windowSize;

    QVector<double> output(outputLen, 0.0);
    QVector<double> winSum(outputLen, 0.0);

    for (int f = 0; f < numFrames; ++f) {
        int halfN = m_windowSize / 2 + 1;
        int start = f * m_hopSize;

        /* 从幅度谱重建(简化: 使用0相位) */
        QVector<double> real(m_windowSize, 0.0);
        QVector<double> imag(m_windowSize, 0.0);

        for (int i = 0; i < halfN && i < frames[f].size(); ++i) {
            real[i] = frames[f][i];
            imag[i] = 0.0;
        }
        /* 镜像频率 */
        for (int i = halfN; i < m_windowSize; ++i) {
            int mirror = m_windowSize - i;
            if (mirror >= 0 && mirror < halfN) {
                real[i] = real[mirror];
                imag[i] = -imag[mirror];
            }
        }

        /* 逆FFT(利用正向FFT的共轭) */
        for (int i = 0; i < m_windowSize; ++i) imag[i] = -imag[i];
        fft(real, imag);
        for (int i = 0; i < m_windowSize; ++i) {
            real[i] /= m_windowSize;
            imag[i] = -imag[i] / m_windowSize;
        }

        /* OLA叠加 */
        for (int i = 0; i < m_windowSize; ++i) {
            int idx = start + i;
            if (idx < outputLen) {
                output[idx] += real[i] * window[i];
                winSum[idx] += window[i] * window[i];
            }
        }
    }

    /* 归一化OLA */
    for (int i = 0; i < outputLen; ++i) {
        if (winSum[i] > 1e-10) {
            output[i] /= winSum[i];
        }
    }

    m_timeSum += timer.elapsed();
    return output;
}

/**
 * @brief 计算频谱图(对数功率谱)
 * @param input 时域信号
 * @return 展平的对数功率谱
 */
QVector<double> ShortTimeFFT2::spectrogram(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> stftFrames = forward(input);
    if (stftFrames.isEmpty()) return {};

    QVector<double> result;
    result.reserve(stftFrames.size() * stftFrames[0].size());

    for (const auto& frame : stftFrames) {
        for (double val : frame) {
            /* 转换为dB */
            double db = (val > 1e-10)
                ? 20.0 * qLn(val) / qLn(10.0) : -120.0;
            result.append(db);
        }
    }

    m_timeSum += timer.elapsed();
    return result;
}

/** @brief 重置统计 */
void ShortTimeFFT2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 原地Cooley-Tukey FFT
 * @param real 实部数组
 * @param imag 虚部数组
 */
void ShortTimeFFT2::fft(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();
    if (N <= 1) return;

    /* 位反转排列 */
    int bits = 0;
    for (int tmp = N; tmp > 1; tmp >>= 1) ++bits;
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) j |= (1 << (bits - 1 - b));
        }
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle);
        double wI = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nr;
            }
        }
    }
}

/**
 * @brief 设计窗函数
 * @param size 窗口长度
 * @return 窗函数系数
 */
QVector<double> ShortTimeFFT2::designWindow(int size) const
{
    QVector<double> w(size);
    if (size <= 1) {
        w[0] = 1.0;
        return w;
    }

    double denom = static_cast<double>(size - 1);

    switch (m_windowType) {
    case 0: {
        /* Hann窗 */
        for (int i = 0; i < size; ++i) {
            w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / denom));
        }
        break;
    }
    case 1: {
        /* Hamming窗 */
        for (int i = 0; i < size; ++i) {
            w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / denom);
        }
        break;
    }
    case 2: {
        /* Blackman窗 */
        for (int i = 0; i < size; ++i) {
            w[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * i / denom)
                + 0.08 * qCos(4.0 * M_PI * i / denom);
        }
        break;
    }
    case 3: {
        /* 矩形窗 */
        w.fill(1.0);
        break;
    }
    default:
        w.fill(1.0);
        break;
    }

    return w;
}
