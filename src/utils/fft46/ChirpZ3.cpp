/**
 * @file ChirpZ3.cpp
 * @brief Chirp-Z变换3实现 — 任意等高线+精细化FFT
 *
 * Chirp-Z变换允许在单位圆上任意弧段进行等间隔频谱分析，
 * 比标准FFT更灵活，适合需要高分辨率局部频谱分析的场景。
 * 本实现基于Bluestein算法，利用卷积实现CZT。
 */

#include "utils/fft46/ChirpZ3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
ChirpZ3::ChirpZ3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置CZT参数
 * @param A_real 起始点A的实部
 * @param A_imag 起始点A的虚部
 * @param W_real 采样点间距W的实部
 * @param W_imag 采样点间距W的虚部
 * @param M 输出点数
 */
void ChirpZ3::setParameters(double A_real, double A_imag,
                              double W_real, double W_imag, int M)
{
    m_Areal = A_real;
    m_Aimag = A_imag;
    m_Wreal = W_real;
    m_Wimag = W_imag;
    m_M = qMax(1, M);
}

/**
 * @brief 执行Chirp-Z变换
 * @param real 输入信号实部
 * @param imag 输入信号虚部
 * @return 输出频谱的幅度谱（大小为M）
 *
 * 使用Bluestein算法: X[k] = sum_n x[n] * A^{-n} * W^{nk}
 * 通过将nk展开为 (n^2 + k^2 - (k-n)^2)/2 实现卷积计算。
 */
QVector<double> ChirpZ3::transform(const QVector<double>& real,
                                     const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    const int N = real.size();
    const int M = m_M;

    if (N == 0 || M == 0) {
        return QVector<double>();
    }

    /* 卷积长度: >= N + M - 1 且为2的幂 */
    const int convLen = nextPow2(N + M - 1);

    /* ---- 构造 chirp 序列 y[n] = W^{n^2/2} ---- */
    QVector<double> yReal(convLen, 0.0);
    QVector<double> yImag(convLen, 0.0);
    for (int n = 0; n < N + M - 1; ++n) {
        double angle = 0.0;
        /* 计算 n^2/2 * arg(W) */
        double wMag = qSqrt(m_Wreal * m_Wreal + m_Wimag * m_Wimag);
        double wArg = qAtan2(m_Wimag, m_Wreal);
        angle = n * n / 2.0 * wArg;

        if (wMag > 0) {
            double wMagN2 = qPow(wMag, n * n / 2.0);
            yReal[n] = wMagN2 * qCos(angle);
            yImag[n] = wMagN2 * qSin(angle);
        }
    }

    /* ---- 构造输入序列: a[n] = x[n] * A^{-n} * W^{n^2/2} ---- */
    QVector<double> aReal(convLen, 0.0);
    QVector<double> aImag(convLen, 0.0);

    double aMag = qSqrt(m_Areal * m_Areal + m_Aimag * m_Aimag);
    double aArg = qAtan2(m_Aimag, m_Areal);
    double wMag = qSqrt(m_Wreal * m_Wreal + m_Wimag * m_Wimag);
    double wArg = qAtan2(m_Wimag, m_Wreal);

    for (int n = 0; n < N; ++n) {
        /* A^{-n} */
        double aInvMag = (aMag > 1e-12) ? qPow(aMag, -n) : 0.0;
        double aInvArg = -n * aArg;

        /* W^{n^2/2} */
        double wMagN2 = (wMag > 0) ? qPow(wMag, n * n / 2.0) : 1.0;
        double wArgN2 = n * n / 2.0 * wArg;

        double combinedMag = aInvMag * wMagN2;
        double combinedArg = aInvArg + wArgN2;

        double factorReal = combinedMag * qCos(combinedArg);
        double factorImag = combinedMag * qSin(combinedArg);

        aReal[n] = real[n] * factorReal - imag[n] * factorImag;
        aImag[n] = real[n] * factorImag + imag[n] * factorReal;
    }

    /* ---- 卷积: a * y (通过FFT) ---- */
    fft(aReal, aImag, convLen);
    fft(yReal, yImag, convLen);

    QVector<double> convReal(convLen, 0.0);
    QVector<double> convImag(convLen, 0.0);
    for (int i = 0; i < convLen; ++i) {
        convReal[i] = aReal[i] * yReal[i] - aImag[i] * yImag[i];
        convImag[i] = aReal[i] * yImag[i] + aImag[i] * yReal[i];
    }

    /* IFFT */
    for (int i = 0; i < convLen; ++i) {
        convImag[i] = -convImag[i];
    }
    fft(convReal, convImag, convLen);
    for (int i = 0; i < convLen; ++i) {
        convReal[i] /= convLen;
        convImag[i] = -convImag[i] / convLen;
    }

    /* ---- 提取结果: X[k] = conv[k] * W^{k^2/2} ---- */
    QVector<double> magnitude(M, 0.0);
    for (int k = 0; k < M; ++k) {
        double wMagK2 = (wMag > 0) ? qPow(wMag, k * k / 2.0) : 1.0;
        double wArgK2 = k * k / 2.0 * wArg;

        double wReal = wMagK2 * qCos(wArgK2);
        double wImag = wMagK2 * qSin(wArgK2);

        double xReal = convReal[k] * wReal - convImag[k] * wImag;
        double xImag = convReal[k] * wImag + convImag[k] * wReal;

        magnitude[k] = qSqrt(xReal * xReal + xImag * xImag);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.totalPointsProcessed += N;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, M);
    return magnitude;
}

