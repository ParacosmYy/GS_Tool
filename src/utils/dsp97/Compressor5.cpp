#include "Compressor5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Compressor5.cpp
 * @brief 动态范围压缩器实现
 *
 * 通过增益控制减少信号的动态范围，当信号超过阈值时
 * 按设定的压缩比降低增益，支持软/硬拐点平滑过渡。
 */

/**
 * @brief 构造函数，初始化默认压缩参数
 * @param parent 父QObject对象指针
 */
Compressor5::Compressor5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置压缩阈值
 * @param thresholdDb 压缩开始生效的信号电平(dB)
 */
void Compressor5::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置压缩比
 * @param ratio 压缩比(如4.0表示4:1压缩)
 */
void Compressor5::setRatio(double ratio)
{
    m_ratio = qMax(1.0, ratio);
}

/**
 * @brief 设置软拐点宽度
 * @param kneeDb 拐点过渡区域宽度(dB)，0为硬拐点
 */
void Compressor5::setKnee(double kneeDb)
{
    m_knee = qMax(0.0, kneeDb);
}

/**
 * @brief 处理音频数据帧
 *
 * 对输入采样逐帧计算增益衰减量，应用压缩特性曲线：
 * - 低于阈值的信号不压缩
 * - 软拐点模式下在阈值附近平滑过渡
 * - 超过阈值的信号按压缩比衰减
 *
 * @param samples 输入音频采样数据
 * @return 压缩处理后的音频采样数据
 */
QVector<double> Compressor5::process(const QVector<double>& samples)
{
    if (samples.isEmpty()) return samples;

    QElapsedTimer timer;
    timer.start();

    QVector<double> output(samples.size());
    double gainSum = 0.0;

    for (int i = 0; i < samples.size(); ++i) {
        // 计算输入电平(dB)
        const double absSample = std::fabs(samples[i]);
        const double inputDb = 20.0 * std::log10(qMax(absSample, 1e-10));

        double gainReductionDb = 0.0;

        if (m_knee > 0.0) {
            // 软拐点压缩: 平滑过渡区域
            const double halfKnee = m_knee / 2.0;
            if (inputDb < m_threshold - halfKnee) {
                // 低于拐点区域，不压缩
                gainReductionDb = 0.0;
            } else if (inputDb > m_threshold + halfKnee) {
                // 高于拐点区域，完全压缩
                gainReductionDb = (inputDb - m_threshold) * (1.0 - 1.0 / m_ratio);
            } else {
                // 拐点过渡区域内，二次插值
                const double x = inputDb - m_threshold + halfKnee;
                gainReductionDb = (x * x) / (2.0 * m_knee) * (1.0 - 1.0 / m_ratio);
            }
        } else {
            // 硬拐点压缩
            if (inputDb > m_threshold) {
                gainReductionDb = (inputDb - m_threshold) * (1.0 - 1.0 / m_ratio);
            }
        }

        // 应用增益衰减
        const double gain = std::pow(10.0, -gainReductionDb / 20.0);
        output[i] = samples[i] * gain;
        gainSum += gainReductionDb;
    }

    // 更新统计信息
    m_stats.totalFrames++;
    m_stats.avgGainReduction = gainSum / samples.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit processingCompleted(output.size());
    return output;
}

/**
 * @brief 重置所有统计信息
 */
void Compressor5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
