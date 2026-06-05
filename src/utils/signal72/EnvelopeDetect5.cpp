/**
 * @file EnvelopeDetect5.cpp
 * @brief 包络检测器实现
 *
 * 实现基于Hilbert变换和峰值检测的信号包络提取，
 * 支持攻击/释放时间常数和RMS电平计算。
 */

#include "utils/signal72/EnvelopeDetect5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
EnvelopeDetect5::EnvelopeDetect5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)
 */
void EnvelopeDetect5::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间(ms)
 */
void EnvelopeDetect5::setAttackTime(double ms)
{
    m_attack = qBound(0.01, ms, 1000.0);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间(ms)
 */
void EnvelopeDetect5::setReleaseTime(double ms)
{
    m_release = qBound(0.1, ms, 10000.0);
}

/**
 * @brief 设置检测方法
 * @param method 方法名称："hilbert"（Hilbert变换）或 "peak"（峰值检测）
 */
void EnvelopeDetect5::setMethod(const QString& method)
{
    if (method == "hilbert" || method == "peak") {
        m_method = method;
    }
}

/**
 * @brief 检测信号包络
 * @param signal 输入信号
 * @return 包络曲线
 */
QVector<double> EnvelopeDetect5::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> envelope;
    if (signal.isEmpty()) return envelope;

    const int N = signal.size();

    if (m_method == "hilbert") {
        envelope = hilbertEnvelope(signal);
    } else {
        envelope = peakEnvelope(signal);
    }

    // 计算峰值和RMS
    m_peak = 0.0;
    double sumSq = 0.0;
    for (int i = 0; i < envelope.size(); ++i) {
        if (envelope[i] > m_peak) m_peak = envelope[i];
        sumSq += envelope[i] * envelope[i];
    }
    m_rms = (envelope.size() > 0) ? qSqrt(sumSq / envelope.size()) : 0.0;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalFrames += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detected(m_peak, m_rms);
    return envelope;
}

/**
 * @brief 重置统计信息
 */
void EnvelopeDetect5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Hilbert变换包络检测
 * @param sig 输入信号
 * @return 包络（解析信号的幅度）
 *
 * 通过简化的FIR Hilbert滤波器实现正交分量提取，
 * 然后计算解析信号的幅度。
 */
QVector<double> EnvelopeDetect5::hilbertEnvelope(const QVector<double>& sig)
{
    const int N = sig.size();
    QVector<double> env(N, 0.0);

    // 简化Hilbert变换：使用频域方法
    // 构造解析信号 a[n] = x[n] + j * H{x}[n]
    // 使用FIR近似Hilbert滤波器

    int filterLen = qMin(63, N / 4);
    if (filterLen % 2 == 0) filterLen++;
    int halfLen = filterLen / 2;

    // 设计Hilbert FIR滤波器系数
    QVector<double> h(filterLen, 0.0);
    for (int n = 0; n < filterLen; ++n) {
        int k = n - halfLen;
        if (k == 0) {
            h[n] = 0.0;
        } else {
            // 理想Hilbert: h[k] = 2/(pi*k) for odd k, 0 for even k
            if (k % 2 != 0) {
                h[n] = 2.0 / (M_PI * k);
            }
        }
        // 加窗
        double win = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (filterLen - 1));
        h[n] *= win;
    }

    // 卷积计算正交分量
    for (int i = 0; i < N; ++i) {
        double re = sig[i];
        double im = 0.0;
        for (int j = 0; j < filterLen; ++j) {
            int idx = i - j + halfLen;
            if (idx >= 0 && idx < N) {
                im += h[j] * sig[idx];
            }
        }
        env[i] = qSqrt(re * re + im * im);
    }

    return env;
}

/**
 * @brief 峰值包络检测
 * @param sig 输入信号
 * @return 峰值保持包络
 *
 * 使用攻击/释放时间常数的简单峰值检测器。
 */
QVector<double> EnvelopeDetect5::peakEnvelope(const QVector<double>& sig)
{
    const int N = sig.size();
    QVector<double> env(N, 0.0);

    double attackCoeff = qExp(-1.0 / (m_sampleRate * m_attack / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_sampleRate * m_release / 1000.0));
    double level = 0.0;

    for (int i = 0; i < N; ++i) {
        double absVal = qAbs(sig[i]);
        if (absVal > level) {
            level = attackCoeff * level + (1.0 - attackCoeff) * absVal;
        } else {
            level = releaseCoeff * level + (1.0 - releaseCoeff) * absVal;
        }
        env[i] = level;
    }

    return env;
}