/**
 * @brief 获取CZT对应的频率轴
 * @return 频率向量(Hz)
 */
QVector<double> ChirpZ3::frequencies() const
{
    QVector<double> freq(m_M, 0.0);
    /* 对于标准单位圆CZT: f[k] = arg(A * W^k) / (2*pi) * sampleRate */
    /* 简化: 假设单位圆上等间隔采样 */
    for (int k = 0; k < m_M; ++k) {
        double wArg = qAtan2(m_Wimag, m_Wreal);
        double aArg = qAtan2(m_Aimag, m_Areal);
        freq[k] = (aArg + k * wArg) / (2.0 * M_PI) * 44100.0;
    }
    return freq;
}

/**
 * @brief 频谱精细化缩放
 * @param fLow 低频边界(Hz)
 * @param fHigh 高频边界(Hz)
 * @param numPoints 输出点数
 * @param sampleRate 采样率(Hz)
 * @return 缩放区域的频谱幅度
 *
 * 在指定频率范围内进行高分辨率CZT分析。
 */
QVector<double> ChirpZ3::zoom(double fLow, double fHigh, int numPoints,
                                double sampleRate)
{
    /* 计算CZT参数: A = e^{j*2*pi*fLow/fs} */
    double aArg = 2.0 * M_PI * fLow / sampleRate;
    double A_r = qCos(aArg);
    double A_i = qSin(aArg);

    /* W = e^{j*2*pi*(fHigh-fLow)/(numPoints*fs)} */
    double wArg = 2.0 * M_PI * (fHigh - fLow) / (numPoints * sampleRate);
    double W_r = qCos(wArg);
    double W_i = qSin(wArg);

    setParameters(A_r, A_i, W_r, W_i, numPoints);

    /* 使用单位脉冲作为输入（频谱分析模式） */
    QVector<double> realIn(numPoints, 0.0);
    QVector<double> imagIn(numPoints, 0.0);
    realIn[0] = 1.0;

    return transform(realIn, imagIn);
}

/**
 * @brief 基2 FFT实现
 * @param real 实部数组
 * @param imag 虚部数组
 * @param n FFT点数（必须为2的幂）
 */
void ChirpZ3::fft(QVector<double>& real, QVector<double>& imag, int n) const
{
    /* 位反转 */
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
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle);
        double wI = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curR = 1.0, curI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tR = curR * real[v] - curI * imag[v];
                double tI = curR * imag[v] + curI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double newR = curR * wR - curI * wI;
                curI = curR * wI + curI * wR;
                curR = newR;
            }
        }
    }
}

/**
 * @brief 计算大于等于n的最小2的幂
 * @param n 输入值
 * @return >= n 的最小2的幂
 */
int ChirpZ3::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/**
 * @brief 重置所有统计信息
 */
void ChirpZ3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
