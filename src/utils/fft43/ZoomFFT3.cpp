/**
 * @file ZoomFFT3.cpp
 * @brief Zoom-FFT3实现 — 多级细化+带通下采样
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft43/ZoomFFT3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
ZoomFFT3::ZoomFFT3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ZoomFFT3"));
}

/**
 * @brief 设置细化分析的频率范围
 *
 * 指定中心频率和带宽，系统自动设计带通滤波器
 * 和下采样参数以实现频率细化。
 *
 * @param centerFreq 中心频率（Hz）
 * @param bandwidth 分析带宽（Hz）
 * @param sampleRate 采样率（Hz）
 */
void ZoomFFT3::setZoomRange(double centerFreq, double bandwidth, double sampleRate)
{
    m_centerFreq = qMax(0.0, centerFreq);
    m_bandwidth = qMax(1.0, bandwidth);
    m_sampleRate = qMax(m_bandwidth * 2.0, sampleRate);
    m_initialized = false;
}

/**
 * @brief 设置细化倍数
 *
 * 细化倍数决定了输出分辨率相对于标准FFT的提升量。
 * 例如factor=8表示频率分辨率提升8倍。
 *
 * @param factor 细化倍数（最小为2）
 */
void ZoomFFT3::setZoomFactor(int factor)
{
    m_zoomFactor = qMax(2, factor);
    m_initialized = false;
}

/**
 * @brief 执行Zoom-FFT变换
 *
 * 处理流程: 频率搬移 -> 带通滤波 -> 下采样 -> FFT。
 * 输出仅在目标频段内具有高分辨率，其余频段被忽略。
 *
 * @param signal 输入信号
 * @return 细化频谱（复数幅度）
 */
QVector<double> ZoomFFT3::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return {};

    /* 懒初始化滤波器 */
    if (!m_initialized) {
        designBandpass();
        m_initialized = true;
    }

    /* 1. 频率搬移：乘以exp(-j*2*pi*fc*t)将中心频率移到直流 */
    int N = signal.size();
    QVector<double> shifted(N);
    double omega = 2.0 * M_PI * m_centerFreq / m_sampleRate;
    for (int i = 0; i < N; ++i) {
        /* 取实部即可（等效于复数乘法取实部） */
        shifted[i] = signal[i] * qCos(omega * i);
    }

    /* 2. 带通滤波 */
    QVector<double> filtered = bandpassFilter(shifted);

    /* 3. 下采样 */
    QVector<double> decimated = decimate(filtered, m_zoomFactor);

    /* 4. 对下采样信号做FFT（简化DFT） */
    int M = decimated.size();
    m_outputSize = M / 2;
    QVector<double> spectrum(m_outputSize);

    for (int k = 0; k < m_outputSize; ++k) {
        double real = 0.0, imag = 0.0;
        double freq = 2.0 * M_PI * k / M;
        for (int n = 0; n < M; ++n) {
            real += decimated[n] * qCos(freq * n);
            imag -= decimated[n] * qSin(freq * n);
        }
        spectrum[k] = qSqrt(real * real + imag * imag) / M * 2.0;
    }

    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += signal.size();
    m_stats.zoomFactor = m_zoomFactor;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(signal.size(), m_outputSize, resolution());
    return spectrum;
}

/**
 * @brief 获取细化频谱对应的频率轴
 *
 * 频率范围从 centerFreq - bandwidth/2 到 centerFreq + bandwidth/2。
 *
 * @return 频率轴向量（Hz）
 */
QVector<double> ZoomFFT3::frequencies() const
{
    QVector<double> freqs(m_outputSize);
    double df = (m_sampleRate / m_zoomFactor) / (2.0 * m_outputSize);
    double fStart = m_centerFreq - m_bandwidth / 2.0;
    for (int i = 0; i < m_outputSize; ++i) {
        freqs[i] = fStart + i * df;
    }
    return freqs;
}

/**
 * @brief 计算频率分辨率
 *
 * 分辨率 = (采样率 / 细化倍数) / 输出点数
 *
 * @return 频率分辨率（Hz）
 */
double ZoomFFT3::resolution() const
{
    if (m_outputSize == 0) return 0.0;
    return (m_sampleRate / m_zoomFactor) / (2.0 * m_outputSize);
}

/**
 * @brief 设计带通滤波器
 *
 * 使用二阶IIR Butterworth带通滤波器，中心频率对准m_centerFreq，
 * 带宽为m_bandwidth。
 */
void ZoomFFT3::designBandpass()
{
    double fLow = m_centerFreq - m_bandwidth / 2.0;
    double fHigh = m_centerFreq + m_bandwidth / 2.0;

    /* 确保频率在有效范围内 */
    fLow = qMax(0.0, fLow);
    fHigh = qMin(m_sampleRate / 2.0, fHigh);

    double fCenter = (fLow + fHigh) / 2.0;
    double bw = fHigh - fLow;
    double Q = (bw > 0) ? fCenter / bw : 1.0;

    /* 二阶IIR带通 */
    double w0 = 2.0 * M_PI * fCenter / m_sampleRate;
    double alphaF = qSin(w0) / (2.0 * Q);

    double b0 = alphaF;
    double b1 = 0.0;
    double b2 = -alphaF;
    double a0 = 1.0 + alphaF;
    double a1 = -2.0 * qCos(w0);
    double a2 = 1.0 - alphaF;

    m_bandpassCoeffs = {b0 / a0, b1 / a0, b2 / a0, 1.0, a1 / a0, a2 / a0};
    m_bandpassState.fill(0.0, 6);
}

/**
 * @brief 对输入信号应用带通滤波
 *
 * 使用直接II型IIR滤波器结构。
 *
 * @param input 输入信号
 * @return 滤波后信号
 */
QVector<double> ZoomFFT3::bandpassFilter(const QVector<double>& input)
{
    int len = input.size();
    QVector<double> output(len, 0.0);

    if (m_bandpassCoeffs.size() < 6) return output;

    double b0 = m_bandpassCoeffs[0], b1 = m_bandpassCoeffs[1], b2 = m_bandpassCoeffs[2];
    double a1 = m_bandpassCoeffs[4], a2 = m_bandpassCoeffs[5];

    double w1 = 0.0, w2 = 0.0;

    for (int i = 0; i < len; ++i) {
        double w0 = input[i] - a1 * w1 - a2 * w2;
        output[i] = b0 * w0 + b1 * w1 + b2 * w2;
        w2 = w1;
        w1 = w0;
    }

    return output;
}

/**
 * @brief 对信号进行整数倍下采样
 *
 * 简单的每隔factor取一个样本。
 * 在下采样前应已完成带通滤波以避免混叠。
 *
 * @param input 输入信号
 * @param factor 下采样因子
 * @return 下采样后信号
 */
QVector<double> ZoomFFT3::decimate(const QVector<double>& input, int factor)
{
    QVector<double> output;
    output.reserve(input.size() / factor);
    for (int i = 0; i < input.size(); i += factor) {
        output.append(input[i]);
    }
    return output;
}

/**
 * @brief 重置所有统计数据
 */
void ZoomFFT3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
