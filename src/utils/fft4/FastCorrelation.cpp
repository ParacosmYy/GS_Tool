/**
 * @file FastCorrelation.cpp
 * @brief 快速互相关实现 — 基于FFT
 */

#include "utils/fft4/FastCorrelation.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
FastCorrelation::FastCorrelation(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算两个信号的互相关
 *  @param signal1 第一路信号
 *  @param signal2 第二路信号
 *  @return 互相关结果 */
QVector<double> FastCorrelation::correlate(const QVector<double>& signal1,
                                           const QVector<double>& signal2)
{
    QElapsedTimer timer;
    timer.start();

    int n1 = signal1.size();
    int n2 = signal2.size();
    int N = nextPowerOf2(n1 + n2 - 1);

    /* 将signal1和signal2打包为复数数组(real,imag交错) */
    QVector<double> fft1(2 * N, 0.0);
    QVector<double> fft2(2 * N, 0.0);

    for (int i = 0; i < n1; ++i) {
        fft1[2 * i] = signal1[i];
    }
    for (int i = 0; i < n2; ++i) {
        fft2[2 * i] = signal2[i];
    }

    /* FFT */
    fftImpl(fft1, N, false);
    fftImpl(fft2, N, false);

    /* 互相关: FFT1 * conj(FFT2) */
    for (int i = 0; i < N; ++i) {
        double re1 = fft1[2 * i], im1 = fft1[2 * i + 1];
        double re2 = fft2[2 * i], im2 = fft2[2 * i + 1];
        fft1[2 * i] = re1 * re2 + im1 * im2;     /* 实部 */
        fft1[2 * i + 1] = im1 * re2 - re1 * im2;  /* 虚部(取共轭) */
    }

    /* IFFT */
    fftImpl(fft1, N, true);

    /* 提取结果(取实部) */
    int resultLen = n1 + n2 - 1;
    QVector<double> result(resultLen);
    for (int i = 0; i < resultLen; ++i) {
        result[i] = fft1[2 * i];
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalCorrelations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalCorrelations);

    emit correlationCompleted(resultLen);
    return result;
}

/** @brief 计算信号的自相关
 *  @param signal 输入信号
 *  @return 自相关结果 */
QVector<double> FastCorrelation::autoCorrelate(const QVector<double>& signal)
{
    return correlate(signal, signal);
}

/** @brief 查找两个信号之间的滞后
 *  @param signal1 第一路信号
 *  @param signal2 第二路信号
 *  @return 滞后样本数 */
int FastCorrelation::findLag(const QVector<double>& signal1,
                             const QVector<double>& signal2)
{
    QVector<double> corr = correlate(signal1, signal2);

    int n1 = signal1.size();
    int offset = n1 - 1; /* 零滞后对应的索引 */

    int bestIdx = offset;
    double bestVal = qAbs(corr[offset]);

    for (int i = 0; i < corr.size(); ++i) {
        double val = qAbs(corr[i]);
        if (val > bestVal) {
            bestVal = val;
            bestIdx = i;
        }
    }

    return bestIdx - offset;
}

/** @brief 重置统计 */
void FastCorrelation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief FFT实现(基2-Cooley-Tukey迭代)
 *  @param data 复数数组(real,imag交错) @param n 长度 @param inverse 是否逆变换 */
void FastCorrelation::fftImpl(QVector<double>& data, int n, bool inverse)
{
    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[2 * i], data[2 * j]);
            std::swap(data[2 * i + 1], data[2 * j + 1]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / static_cast<double>(len)
                       * (inverse ? -1.0 : 1.0);
        double wRe = std::cos(angle);
        double wIm = std::sin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int idx1 = 2 * (i + j);
                int idx2 = 2 * (i + j + len / 2);
                double tRe = curRe * data[idx2] - curIm * data[idx2 + 1];
                double tIm = curRe * data[idx2 + 1] + curIm * data[idx2];
                data[idx2] = data[idx1] - tRe;
                data[idx2 + 1] = data[idx1 + 1] - tIm;
                data[idx1] += tRe;
                data[idx1 + 1] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < 2 * n; ++i) {
            data[i] /= static_cast<double>(n);
        }
    }
}

/** @brief 补零到2的幂次 */
int FastCorrelation::nextPowerOf2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}
