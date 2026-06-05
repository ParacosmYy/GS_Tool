/**
 * @file ShortTimeFFT5.cpp
 * @brief 短时傅里叶变换(STFT)实现
 *
 * 支持多种窗函数和跳步长配置的STFT实现，
 * 提供时频矩阵输出和逆变换重建。
 * STFT将信号分为重叠帧，每帧加窗后做FFT，
 * 生成时间-频率二维表示。
 */

#include "utils/fft76/ShortTimeFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认FFT大小1024，跳步512，Hann窗。
 */
ShortTimeFFT5::ShortTimeFFT5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 配置STFT参数
 * @param fftSize FFT大小(必须是2的幂)
 * @param hopSize 跳步大小(通常为fftSize/2或fftSize/4)
 * @param windowType 窗函数类型
 * @return true如果参数有效
 */
bool ShortTimeFFT5::configure(int fftSize, int hopSize, const QString& windowType)
{
    if (fftSize < 8 || hopSize < 1 || hopSize > fftSize) return false;

    m_fftSize = fftSize;
    m_hopSize = hopSize;
    m_windowType = windowType.toLower();
    return true;
}

/**
 * @brief 生成窗函数
 * @param n 窗口长度
 * @param type 窗类型
 * @return 窗系数向量
 */
static QVector<double> generateWindow(int n, const QString& type)
{
    QVector<double> w(n, 1.0);
    QString t = type.toLower();

    for (int i = 0; i < n; ++i) {
        double phase = 2.0 * M_PI * i / (n - 1);
        if (t == "hann") {
            w[i] = 0.5 * (1.0 - qCos(phase));
        } else if (t == "hamming") {
            w[i] = 0.54 - 0.46 * qCos(phase);
        } else if (t == "blackman") {
            w[i] = 0.42 - 0.5 * qCos(phase) + 0.08 * qCos(2.0 * phase);
        } else {
            w[i] = 1.0; /* rectangular */
        }
    }
    return w;
}

/**
 * @brief 执行简单FFT(复数)
 * @param real 实部数组(长度必须为2的幂)
 * @param imag 虚部数组
 *
 * Cooley-Tukey FFT，原地计算。
 */
static void simpleFFT(QVector<double>& real, QVector<double>& imag)
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
        double angle = -2.0 * M_PI / len;
        double wReal = qCos(angle), wImag = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];
                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;
                double newCurReal = curReal * wReal - curImag * wImag;
                double newCurImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
                curImag = newCurImag;
            }
        }
    }
}

/**
 * @brief 执行逆FFT(复数)
 */
static void simpleIFFT(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    /* 共轭 */
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    simpleFFT(real, imag);
    /* 共轭并归一化 */
    double norm = 1.0 / n;
    for (int i = 0; i < n; ++i) {
        real[i] *= norm;
        imag[i] = -imag[i] * norm;
    }
}

/**
 * @brief 执行STFT
 * @param signal 输入时域信号
 * @return 时频矩阵[frame][bin]，复数值
 *
 * 1. 将信号分帧(重叠)
 * 2. 每帧加窗
 * 3. 对每帧执行FFT
 * 4. 返回复数时频矩阵
 */
QVector<QVector<std::complex<double>>> ShortTimeFFT5::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = (signal.size() - m_fftSize) / m_hopSize + 1;
    if (numFrames <= 0) numFrames = 1;

    QVector<double> window = generateWindow(m_fftSize, m_windowType);
    QVector<QVector<std::complex<double>>> stftMatrix(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        QVector<double> real(m_fftSize, 0.0), imag(m_fftSize, 0.0);

        /* 加窗 */
        for (int i = 0; i < m_fftSize; ++i) {
            int idx = start + i;
            real[i] = (idx < signal.size() ? signal[idx] : 0.0) * window[i];
        }

        simpleFFT(real, imag);

        stftMatrix[f].resize(m_fftSize);
        for (int i = 0; i < m_fftSize; ++i) {
            stftMatrix[f][i] = std::complex<double>(real[i], imag[i]);
        }
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames);
    return stftMatrix;
}

/**
 * @brief 执行逆STFT重建时域信号
 * @param stftMatrix 复数时频矩阵
 * @return 重建的时域信号
 *
 * 使用重叠相加(overlap-add)方法:
 * 1. 对每帧执行IFFT
 * 2. 加综合窗
 * 3. 重叠相加重建信号
 */
QVector<double> ShortTimeFFT5::inverse(const QVector<QVector<std::complex<double>>>& stftMatrix)
{
    QElapsedTimer timer;
    timer.start();

    if (stftMatrix.isEmpty()) return QVector<double>();

    int numFrames = stftMatrix.size();
    int n = m_fftSize;
    int totalSamples = (numFrames - 1) * m_hopSize + n;
    QVector<double> output(totalSamples, 0.0);
    QVector<double> window = generateWindow(n, m_windowType);

    /* 计算归一化因子(窗函数归一化) */
    double winNorm = 0.0;
    for (int i = 0; i < n; ++i) {
        double overlapSum = 0.0;
        for (int f = 0; f * m_hopSize + i < totalSamples && i < n; f++) {
            overlapSum += window[i] * window[i];
        }
        winNorm = qMax(winNorm, overlapSum);
    }

    for (int f = 0; f < numFrames; ++f) {
        QVector<double> real(n, 0.0), imag(n, 0.0);
        for (int i = 0; i < qMin(n, (int)stftMatrix[f].size()); ++i) {
            real[i] = stftMatrix[f][i].real();
            imag[i] = stftMatrix[f][i].imag();
        }

        simpleIFFT(real, imag);

        /* 加窗并重叠相加 */
        int start = f * m_hopSize;
        for (int i = 0; i < n && start + i < totalSamples; ++i) {
            output[start + i] += real[i] * window[i];
        }
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return output;
}

/**
 * @brief 获取幅度谱矩阵
 * @param signal 输入信号
 * @return 幅度矩阵[frame][bin]
 */
QVector<QVector<double>> ShortTimeFFT5::magnitudeMatrix(const QVector<double>& signal)
{
    auto stft = forward(signal);
    QVector<QVector<double>> mag(stft.size());

    for (int f = 0; f < stft.size(); ++f) {
        mag[f].resize(stft[f].size());
        for (int i = 0; i < (int)stft[f].size(); ++i) {
            mag[f][i] = std::abs(stft[f][i]);
        }
    }
    return mag;
}

/**
 * @brief 重置统计信息
 */
void ShortTimeFFT5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
