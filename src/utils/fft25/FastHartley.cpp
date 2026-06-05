/**
 * @file FastHartley.cpp
 * @brief 快速Hartley变换(FHT)实现 — 原地蝶形+比特反转+卷积
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft25/FastHartley.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
FastHartley::FastHartley(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 正向FHT变换
 *
 * 对实数序列执行快速Hartley变换。
 * Hartley变换结果H(k) = sum_n x(n)*cas(2*pi*n*k/N),
 * 其中cas(theta) = cos(theta) + sin(theta)。
 *
 * @param data 输入数据(长度必须为2的幂)
 * @return Hartley变换结果
 */
QVector<double> FastHartley::forward(const QVector<double>& data)
{
    if (data.isEmpty()) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    fhtInPlace(result);

    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += data.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalTransforms + m_stats.totalInverseTransforms);

    emit transformCompleted(data.size());
    return result;
}

/**
 * @brief 逆向FHT变换
 *
 * FHT是自逆变换: 正变换和逆变换结构相同,
 * 仅需除以N即可得到原信号。
 *
 * @param spectrum Hartley谱
 * @return 重建信号
 */
QVector<double> FastHartley::inverse(const QVector<double>& spectrum)
{
    if (spectrum.isEmpty()) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    QVector<double> result = spectrum;
    fhtInPlace(result);

    /* 逆变换需除以N */
    double invN = 1.0 / result.size();
    for (auto& val : result) {
        val *= invN;
    }

    m_stats.totalInverseTransforms++;
    m_stats.totalSamplesProcessed += spectrum.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalTransforms + m_stats.totalInverseTransforms);

    return result;
}

/**
 * @brief 从Hartley谱计算功率谱
 *
 * 利用H(k)和H(N-k)的对称性:
 * Re{X(k)} = (H(k) + H(N-k)) / 2
 * Im{X(k)} = (H(k) - H(N-k)) / 2
 * Power(k) = Re^2 + Im^2
 *
 * @param spectrum Hartley谱
 * @return 功率谱
 */
QVector<double> FastHartley::powerSpectrum(const QVector<double>& spectrum) const
{
    int n = spectrum.size();
    if (n == 0) {
        return {};
    }

    QVector<double> power(n / 2 + 1);
    for (int k = 0; k <= n / 2; ++k) {
        int kn = (n - k) % n;
        double re = (spectrum[k] + spectrum[kn]) / 2.0;
        double im = (spectrum[k] - spectrum[kn]) / 2.0;
        power[k] = re * re + im * im;
    }
    return power;
}

/**
 * @brief 从Hartley谱计算幅度和相位
 *
 * 利用H(k)与DFT的关系提取幅度谱和相位谱。
 *
 * @param spectrum Hartley谱
 * @return (幅度谱, 相位谱) 对
 */
QPair<QVector<double>, QVector<double>> FastHartley::magnitudePhase(
    const QVector<double>& spectrum) const
{
    int n = spectrum.size();
    if (n == 0) {
        return {};
    }

    QVector<double> mag(n / 2 + 1);
    QVector<double> phase(n / 2 + 1);

    for (int k = 0; k <= n / 2; ++k) {
        int kn = (n - k) % n;
        double re = (spectrum[k] + spectrum[kn]) / 2.0;
        double im = (spectrum[k] - spectrum[kn]) / 2.0;
        mag[k] = std::sqrt(re * re + im * im);
        phase[k] = std::atan2(im, re);
    }
    return {mag, phase};
}

/**
 * @brief Hartley域卷积
 *
 * 利用Hartley变换实现线性卷积:
 * Conv(a,b)[k] = (H1[k]*H2[k] + H1[k]*H2[N-k]
 *               - H1[N-k]*H2[k] + H1[N-k]*H2[N-k]) / 2
 *
 * @param h1 谱1
 * @param h2 谱2
 * @return 卷积谱(Hartley域)
 */
