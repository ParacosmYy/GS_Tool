#include "ChorusEffect4.h"
#include <QElapsedTimer>
#include <cmath>

/**
 * @brief 构造函数，初始化合唱效果器
 * @param parent 父QObject对象指针
 *
 * 默认调制深度0.5，速率1.5Hz，产生适中的合唱空间感。
 */
ChorusEffect4::ChorusEffect4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 处理音频缓冲区，应用合唱效果
 *
 * 使用正弦调制延迟线模拟多个声部叠加的效果。
 * 延迟时间在[baseDelay - depth, baseDelay + depth]范围内周期性变化，
 * 产生丰富的合唱/空间感。延迟线以环形缓冲区方式实现。
 *
 * @param input 输入音频采样序列
 * @return 处理后的音频采样序列
 */
QVector<double> ChorusEffect4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    QVector<double> output(n);

    /// 延迟线参数
    const int maxDelaySamples = 512;  ///< 最大延迟采样数
    const double baseDelayMs = 25.0;  ///< 基准延迟25ms
    const double sampleRate = 44100.0;
    const int baseDelaySamples = static_cast<int>(baseDelayMs * sampleRate / 1000.0);
    const double modDepthSamples = m_depth * maxDelaySamples;

    /// 环形延迟缓冲区
    static QVector<double> delayBuffer(maxDelaySamples * 2, 0.0);
    static int writePos = 0;

    /// 处理每个采样点
    for (int i = 0; i < n; ++i) {
        /// 计算当前调制的延迟量
        double phase = 2.0 * M_PI * m_rate * i / sampleRate;
        double modDelay = baseDelaySamples + modDepthSamples * std::sin(phase);

        /// 线性插值读取延迟线
        int intDelay = static_cast<int>(modDelay);
        double frac = modDelay - intDelay;
        int readPos1 = (writePos - intDelay + delayBuffer.size()) % delayBuffer.size();
        int readPos2 = (readPos1 - 1 + delayBuffer.size()) % delayBuffer.size();
        double delayed = delayBuffer[readPos1] * (1.0 - frac) + delayBuffer[readPos2] * frac;

        /// 混合原始信号与延迟信号
        output[i] = input[i] * 0.7 + delayed * 0.5;

        /// 更新延迟线
        delayBuffer[writePos] = input[i];
        writePos = (writePos + 1) % delayBuffer.size();
    }

    /// 更新统计信息
    m_stats.totalSamplesProcessed += n;
    m_stats.totalBuffersApplied++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuffersApplied;

    emit effectApplied(n, m_depth);
    return output;
}

/**
 * @brief 设置合唱效果的调制参数
 *
 * @param depth 调制深度(0.0~1.0)，控制延迟变化幅度
 * @param rateHz 调制速率(Hz)，控制延迟变化频率，典型值0.5~3.0Hz
 */
void ChorusEffect4::setModulation(double depth, double rateHz)
{
    m_depth = qBound(0.0, depth, 1.0);
    m_rate = qBound(0.1, rateHz, 10.0);
}

/**
 * @brief 获取当前统计数据
 * @return 包含采样处理数、缓冲区数和平均耗时的Stats结构
 */
ChorusEffect4::Stats ChorusEffect4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void ChorusEffect4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
