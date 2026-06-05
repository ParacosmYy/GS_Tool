/**
 * @file HilbertTransform2.cpp
 * @brief 希尔伯特变换增强实现 — FIR设计/频域解析/瞬时频率/包络提取
 */

#include "utils/fft31/HilbertTransform2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstring>

/** @brief 构造函数 @param parent 父对象 */
HilbertTransform2::HilbertTransform2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置FIR滤波器长度 @param length 长度(必须是偶数) */
void HilbertTransform2::setFilterLength(int length)
{
    m_filterLength = qMax(4, length | 1); /* 确保奇数 */
    if (m_filterLength % 2 == 0) m_filterLength++;
}

/** @brief 计算解析信号 @param input 实信号 @return 解析信号的虚部(希尔伯特变换结果) */
QVector<double> HilbertTransform2::analyticalSignal(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    /* 找到大于等于n的2的幂次 */
    int N = 1;
    while (N < n) N <<= 1;

    QVector<double> real(N, 0.0);
    QVector<double> imag(N, 0.0);
    for (int i = 0; i < n; ++i) {
        real[i] = input[i];
    }

    /* 正向FFT */
    fft(real, imag);

    /* 构造解析信号频谱: 正频率*2, 负频率*0, DC和Nyquist不变 */
    for (int i = 1; i < N / 2; ++i) {
        real[i] *= 2.0;
        imag[i] *= 2.0;
    }
    for (int i = N / 2 + 1; i < N; ++i) {
        real[i] = 0.0;
        imag[i] = 0.0;
    }

    /* 逆FFT (通过共轭+正变换+共轭实现) */
    for (int i = 0; i < N; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < N; ++i) {
        real[i] /= N;
        imag[i] = -imag[i] / N;
    }

    /* 返回解析信号的虚部即希尔伯特变换 */
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = imag[i];
    }

    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalTransforms));

    emit transformComplete(n);
    return result;
}

/** @brief 计算信号包络(瞬时幅度) @param input 实信号 @return 包络 */
QVector<double> HilbertTransform2::envelope(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> hilbert = analyticalSignal(input);
    QVector<double> env(input.size());

    for (int i = 0; i < input.size(); ++i) {
        env[i] = qSqrt(input[i] * input[i] + hilbert[i] * hilbert[i]);
    }

    m_timeSum += timer.elapsed();
    return env;
}

/** @brief 计算瞬时相位 @param input 实信号 @return 瞬时相位(弧度) */
QVector<double> HilbertTransform2::instantaneousPhase(const QVector<double>& input)
{
    QVector<double> hilbert = analyticalSignal(input);
    QVector<double> phase(input.size());

    for (int i = 0; i < input.size(); ++i) {
        phase[i] = qAtan2(hilbert[i], input[i]);
    }
    return phase;
}

/** @brief 计算瞬时频率 @param input 实信号 @param sampleRate 采样率 @return 瞬时频率(Hz) */
QVector<double> HilbertTransform2::instantaneousFrequency(
    const QVector<double>& input, double sampleRate)
{
    QVector<double> phase = instantaneousPhase(input);
    QVector<double> freq(input.size(), 0.0);

    if (input.size() < 2) return freq;

    /* 展开相位并差分求瞬时频率 */
    QVector<double> unwrapped(input.size());
    unwrapped[0] = phase[0];
    for (int i = 1; i < phase.size(); ++i) {
        double diff = phase[i] - phase[i - 1];
        /* 相位展开 */
        while (diff > M_PI) diff -= 2.0 * M_PI;
        while (diff < -M_PI) diff += 2.0 * M_PI;
        unwrapped[i] = unwrapped[i - 1] + diff;
    }

    for (int i = 1; i < unwrapped.size(); ++i) {
        freq[i] = (unwrapped[i] - unwrapped[i - 1]) * sampleRate
            / (2.0 * M_PI);
    }
    if (!freq.isEmpty()) freq[0] = freq.size() > 1 ? freq[1] : 0.0;

    return freq;
}

/** @brief 设计希尔伯特FIR滤波器 @param length 长度 @return 滤波器系数 */
QVector<double> HilbertTransform2::designHilbertFIR(int length) const
{
    int len = qMax(3, length | 1);
    if (len % 2 == 0) len++;
    int M = (len - 1) / 2;

    QVector<double> h(len, 0.0);

    for (int n = -M; n <= M; ++n) {
        if (n == 0) {
            h[n + M] = 0.0;
        } else {
            /* 理想希尔伯特变换器的冲击响应 */
            double ideal = (1.0 - qCos(M_PI * n)) / (M_PI * n);
            /* Blackman窗 */
            double w = 0.42 + 0.5 * qCos(2.0 * M_PI * n / (len - 1))
                + 0.08 * qCos(4.0 * M_PI * n / (len - 1));
            h[n + M] = ideal * w;
        }
    }
    return h;
}

/** @brief 原地FFT(Cooley-Tukey) @param real 实部 @param imag 虚部 */
void HilbertTransform2::fft(QVector<double>& real, QVector<double>& imag)
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
        double wReal = qCos(angle);
        double wImag = qSin(angle);

        for (int i = 0; i < N; i += len) {
            double curReal = 1.0;
            double curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];
                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;
                double newCurReal = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
            }
        }
    }
}

/** @brief 重置统计 */
void HilbertTransform2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
