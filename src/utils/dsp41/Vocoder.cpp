/**
 * @file Vocoder.cpp
 * @brief 声码器实现 — 通道声码器+载波调制
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp41/Vocoder.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化滤波器组
 * @param numChannels 通道数
 * @param parent 父对象
 */
Vocoder::Vocoder(int numChannels, QObject* parent)
    : QObject(parent)
    , m_numChannels(qMax(2, numChannels))
{
    setObjectName(QStringLiteral("Vocoder"));
    designFilterBank();
}

/**
 * @brief 设置采样率
 *
 * 更改采样率后需要重新设计滤波器组的系数。
 *
 * @param sampleRate 采样率（Hz）
 */
void Vocoder::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(8000.0, sampleRate);
    designFilterBank();
}

/**
 * @brief 设置通道数
 *
 * 重新设计对应数量的带通滤波器组。
 *
 * @param channels 通道数（最小为2）
 */
void Vocoder::setNumChannels(int channels)
{
    m_numChannels = qMax(2, channels);
    designFilterBank();
}

/**
 * @brief 处理调制器和载波信号
 *
 * 对调制器（通常是语音）和载波（通常是合成音）分别通过带通滤波器组。
 * 提取调制器各通道的包络，乘以载波对应通道的滤波输出，叠加合成。
 *
 * @param modulator 调制器信号（语音）
 * @param carrier 载波信号（合成音）
 * @return 声码器合成输出
 */
QVector<double> Vocoder::process(const QVector<double>& modulator,
                                  const QVector<double>& carrier)
{
    QElapsedTimer timer;
    timer.start();

    int len = qMin(modulator.size(), carrier.size());
    if (len == 0) return {};

    QVector<double> output(len, 0.0);

    /* 重置通道能量 */
    m_channelEnergy.fill(0.0, m_numChannels);

    for (int ch = 0; ch < m_numChannels; ++ch) {
        /* 调制器带通滤波 */
        QVector<double> modBP = bandpass(modulator, ch);

        /* 载波带通滤波 */
        QVector<double> carBP = bandpass(carrier, ch);

        /* 提取调制器包络（全波整流+平滑） */
        double alpha = 0.95;
        m_envelopeState[ch] = 0.0;
        double channelEnergy = 0.0;

        for (int i = 0; i < len; ++i) {
            double env = qFabs(modBP[i]);
            m_envelopeState[ch] = alpha * m_envelopeState[ch] + (1.0 - alpha) * env;

            /* 包络调制载波 */
            output[i] += m_envelopeState[ch] * carBP[i];
            channelEnergy += output[i] * output[i];
        }

        m_channelEnergy[ch] = qSqrt(channelEnergy / qMax(1, len));
    }

    /* 归一化输出 */
    double maxVal = 0.0;
    for (int i = 0; i < len; ++i) {
        maxVal = qMax(maxVal, qFabs(output[i]));
    }
    if (maxVal > 1e-6) {
        double scale = 1.0 / maxVal;
        for (int i = 0; i < len; ++i) {
            output[i] *= scale;
        }
    }

    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += len;
    m_stats.numChannels = m_numChannels;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    double totalEnergy = 0.0;
    for (double e : m_channelEnergy) totalEnergy += e;

    emit processingCompleted(len, totalEnergy);
    return output;
}

/**
 * @brief 获取指定通道的包络
 *
 * 返回上一次处理过程中各通道的包络状态值。
 *
 * @param channel 通道索引
 * @return 该通道的包络值（单点，非时间序列）
 */
QVector<double> Vocoder::envelope(int channel) const
{
    if (channel < 0 || channel >= m_numChannels) return {};
    return {m_envelopeState[channel]};
}

/**
 * @brief 获取所有通道的能量分布
 * @return 每个通道的RMS能量
 */
QVector<double> Vocoder::channelEnergies() const
{
    return m_channelEnergy;
}

/**
 * @brief 设计带通滤波器组
 *
 * 将频率范围均匀分为m_numChannels个频段。
 * 每个频段使用二阶IIR带通滤波器。
 * 滤波器系数通过双线性变换从模拟原型获得。
 */
void Vocoder::designFilterBank()
{
    double fMin = 80.0;
    double fMax = m_sampleRate / 2.0 - 100.0;
    double bw = (fMax - fMin) / m_numChannels;

    m_freqLow.resize(m_numChannels);
    m_freqHigh.resize(m_numChannels);
    m_bpCoeffsA.resize(m_numChannels);
    m_bpCoeffsB.resize(m_numChannels);
    m_envelopeState.fill(0.0, m_numChannels);
    m_channelEnergy.fill(0.0, m_numChannels);

    for (int ch = 0; ch < m_numChannels; ++ch) {
        m_freqLow[ch] = fMin + ch * bw;
        m_freqHigh[ch] = fMin + (ch + 1) * bw;

        double fCenter = (m_freqLow[ch] + m_freqHigh[ch]) / 2.0;
        double Q = fCenter / qMax(1.0, bw);

        /* 二阶带通滤波器系数 */
        double w0 = 2.0 * M_PI * fCenter / m_sampleRate;
        double alphaF = qSin(w0) / (2.0 * Q);

        double b0 = alphaF;
        double b1 = 0.0;
        double b2 = -alphaF;
        double a0 = 1.0 + alphaF;
        double a1 = -2.0 * qCos(w0);
        double a2 = 1.0 - alphaF;

        m_bpCoeffsB[ch] = {b0 / a0, b1 / a0, b2 / a0};
        m_bpCoeffsA[ch] = {1.0, a1 / a0, a2 / a0};
    }

    m_stats.numChannels = m_numChannels;
}

/**
 * @brief 对输入信号执行指定通道的带通滤波
 *
 * 使用二阶IIR直接II型结构实现带通滤波。
 * 维护滤波器状态以支持逐块处理。
 *
 * @param input 输入信号
 * @param ch 通道索引
 * @return 滤波后信号
 */
QVector<double> Vocoder::bandpass(const QVector<double>& input, int ch)
{
    int len = input.size();
    QVector<double> output(len, 0.0);

    if (ch < 0 || ch >= m_bpCoeffsB.size()) return output;

    const auto& b = m_bpCoeffsB[ch];
    const auto& a = m_bpCoeffsA[ch];

    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;

    for (int i = 0; i < len; ++i) {
        double x0 = input[i];
        double y0 = b[0] * x0 + b[1] * x1 + b[2] * x2 - a[1] * y1 - a[2] * y2;
        output[i] = y0;

        x2 = x1; x1 = x0;
        y2 = y1; y1 = y0;
    }

    return output;
}

/**
 * @brief 重置所有统计数据
 */
void Vocoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
