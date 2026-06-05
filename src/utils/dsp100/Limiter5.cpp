#include "Limiter5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Limiter5.cpp
 * @brief 音频限幅器实现
 *
 * 将信号绝对峰值限制在指定上限以内，防止削波失真。
 * 当信号超过阈值时立即降低增益，释放时间内逐渐恢复。
 */

/**
 * @brief 构造函数，初始化默认限幅参数
 * @param parent 父QObject对象指针
 */
Limiter5::Limiter5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置限幅阈值
 * @param thresholdDb 限幅开始生效的电平(dB)
 */
void Limiter5::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置输出上限
 * @param ceilingDb 输出信号的最大允许电平(dB)
 */
void Limiter5::setCeiling(double ceilingDb)
{
    m_ceiling = ceilingDb;
}

/**
 * @brief 设置释放时间
 * @param releaseMs 增益恢复到正常值的时间(ms)
 */
void Limiter5::setRelease(double releaseMs)
{
    m_release = qMax(1.0, releaseMs);
}

/**
 * @brief 处理音频数据帧
 *
 * 限幅处理流程:
 * 1. 检测信号峰值电平
 * 2. 如果峰值超过阈值，立即降低增益
 * 3. 增益下限由ceiling参数确定
 * 4. 释放时间内增益逐渐恢复
 *
 * @param samples 输入音频采样数据
 * @return 限幅处理后的音频采样数据
 */
QVector<double> Limiter5::process(const QVector<double>& samples)
{
    if (samples.isEmpty()) return samples;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    QVector<double> output(N);

    // 线性域参数
    const double thresholdLin = std::pow(10.0, m_threshold / 20.0);
    const double ceilingLin = std::pow(10.0, m_ceiling / 20.0);

    // 释放时间系数
    const double sampleRate = 44100.0;
    const double releaseCoeff = std::exp(-1.0 / (m_release * 0.001 * sampleRate));

    double currentGain = 1.0;
    double gainSum = 0.0;

    for (int i = 0; i < N; ++i) {
        const double absSample = std::fabs(samples[i]);

        // 计算所需增益
        if (absSample * currentGain > ceilingLin) {
            // 超过上限，需要立即降低增益(lookahead-free)
            currentGain = ceilingLin / qMax(absSample, 1e-10);
        } else if (absSample > thresholdLin && currentGain >= 1.0) {
            // 超过阈值但增益未降低，计算目标增益
            const double targetGain = thresholdLin / qMax(absSample, 1e-10);
            currentGain = qMin(currentGain, targetGain);
        } else {
            // 释放阶段: 增益逐渐恢复
            currentGain = releaseCoeff * currentGain + (1.0 - releaseCoeff) * 1.0;
            currentGain = qMin(currentGain, 1.0);
        }

        // 确保增益不低于ceiling/threshold
        const double minGain = ceilingLin / qMax(thresholdLin, 1e-10);
        currentGain = qMax(currentGain, minGain);

        output[i] = samples[i] * currentGain;

        // 统计增益衰减
        const double reductionDb = -20.0 * std::log10(qMax(currentGain, 1e-10));
        gainSum += qMax(0.0, reductionDb);
    }

    // 更新统计信息
    m_stats.totalFrames++;
    m_stats.avgGainReduction = gainSum / N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit processingCompleted(N);
    return output;
}

/**
 * @brief 重置所有统计信息
 */
void Limiter5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