QVector<double> FastHartley::convolve(const QVector<double>& h1,
                                       const QVector<double>& h2) const
{
    int n = h1.size();
    if (n != h2.size() || n == 0) {
        return {};
    }

    QVector<double> result(n);
    for (int k = 0; k < n; ++k) {
        int kn = (n - k) % n;
        result[k] = (h1[k] * h2[k] + h1[k] * h2[kn]
                    - h1[kn] * h2[k] + h1[kn] * h2[kn]) / 2.0;
    }
    return result;
}

/**
 * @brief Hartley域自相关
 *
 * 自相关 = IHT(PowerSpectrum), 等价于信号与其自身的互相关。
 *
 * @param spectrum Hartley谱
 * @return 自相关结果
 */
QVector<double> FastHartley::autocorrelate(const QVector<double>& spectrum) const
{
    int n = spectrum.size();
    if (n == 0) {
        return {};
    }

    /* 计算功率谱(等效于H(k)*H(N-k)的Hartley卷积) */
    QVector<double> power(n);
    for (int k = 0; k < n; ++k) {
        int kn = (n - k) % n;
        double re = (spectrum[k] + spectrum[kn]) / 2.0;
        double im = (spectrum[k] - spectrum[kn]) / 2.0;
        power[k] = (re * re + im * im);
    }

    /* 对功率谱做逆FHT得到自相关 */
    FastHartley temp;
    QVector<double> result = temp.inverse(power);
    return result;
}

/**
 * @brief 原地快速Hartley变换
 *
 * 基2 FHT算法:
 * 1. 比特反转重排
 * 2. 逐级蝶形运算,使用cas(a+b)=cas(a)*cos(b)+cas(N-a)*sin(b)性质
 *
 * @param data 数据(长度必须为2的幂,原地修改)
 */
void FastHartley::fhtInPlace(QVector<double>& data) const
{
    int n = data.size();
    if (n <= 1) {
        return;
    }

    /* 比特反转重排 */
    int j = 0;
    for (int i = 1; i < n; ++i) {
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

    /* 逐级蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        int halfLen = len >> 1;
        double angle = -2.0 * M_PI / len;

        for (int i = 0; i < n; i += len) {
            for (int k = 0; k < halfLen; ++k) {
                int idx1 = i + k;
                int idx2 = i + k + halfLen;

                double cosVal = std::cos(angle * k);
                double sinVal = std::sin(angle * k);
                int reflIdx = (n - k) % n;

                /* cas蝶形: cas(a+b) = cas(a)*cos(b) + cas(N-a)*sin(b)
                 * 使用临时索引获取反射项 */
                double reflData = data[(i + halfLen - k) % n + i / len * len];
                if (k == 0) {
                    reflData = data[idx2];
                }

                double casProd;
                if (k == 0) {
                    casProd = data[idx2];
                } else {
                    int mirrorIdx2 = i + halfLen - k;
                    if (mirrorIdx2 < i) {
                        mirrorIdx2 += halfLen;
                    }
                    casProd = data[idx2] * cosVal + data[mirrorIdx2] * sinVal;
                }

                double temp;
                if (k == 0) {
                    temp = data[idx2];
                } else {
                    int mirrorK = len - k;
                    int mirrorIdx = i + mirrorK;
                    if (mirrorIdx >= i + len) {
                        mirrorIdx -= len;
                    }
                    /* 简化的cas蝶形 */
                    temp = data[idx2] * (cosVal + sinVal);
                }

                double oldEven = data[idx1];
                if (k == 0) {
                    data[idx1] = oldEven + data[idx2];
                    data[idx2] = oldEven - data[idx2];
                } else {
                    double cs = cosVal + sinVal;
                    double cm = cosVal - sinVal;
                    data[idx1] = oldEven + data[idx2] * cs
                                + data[i + len - k] * cm;
                    data[idx2] = oldEven - data[idx2] * cs
                                - data[i + len - k] * cm;
                }
            }
        }
    }
}

/**
 * @brief 重置统计信息
 */
void FastHartley::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
