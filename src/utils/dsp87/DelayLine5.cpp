#include "DelayLine5.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @class DelayLine5
 * @brief 延迟线处理器实现
 *
 * 数字延迟线是音频效果处理的基础模块。
 * 信号延迟指定时间后与原信号混合，配合反馈回路
 * 可产生回声、合唱、镶边等效果。
 *
 * 延迟输出: y[n] = x[n] * dry + x[n-D] * wet
 * 反馈回路: buffer[writePos] = x[n] + buffer[readPos] * feedback
 * 其中D为延迟采样数，dry/wet为干湿比。
 */

/**
 * @brief 构造函数，初始化默认延迟参数
 * @param parent 父QObject
 */
DelayLine5::DelayLine5(QObject* parent)
    : QObject(parent)
    , m_delayMs(250.0)
    , m_feedback(0.3)
    , m_mix(0.5)
{
}

/**
 * @brief 处理音频缓冲区
 *
 * 使用环形缓冲区实现延迟效果:
 * 1. 将输入采样写入延迟缓冲区的当前位置
 * 2. 从延迟位置读取历史采样
 * 3. 将延迟采样乘以反馈系数后写回缓冲区(产生重复回声)
 * 4. 混合干信号和湿信号输出
 *
 * @param input 输入音频采样缓冲区
 * @return 添加延迟效果后的音频缓冲区
 */
QVector<double> DelayLine5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    QVector<double> output(N, 0.0);

    if (N == 0) {
        m_timeSum += timer.elapsed();
        return output;
    }

    /* 延迟采样数(假设44100Hz采样率) */
    const int sampleRate = 44100;
    int delaySamples = qMax(1, static_cast<int>(m_delayMs * sampleRate / 1000.0));

    /* 初始化/扩展延迟缓冲区 */
    if (m_delayBuffer.size() != delaySamples) {
        m_delayBuffer.resize(delaySamples, 0.0);
        m_writePos = 0;
    }

    for (int n = 0; n < N; ++n) {
        /* 读取延迟输出 */
        int readPos = (m_writePos - delaySamples + m_delayBuffer.size()) % m_delayBuffer.size();
        double delayed = m_delayBuffer[readPos];

        /* 写入当前输入 + 反馈 */
        m_delayBuffer[m_writePos] = input[n] + delayed * m_feedback;

        /* 推进写入位置 */
        m_writePos = (m_writePos + 1) % m_delayBuffer.size();

        /* 干湿混合 */
        double dry = 1.0 - m_mix;
        output[n] = input[n] * dry + delayed * m_mix;
    }

    m_stats.totalSamplesProcessed += N;
    m_stats.totalBuffersApplied++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBuffersApplied);

    emit effectApplied(N, m_delayMs);

    return output;
}

/**
 * @brief 设置延迟参数
 *
 * @param delayMs 延迟时间(毫秒)，1~5000ms
 * @param feedback 反馈系数(0~0.95)，越大回声次数越多
 * @param mix 干湿混合比(0~1)，0为全干信号，1为全湿信号
 */
void DelayLine5::setParameters(double delayMs, double feedback, double mix)
{
    QElapsedTimer timer;
    timer.start();

    m_delayMs = qBound(1.0, delayMs, 5000.0);
    m_feedback = qBound(0.0, feedback, 0.95);
    m_mix = qBound(0.0, mix, 1.0);

    m_timeSum += timer.elapsed();
}

/**
 * @brief 重置所有统计数据
 *
 * 将采样计数、缓冲区计数和计时归零。
 */
void DelayLine5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_delayBuffer.clear();
}
