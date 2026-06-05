#include "Gate4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Gate4.cpp
 * @brief 噪声门处理器实现
 *
 * 当信号电平低于阈值时将信号衰减至静音，高于阈值时信号正常通过。
 * 通过起音和释放时间参数控制增益变化的平滑度，避免咔嗒噪声。
 */

/**
 * @brief 构造函数，初始化默认噪声门参数
 * @param parent 父QObject对象指针
 */
Gate4::Gate4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置门限阈值
 * @param thresholdDb 噪声门开启/关闭的信号电平阈值(dB)
 */
void Gate4::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置起音时间
 * @param attackMs 门从关闭到打开的过渡时间(ms)
 */
void Gate4::setAttack(double attackMs)
{
    m_attack = qMax(0.01, attackMs);
}

/**
 * @brief 设置释放时间
 * @param releaseMs 门从打开到关闭的过渡时间(ms)
 */
void Gate4::setRelease(double releaseMs)
{
    m_release = qMax(0.01, releaseMs);
}

/**
 * @brief 处理音频数据帧
 *
 * 噪声门处理流程:
 * 1. 计算每帧信号的RMS电平(dB)
 * 2. 与阈值比较确定门状态(开/关)
 * 3. 根据起音/释放时间平滑增益变化
 * 4. 应用增益到输出信号
 *
 * @param samples 输入音频采样数据
 * @return 门控处理后的音频采样数据
 */
QVector<double> Gate4::process(const QVector<double>& samples)
{
    if (samples.isEmpty()) return samples;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();

    // 起音/释放时间系数
    const double sampleRate = 44100.0;
    const double attackCoeff = std::exp(-1.0 / (m_attack * 0.001 * sampleRate));
    const double releaseCoeff = std::exp(-1.0 / (m_release * 0.001 * sampleRate));

    QVector<double> output(N);
    double currentGain = 0.0; // 初始状态门关闭
    double gainSum = 0.0;

    const double thresholdLin = std::pow(10.0, m_threshold / 20.0);

    for (int i = 0; i < N; ++i) {
        // 计算瞬时电平
        const double absSample = std::fabs(samples[i]);

        // 确定目标增益
        double targetGain = 0.0;
        if (absSample >= thresholdLin) {
            targetGain = 1.0; // 信号超过阈值，门打开
        }

        // 平滑增益变化
        if (targetGain > currentGain) {
            // 起音阶段: 门正在打开
            currentGain = attackCoeff * currentGain + (1.0 - attackCoeff) * targetGain;
        } else {
            // 释放阶段: 门正在关闭
            currentGain = releaseCoeff * currentGain + (1.0 - releaseCoeff) * targetGain;
        }

        // 应用增益
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
void Gate4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
