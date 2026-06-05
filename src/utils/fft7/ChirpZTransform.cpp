/**
 * @file ChirpZTransform.cpp
 * @brief Chirp Z-Transform 变换引擎实现 — Bluestein算法
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/fft7/ChirpZTransform.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
ChirpZTransform::ChirpZTransform(QObject *parent)
    : QObject(parent)
{
}

/** @brief 在指定频率范围内执行Chirp Z变换，使用Bluestein算法将CZT转为循环卷积再用FFT加速
 *  @param signal 输入时域信号
 *  @param startFreq 起始频率(归一化0~0.5)
 *  @param endFreq 结束频率(归一化0~0.5)
 *  @param numPoints 输出频率点数
 *  @return 变换结果的幅度谱 */
QVector<double> ChirpZTransform::transform(const QVector<double> &signal,
                                           double startFreq, double endFreq,
                                           int numPoints)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    if (N == 0 || numPoints <= 0 || startFreq >= endFreq) {
        return {};
    }

    /* 计算chirp因子: y[k] = exp(-j*pi*(k^2)*dw) 其中dw = (endFreq-startFreq)/numPoints */
    double dw = (endFreq - startFreq) / numPoints;

    /* 构造 chirp 序列: a[n] = signal[n] * exp(-j*2*pi*startFreq*n) * exp(-j*pi*dw*n^2) */
    /* 旋转因子: W = exp(-j*2*pi*dw) */
    int M = numPoints;
    int L = nextPowerOf2(N + M - 1); /* 循环卷积长度 */

    /* 构造 a[n]: signal * 旋转项 * chirp */
    QVector<double> aReal(L, 0.0), aImag(L, 0.0);
    for (int n = 0; n < N; ++n) {
        double phase = -2.0 * M_PI * startFreq * n - M_PI * dw * n * n;
        aReal[n] = signal[n] * qCos(phase);
        aImag[n] = signal[n] * qSin(phase);
    }

    /* 构造 b[n]: b[n] = exp(j*pi*dw*n^2), 周期扩展 */
    QVector<double> bReal(L, 0.0), bImag(L, 0.0);
    for (int n = 0; n < L; ++n) {
        int idx = (n <= M + N - 2) ? n : n - L;
        double phase = M_PI * dw * idx * idx;
        bReal[n] = qCos(phase);
        bImag[n] = -qSin(phase); /* 共轭 */
    }
    /* 实际上 b[n] = exp(j*pi*dw*n^2)，不做共轭 */
    for (int n = 0; n < L; ++n) {
        int idx = (n <= M + N - 2) ? n : n - L;
        double phase = M_PI * dw * idx * idx;
        bReal[n] = qCos(phase);
        bImag[n] = qSin(phase);
    }

    /* 循环卷积 = FFT(a) * FFT(b)，然后IFFT */
    fftImpl(aReal, aImag, false);
    fftImpl(bReal, bImag, false);

    /* 频域相乘 */
    for (int i = 0; i < L; ++i) {
        double re = aReal[i] * bReal[i] - aImag[i] * bImag[i];
        double im = aReal[i] * bImag[i] + aImag[i] * bReal[i];
        aReal[i] = re;
        aImag[i] = im;
    }

    /* IFFT */
    fftImpl(aReal, aImag, true);
    for (int i = 0; i < L; ++i) {
        aReal[i] /= L;
        aImag[i] /= L;
    }

    /* 乘以最终chirp因子得到CZT结果 */
    QVector<double> magnitude(M);
    for (int k = 0; k < M; ++k) {
        double phase = -M_PI * dw * k * k;
        double chirpRe = qCos(phase);
        double chirpIm = qSin(phase);
        double re = aReal[k] * chirpRe - aImag[k] * chirpIm;
        double im = aReal[k] * chirpIm + aImag[k] * chirpRe;
        magnitude[k] = qSqrt(re * re + im * im);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(M, elapsed);
    return magnitude;
}

/** @brief 以中心频率和带宽执行zoom-FFT，自动计算频率范围后委托给transform()
 *  @param signal 输入时域信号
 *  @param centerFreq 中心频率(归一化0~0.5)
 *  @param bandwidth 分析带宽(归一化>0)
 *  @param numPoints 输出频率点数
 *  @return 变换结果的幅度谱 */
QVector<double> ChirpZTransform::zoom(const QVector<double> &signal,
                                      double centerFreq, double bandwidth,
                                      int numPoints)
{
    double halfBw = bandwidth / 2.0;
    double startFreq = qMax(0.0, centerFreq - halfBw);
    double endFreq = qMin(0.5, centerFreq + halfBw);
    if (endFreq <= startFreq) {
        return {};
    }
    return transform(signal, startFreq, endFreq, numPoints);
}

/** @brief 重置统计计数器 */
void ChirpZTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 FFT内部实现(就地)，支持正向和逆向变换
 *  @param real 实部数组
 *  @param imag 虚部数组
 *  @param inverse 是否为逆变换 */
void ChirpZTransform::fftImpl(QVector<double> &real, QVector<double> &imag,
                              bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    double dir = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len *= 2) {
        double angle = dir * 2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j;
                int odd = i + j + len / 2;

                double tRe = curRe * real[odd] - curIm * imag[odd];
                double tIm = curRe * imag[odd] + curIm * real[odd];

                real[odd] = real[even] - tRe;
                imag[odd] = imag[even] - tIm;
                real[even] += tRe;
                imag[even] += tIm;

                double newRe = curRe * wRe - curIm * wIm;
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
                curIm = newIm;
            }
        }
    }
}

/** @brief 求下一个2的幂 @param n 输入值 @return >=n的最小2的幂 */
int ChirpZTransform::nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}
