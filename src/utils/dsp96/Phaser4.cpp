#include "Phaser4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化移相效果器
 * @param parent 父对象指针
 */
Phaser4::Phaser4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置LFO调制速率(Hz)
 * @param rate LFO频率，控制相位扫描速度
 */
void Phaser4::setRate(double rate)
{
    m_rate = qBound(0.01, rate, 20.0);
}

/**
 * @brief 设置调制深度
 * @param depth 调制深度(0~1)
 */
void Phaser4::setDepth(double depth)
{
    m_depth = qBound(0.0, depth, 1.0);
}

/**
 * @brief 设置反馈量
 * @param feedback 反馈增益(0~1)
 */
void Phaser4::setFeedback(double feedback)
{
    m_feedback = qBound(0.0, feedback, 0.99);
}

/**
 * @brief 一阶全通滤波器
 * @param input 输入采样
 * @param coeff 全通系数
 * @param x1 前一个输入(引用更新)
 * @param y1 前一个输出(引用更新)
 * @return 滤波后输出
 */
static double allpass(double input, double coeff, double& x1, double& y1)
{
    double output = -coeff * input + x1 + coeff * y1;
    x1 = input;
    y1 = output;
    return output;
}

/**
 * @brief 处理音频采样数据
 *
 * Phaser效果通过级联全通滤波器实现：
 * 1. LFO生成调制信号(正弦波)
 * 2. LFO调制全通滤波器的截止频率
 * 3. 级联4级全通滤波器产生梳状滤波效应
 * 4. 混合干/湿信号并应用反馈
 *
 * @param samples 输入音频采样
 */
void Phaser4::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
        emit processingCompleted(0);
        return;
    }

    const int N = samples.size();
    double sampleRate = 44100.0;
    double phase = 0.0;
    double phaseInc = 2.0 * M_PI * m_rate / sampleRate;

    /* 全通滤波器状态(4级) */
    double x1[4] = {0}, y1[4] = {0};
    double feedbackSample = 0.0;

    for (int i = 0; i < N; ++i) {
        /* LFO调制 */
        double lfoValue = std::sin(phase);
        phase += phaseInc;
        if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;

        /* 计算全通系数 */
        double modDepth = m_depth * 0.5;
        double minFreq = 300.0;
        double maxFreq = 3000.0;
        double freq = minFreq + (maxFreq - minFreq) * (0.5 + modDepth * lfoValue);
        double coeff = (std::tan(M_PI * freq / sampleRate) - 1.0) /
                       (std::tan(M_PI * freq / sampleRate) + 1.0);

        /* 输入 = 原始信号 + 反馈 */
        double input = samples[i] + feedbackSample * m_feedback;

        /* 通过4级全通滤波器 */
        double output = input;
        for (int stage = 0; stage < 4; ++stage) {
            output = allpass(output, coeff, x1[stage], y1[stage]);
        }

        /* 反馈 */
        feedbackSample = output;

        /* 干/湿混合(50%) */
        double mixed = samples[i] * 0.5 + output * 0.5;
        Q_UNUSED(mixed)
    }

    m_timeSum += timer.elapsed();
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
    emit processingCompleted(N);
}

/**
 * @brief 重置统计数据
 */
void Phaser4::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
