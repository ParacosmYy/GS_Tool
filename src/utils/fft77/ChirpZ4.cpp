/**
 * @file ChirpZ4.cpp
 * @brief Chirp-Z变换实现 — Bluestein算法，任意螺旋轮廓频谱分析
 *
 * Chirp-Z变换(CZT)允许在Z平面任意螺旋轮廓上计算频率响应，
 * 比标准DFT更灵活。本实现基于Bluestein算法，将CZT转化为
 * 线性卷积问题，利用补零到2的幂次后通过FFT高效求解。
 * 支持频段细化(zoom)功能，在指定频率范围内获得高分辨率分析。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/fft77/ChirpZ4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <complex>

/**
 * @brief 构造函数，初始化默认CZT参数
 * @param parent 父QObject指针
 *
 * 默认参数：A=1(起始点)，W=e^{-j*2*pi/N}(单位圆等间距)，
 * 等效于标准DFT。
 */
ChirpZ4::ChirpZ4(QObject* parent)
    : QObject(parent)
    , m_A(1.0, 0.0)
    , m_W(1.0, 0.0)
{
}

/**
 * @brief 设置CZT变换参数
 * @param outputPoints 输出频率点数M
 * @param A 起始复数点(确定起始频率位置)
 * @param W 相邻采样点的复数比值(确定螺旋率和间距)
 * @return 参数是否合法
 *
 * A和W定义了Z平面上的采样轮廓:
 * z_k = A * W^{-k}, k = 0, 1, ..., M-1
 * 单位圆上的CZT: A = e^{j*theta_0}, W = e^{-j*delta_theta}
 */
bool ChirpZ4::setParameters(int outputPoints, const std::complex<double>& A,
                             const std::complex<double>& W)
{
    if (outputPoints <= 0) {
        return false;
    }
    m_outputPoints = outputPoints;
    m_A = A;
    m_W = W;
    return true;
}

/**
 * @brief 执行Chirp-Z变换
 * @param input 输入实数信号
 * @return 变换结果，复数频谱向量(长度为M)
 *
 * 使用Bluestein算法:
 * 1. 构造 chirp 序列: y[n] = W^{n^2/2}
 * 2. 构造输入序列: a[n] = x[n] * A^{-n} * W^{n^2/2}
 * 3. 通过FFT计算线性卷积 a * y
 * 4. 结果乘以 W^{k^2/2} 得到最终CZT输出
 */
QVector<std::complex<double>> ChirpZ4::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int N = input.size();
    const int M = m_outputPoints;

    QVector<std::complex<double>> result;
    if (N == 0 || M == 0) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
            ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(0);
        return result;
    }

    result.resize(M, std::complex<double>(0.0, 0.0));

    /* 卷积长度: >= N + M - 1 且为2的幂 */
    const int convLen = nextPow2(N + M - 1);

    /* 构造 chirp 序列: y[n] = W^{n^2/2}，范围 0 ~ N+M-2 */
    QVector<std::complex<double>> y(convLen, std::complex<double>(0.0, 0.0));
    for (int n = 0; n < N + M - 1; ++n) {
        double angle = static_cast<double>(n * n) / 2.0 * std::arg(m_W);
        double mag = std::pow(std::abs(m_W), static_cast<double>(n * n) / 2.0);
        y[n] = std::polar(mag, angle);
    }

    /* 构造输入序列: a[n] = x[n] * A^{-n} * W^{n^2/2}，补零到 convLen */
    QVector<std::complex<double>> a(convLen, std::complex<double>(0.0, 0.0));
    for (int n = 0; n < N; ++n) {
        double nHalf = static_cast<double>(n * n) / 2.0;
        std::complex<double> aInvN = std::pow(std::complex<double>(1.0, 0.0) / m_A,
                                               static_cast<double>(n));
        std::complex<double> wN2 = std::polar(std::pow(std::abs(m_W), nHalf),
                                               nHalf * std::arg(m_W));
        a[n] = input[n] * aInvN * wN2;
    }

    /* 通过FFT计算卷积: conv = IFFT(FFT(a) * FFT(y)) */
    fftInplace(a, false);
    fftInplace(y, false);

    QVector<std::complex<double>> conv(convLen);
    for (int i = 0; i < convLen; ++i) {
        conv[i] = a[i] * y[i];
    }

    fftInplace(conv, true);

    /* 提取前M个点并乘以 W^{k^2/2} */
    for (int k = 0; k < M; ++k) {
        double kHalf = static_cast<double>(k * k) / 2.0;
        std::complex<double> wK2 = std::polar(std::pow(std::abs(m_W), kHalf),
                                               kHalf * std::arg(m_W));
        result[k] = conv[k] * wK2;
    }

    /* 更新统计信息 */
    m_stats.totalTransforms++;
    m_stats.totalOutputPoints += M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(M);
    return result;
}

/**
 * @brief 在指定频率范围内进行细化分析
 * @param freqStart 起始频率(归一化频率，0~1)
 * @param freqEnd 结束频率(归一化频率，0~1)
 * @param numPoints 输出频率点数
 * @param input 输入实数信号
 * @return 细化频谱复数结果
 *
 * 自动计算A和W参数，使CZT采样点落在[freqStart, freqEnd]区间内:
 * A = e^{j*2*pi*freqStart}
 * W = e^{-j*2*pi*(freqEnd-freqStart)/numPoints}
 */
QVector<std::complex<double>> ChirpZ4::zoom(double freqStart, double freqEnd,
                                             int numPoints, const QVector<double>& input)
{
    if (freqStart >= freqEnd || numPoints <= 0) {
        return QVector<std::complex<double>>();
    }

    /* A = e^{j*2*pi*fStart}，位于单位圆上freqStart对应角度 */
    std::complex<double> A = std::polar(1.0, 2.0 * M_PI * freqStart);

    /* W = e^{-j*2*pi*(fEnd-fStart)/M}，单位圆上等间距逆时针 */
    double deltaFreq = (freqEnd - freqStart) / numPoints;
    std::complex<double> W = std::polar(1.0, -2.0 * M_PI * deltaFreq);

    setParameters(numPoints, A, W);
    return transform(input);
}

/**
 * @brief 重置所有累计统计信息
 */
void ChirpZ4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 原地FFT实现(Cooley-Tukey基2算法)
 * @param data 复数数组(长度必须为2的幂)
 * @param inverse true执行IFFT，false执行FFT
 *
 * 使用位反转排列 + 蝶形运算，时间复杂度O(N log N)。
 */
void ChirpZ4::fftInplace(QVector<std::complex<double>>& data, bool inverse) const
{
    const int n = data.size();
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
            std::swap(data[i], data[j]);
        }
    }

    /* 蝶形运算 */
    const double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = sign * 2.0 * M_PI / len;
        std::complex<double> wLen(std::cos(angle), std::sin(angle));

        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> u = data[i + j];
                std::complex<double> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wLen;
            }
        }
    }

    /* 逆变换需要除以N */
    if (inverse) {
        for (int i = 0; i < n; ++i) {
            data[i] /= n;
        }
    }
}

/**
 * @brief 计算大于等于n的最小2的幂
 * @param n 输入值
 * @return >= n 的最小2的幂
 */
int ChirpZ4::nextPow2(int n) const
{
    int p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}
