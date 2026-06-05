#include "Expander4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Expander4.cpp
 * @brief 动态范围扩展器实现
 *
 * 当信号低于阈值时进行额外衰减，增加信号的动态范围。
 * 支持起音时间控制，实现平滑的增益过渡。
 */

/**
 * @brief 构造函数，初始化默认扩展参数
 * @param parent 父QObject对象指针
 */
Expander4::Expander4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置扩展阈值
 * @param thresholdDb 扩展开始生效的信号电平(dB)
 */
void Expander4::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置扩展范围
 * @param rangeDb 最大增益衰减量(dB)
 */
void Expander4::setRange(double rangeDb)
{
    m_range = rangeDb;
}

/**
 * @brief 设置起音时间
 * @param attackMs 增益变化到目标值63%所需时间(ms)
 */
void Expander4::setAttack(double attackMs)
{
    m_attack = qMax(0.1, attackMs);
}

/**
 * @brief 处理音频数据帧
 *
 * 对低于阈值的信号按扩展比进行衰减:
 * - 高于阈值的信号不受影响
 * - 低于阈值的信号按比例衰减
 * - 起音时间控制增益变化的平滑度
 *
 * @param samples 输入音频采样数据
 * @return 扩展处理后的音频采样数据
 */
QVector<double> Expander4::process(const QVector<double>& samples)
{
    if (samples.isEmpty()) return samples;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    QVector<double> output(N);

    // 起音时间系数
    const double attackCoeff = std::exp(-1.0 / (m_attack * 0.001 * 44100.0));
    double currentGain = 1.0;
    double gainSum = 0.0;

    for (int i = 0; i < N; ++i) {
        // 计算输入电平(dB)
        const double absSample = std::fabs(samples[i]);
        const double inputDb = 20.0 * std::log10(qMax(absSample, 1e-10));

        // 计算目标增益
        double targetGain = 1.0;
        if (inputDb < m_threshold) {
            // 低于阈值，计算扩展增益
            const double rangeLin = std::pow(10.0, m_range / 20.0);
            const double thresholdLin = std::pow(10.0, m_threshold / 20.0);
            if (absSample < thresholdLin) {
                // 线性插值计算增益
                const double ratio = absSample / thresholdLin;
                targetGain = 1.0 - (1.0 - ratio) * (1.0 - 1.0 / rangeLin);
            }
        }

        // 平滑增益变化(起音时间)
        currentGain = attackCoeff * currentGain + (1.0 - attackCoeff) * targetGain;

        // 应用增益
        output[i] = samples[i] * currentGain;

        // 统计增益衰减量
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
void Expander4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
