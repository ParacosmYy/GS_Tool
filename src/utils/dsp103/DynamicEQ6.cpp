#include "DynamicEQ6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file DynamicEQ6.cpp
 * @brief 动态均衡器实现
 *
 * 根据输入信号电平自动调节指定频段的增益，
 * 结合参数均衡和动态处理:
 * - 低电平时增强目标频段(提升清晰度)
 * - 高电平时衰减目标频段(去齿音/消除共振)
 */

/**
 * @brief 构造函数，初始化默认EQ参数
 * @param parent 父QObject对象指针
 */
DynamicEQ6::DynamicEQ6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置中心频率
 * @param freq 均衡频段的中心频率(Hz)
 */
void DynamicEQ6::setFreq(double freq)
{
    m_freq = qMax(20.0, freq);
}

/**
 * @brief 设置增益
 * @param gain 最大增益调节量(dB)，正值为增强，负值为衰减
 */
void DynamicEQ6::setGain(double gain)
{
    m_gain = gain;
}

/**
 * @brief 设置Q值
 * @param q 带宽品质因子，值越大频段越窄
 */
void DynamicEQ6::setQ(double q)
{
    m_q = qMax(0.1, q);
}

/**
 * @brief 二阶IIR峰值滤波器系数计算
 * @param freq 中心频率
 * @param gainDb 增益(dB)
 * @param q Q值
 * @param sampleRate 采样率
 * @return 滤波器系数[b0, b1, b2, a1, a2]
 */
static QVector<double> calcPeakingCoeffs(double freq, double gainDb, double q, double sampleRate)
{
    const double A = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * M_PI * freq / sampleRate;
    const double cosW0 = std::cos(w0);
    const double sinW0 = std::sin(w0);
    const double alpha = sinW0 / (2.0 * q);

    const double b0 = 1.0 + alpha * A;
    const double b1 = -2.0 * cosW0;
    const double b2 = 1.0 - alpha * A;
    const double a0 = 1.0 + alpha / A;
    const double a1 = -2.0 * cosW0;
    const double a2 = 1.0 - alpha / A;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

/**
 * @brief 处理音频采样数据
 *
 * 动态EQ处理流程:
 * 1. 计算当前帧的信号电平
 * 2. 根据电平计算动态增益(电平越高，增益越接近目标)
 * 3. 使用二阶峰值滤波器应用频段均衡
 *
 * @param samples 输入音频采样数据
 */
void DynamicEQ6::process(const QVector<double>& samples)
{
    if (samples.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const double sampleRate = 44100.0;

    // 计算RMS电平
    double rms = 0.0;
    for (int i = 0; i < N; ++i) {
        rms += samples[i] * samples[i];
    }
    rms = std::sqrt(rms / N);
    const double rmsDb = 20.0 * std::log10(qMax(rms, 1e-10));

    // 动态增益计算: 信号越强，增益越大(衰减模式)或越小(增强模式)
    const double threshold = -20.0; // dB
    double dynamicGain = 0.0;
    if (rmsDb > threshold) {
        dynamicGain = m_gain * (rmsDb - threshold) / 20.0;
    } else {
        dynamicGain = m_gain * 0.1; // 低电平最小增益
    }

    // 计算滤波器系数
    QVector<double> coeffs = calcPeakingCoeffs(m_freq, dynamicGain, m_q, sampleRate);

    // 应用IIR滤波器
    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
    for (int i = 0; i < N; ++i) {
        const double x0 = samples[i];
        const double y0 = coeffs[0] * x0 + coeffs[1] * x1 + coeffs[2] * x2
                          - coeffs[3] * y1 - coeffs[4] * y2;
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
    }

    // 更新统计信息
    m_stats.totalProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalProcessed / N);

    emit processingCompleted(N);
}

/**
 * @brief 重置所有统计信息
 */
void DynamicEQ6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
