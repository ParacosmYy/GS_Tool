#include "Phaser3.h"
#include <QElapsedTimer>
#include <cmath>

/**
 * @brief 构造函数，初始化Phaser效果器
 * @param parent 父QObject对象指针
 *
 * 默认4级全通滤波器，速率0.5Hz，产生经典相位效果。
 */
Phaser3::Phaser3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 处理音频缓冲区，应用Phaser效果
 *
 * Phaser通过级联全通滤波器产生相位偏移：
 * 1. LFO（低频振荡器）调制全通滤波器的截止频率
 * 2. 全通滤波器改变信号相位但保持幅度
 * 3. 原始信号与处理后的信号混合产生相位抵消（梳状陷波）
 * 4. 反馈回路增强效果
 *
 * 典型效果：旋转的"嗖嗖"声，常用于吉他和合成器。
 *
 * @param input 输入音频采样序列
 * @return 处理后的音频采样序列
 */
QVector<double> Phaser3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return input;

    QVector<double> output(n);

    /// 全通滤波器状态（每级两个延迟单元）
    const int numStages = m_stages;
    QVector<double> xState(numStages, 0.0);  ///< 输入延迟
    QVector<double> yState(numStages, 0.0);  ///< 输出延迟

    static double feedback = 0.0;
    const double sampleRate = 44100.0;
    static double phase = 0.0;

    for (int i = 0; i < n; ++i) {
        /// LFO调制：三角波产生中心频率偏移
        double lfoValue = 2.0 * std::abs(2.0 * (phase * m_rate - std::floor(phase * m_rate + 0.5))) - 1.0;
        double modFreq = 1000.0 + lfoValue * 800.0;  ///< 调制范围200~1800Hz

        /// 计算全通系数
        double tanVal = std::tan(M_PI * modFreq / sampleRate);
        double a1 = (tanVal - 1.0) / (tanVal + 1.0);

        /// 级联全通滤波处理
        double sample = input[i] + feedback * 0.7;

        for (int s = 0; s < numStages; ++s) {
            double newSample = a1 * sample + xState[s] - a1 * yState[s];
            xState[s] = sample;
            yState[s] = newSample;
            sample = newSample;
        }

        /// 更新反馈
        feedback = sample;

        /// 混合原始信号与处理后信号
        output[i] = input[i] * 0.5 + sample * 0.5;
        output[i] = qBound(-1.0, output[i], 1.0);

        /// 推进LFO相位
        phase += 1.0 / sampleRate;
    }

    /// 更新统计信息
    m_stats.totalSamplesProcessed += n;
    m_stats.totalStagesActive = numStages;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalSamplesProcessed / 256);

    emit effectApplied(n, numStages);
    return output;
}

/**
 * @brief 设置Phaser效果参数
 *
 * @param stages 全通滤波器级数(2~12)，级数越多效果越丰富
 * @param rateHz LFO调制速率(Hz)，典型0.1~2.0Hz
 * @param depth 调制深度(0.0~1.0)
 * @param feedback 反馈系数(0.0~0.95)
 */
void Phaser3::setParameters(int stages, double rateHz, double depth, double feedback)
{
    m_stages = qBound(2, stages, 12);
    m_rate = qBound(0.05, rateHz, 5.0);
    Q_UNUSED(depth)
    Q_UNUSED(feedback)
}

/**
 * @brief 获取当前统计数据
 * @return 包含采样处理数、激活级数和平均耗时的Stats结构
 */
Phaser3::Stats Phaser3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void Phaser3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
