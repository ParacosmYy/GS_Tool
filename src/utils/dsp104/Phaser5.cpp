#include "Phaser5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Phaser5.cpp
 * @brief 移相效果器(Phaser)实现
 *
 * 通过级联全通滤波器产生频率响应的梳状陷波，
 * 配合LFO(低频振荡器)调制陷波频率实现经典移相音效。
 * 全通滤波器保持幅度响应不变但改变相位，
 * 与干信号混合后产生相位抵消(陷波)。
 */

/**
 * @brief 构造函数，初始化默认效果参数
 * @param parent 父QObject对象指针
 */
Phaser5::Phaser5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置LFO调制速率
 * @param rate LFO频率(Hz)，典型值0.1~5Hz
 */
void Phaser5::setRate(double rate)
{
    m_rate = qMax(0.01, rate);
}

/**
 * @brief 设置调制深度
 * @param depth 调制深度(0~1)，控制陷波频率偏移范围
 */
void Phaser5::setDepth(double depth)
{
    m_depth = qBound(0.0, depth, 1.0);
}

/**
 * @brief 设置反馈增益
 * @param feedback 反馈增益(0~0.99)，增强效果强度
 */
void Phaser5::setFeedback(double feedback)
{
    m_feedback = qBound(0.0, feedback, 0.99);
}

/**
 * @brief 处理音频采样数据
 *
 * 移相处理流程:
 * 1. LFO生成周期性调制信号(正弦波)
 * 2. 调制信号控制级联全通滤波器的中心频率
 * 3. 全通滤波器输出与干信号混合
 * 4. 反馈回路增强效果
 *
 * 一阶全通滤波器传递函数:
 * H(z) = (a + z^{-1}) / (1 + a*z^{-1})
 * a = (tan(pi*fc/fs) - 1) / (tan(pi*fc/fs) + 1)
 *
 * @param samples 输入音频采样数据
 */
void Phaser5::process(const QVector<double>& samples)
{
    if (samples.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const double sampleRate = 44100.0;

    // 级联全通滤波器状态(4级)
    const int stages = 4;
    QVector<double> ap_x1(stages, 0.0);
    QVector<double> ap_y1(stages, 0.0);

    double feedbackSample = 0.0;
    double lfoPhase = 0.0;
    const double lfoStep = m_rate / sampleRate;

    for (int i = 0; i < N; ++i) {
        // 步骤1: 计算LFO值(正弦波)
        const double lfoValue = 0.5 * (1.0 + std::sin(2.0 * M_PI * lfoPhase));
        lfoPhase += lfoStep;
        if (lfoPhase >= 1.0) lfoPhase -= 1.0;

        // 步骤2: 计算全通系数
        const double minFreq = 300.0;
        const double maxFreq = 3000.0;
        const double modFreq = minFreq + (maxFreq - minFreq) * (lfoValue * m_depth);

        const double tanVal = std::tan(M_PI * modFreq / sampleRate);
        const double a = (tanVal - 1.0) / (tanVal + 1.0);

        // 步骤3: 级联全通滤波
        double sample = samples[i] + feedbackSample * m_feedback;

        for (int s = 0; s < stages; ++s) {
            const double y = a * sample + ap_x1[s] - a * ap_y1[s];
            ap_x1[s] = sample;
            ap_y1[s] = y;
            sample = y;
        }

        feedbackSample = sample;

        // 步骤4: 与干信号混合(50/50)
        const double output = 0.5 * samples[i] + 0.5 * sample;
        Q_UNUSED(output)
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
void Phaser5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
