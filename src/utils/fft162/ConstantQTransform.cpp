/**
 * @file ConstantQTransform.cpp
 * @brief ConstantQTransform 实现
 *
 * 实现常Q变换：几何频率间距的CQT核矩阵构建、
 * 逐bin FFT计算和频谱稀疏化。
 */

#include "utils/fft162/ConstantQTransform.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
ConstantQTransform::ConstantQTransform(QObject* parent)
    : QObject(parent)
{
}

void ConstantQTransform::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
}

void ConstantQTransform::setBinsPerOctave(int bins)
{
    m_binsPerOctave = qMax(1, bins);
}

void ConstantQTransform::setOctaveRange(int minOctave, int maxOctave)
{
    m_minOctave = qMax(0, minOctave);
    m_maxOctave = qMax(m_minOctave, maxOctave);
}

void ConstantQTransform::setThreshold(double threshold)
{
    m_threshold = qBound(0.0, threshold, 1.0);
}

/**
 * @brief 计算各bin所需的最小FFT长度
 */
int ConstantQTransform::computeMinFFT() const
{
    /* 最低频率对应最长FFT */
    double fMin = 440.0 * qPow(2.0, (m_minOctave * 12 - 69) / 12.0);
    fMin = qMax(20.0, fMin);
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
    int N = static_cast<int>(qCeil(Q * m_sampleRate / fMin));

    /* 向上取整到2的幂 */
    int power = 1;
    while (power < N) power <<= 1;
    return power;
}

/**
 * @brief 就地基2 FFT
 */
void ConstantQTransform::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    if (n <= 1) return;

    /* 位反转 */
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
        double wReal = qCos(angle);
        double wImag = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uReal = real[i + j];
                double uImag = imag[i + j];
                double tReal = curReal * real[i + j + len / 2]
                             - curImag * imag[i + j + len / 2];
                double tImag = curReal * imag[i + j + len / 2]
                             + curImag * real[i + j + len / 2];
                real[i + j] = uReal + tReal;
                imag[i + j] = uImag + tImag;
                real[i + j + len / 2] = uReal - tReal;
                imag[i + j + len / 2] = uImag - tImag;
                double nr = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = nr;
            }
        }
    }
}

/**
 * @brief 构建CQT核矩阵
 *
 * 对每个CQT bin：
 * 1) 计算中心频率 f_k
 * 2) 计算Q值和对应FFT长度 N_k
 * 3) 生成汉宁窗加权的复指数核
 * 4) 稀疏化：仅保留高于阈值的系数
 */
void ConstantQTransform::buildKernel()
{
    const double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
    m_totalBins = m_binsPerOctave * (m_maxOctave - m_minOctave + 1);
    m_freqs.resize(m_totalBins);
    m_fftLengths.resize(m_totalBins);
    m_kernelReal.resize(m_totalBins);
    m_kernelImag.resize(m_totalBins);

    int maxFFT = computeMinFFT();

    for (int k = 0; k < m_totalBins; ++k) {
        /* 中心频率：从高到低排列 */
        int binInOctaves = m_totalBins - 1 - k;
        double freq = 440.0 * qPow(2.0,
            (m_minOctave * 12.0 + binInOctaves * (12.0 / m_binsPerOctave) - 69.0) / 12.0);
        freq = qMax(20.0, qMin(m_sampleRate / 2.0 - 1.0, freq));
        m_freqs[k] = freq;

        /* FFT长度 */
        int Nk = static_cast<int>(qCeil(Q * m_sampleRate / freq));
        /* 向上到2的幂 */
        int fftLen = 1;
        while (fftLen < Nk) fftLen <<= 1;
        fftLen = qMin(fftLen, maxFFT);
        m_fftLengths[k] = fftLen;

        /* 生成汉宁窗复指数核 */
        QVector<double> kr(fftLen, 0.0);
        QVector<double> ki(fftLen, 0.0);
        for (int n = 0; n < Nk && n < fftLen; ++n) {
            double window = 0.5 * (1.0 - qCos(2.0 * M_PI * n / Nk));
            double angle = 2.0 * M_PI * Q * n / Nk;
            kr[n] = window * qCos(angle) / fftLen;
            ki[n] = -window * qSin(angle) / fftLen;
        }

        /* FFT变换到频域 */
        fft(kr, ki);

        /* 稀疏化：仅保留高于阈值的系数 */
        double maxMag = 0.0;
        for (int i = 0; i < fftLen; ++i) {
            double mag = qSqrt(kr[i] * kr[i] + ki[i] * ki[i]);
            if (mag > maxMag) maxMag = mag;
        }

        double thresh = m_threshold * maxMag;
        m_kernelReal[k].clear();
        m_kernelImag[k].clear();

        /* 存储稀疏核(只存非零) */
        m_kernelReal[k].resize(fftLen);
        m_kernelImag[k].resize(fftLen);
        for (int i = 0; i < fftLen; ++i) {
            double mag = qSqrt(kr[i] * kr[i] + ki[i] * ki[i]);
            if (mag >= thresh) {
                m_kernelReal[k][i] = kr[i];
                m_kernelImag[k][i] = ki[i];
            } else {
                m_kernelReal[k][i] = 0.0;
                m_kernelImag[k][i] = 0.0;
            }
        }
    }

    m_stats.minFrequency = m_freqs.last();
    m_stats.maxFrequency = m_freqs.first();
}

/**
 * @brief 执行CQT变换
 *
 * 对输入信号执行FFT，然后与预计算的CQT核矩阵相乘得到各bin的CQT系数。
 */
QVector<QVector<double>> ConstantQTransform::transform(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || m_kernelReal.isEmpty()) {
        if (m_kernelReal.isEmpty()) buildKernel();
        if (signal.isEmpty()) return QVector<QVector<double>>();
    }

    const int sigLen = signal.size();
    const int maxFFT = m_fftLengths[0];

    /* 对信号做FFT */
    int fftLen = 1;
    while (fftLen < sigLen) fftLen <<= 1;
    fftLen = qMax(fftLen, maxFFT);

    QVector<double> sigReal(fftLen, 0.0);
    QVector<double> sigImag(fftLen, 0.0);
    for (int i = 0; i < qMin(sigLen, fftLen); ++i) {
        sigReal[i] = signal[i];
    }
    fft(sigReal, sigImag);

    /* 计算每个CQT bin的系数 */
    QVector<QVector<double>> cqtCoeffs(m_totalBins);
    for (int k = 0; k < m_totalBins; ++k) {
        int Nk = m_fftLengths[k];
        double realSum = 0.0;
        double imagSum = 0.0;
        for (int i = 0; i < Nk; ++i) {
            realSum += sigReal[i] * m_kernelReal[k][i]
                     - sigImag[i] * m_kernelImag[k][i];
            imagSum += sigReal[i] * m_kernelImag[k][i]
                     + sigImag[i] * m_kernelReal[k][i];
        }
        double mag = qSqrt(realSum * realSum + imagSum * imagSum);
        cqtCoeffs[k].append(mag);
    }

    m_stats.totalBinsComputed += m_totalBins;
    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(m_totalBins, 1);
    return cqtCoeffs;
}

void ConstantQTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
