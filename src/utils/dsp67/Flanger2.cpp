/**
 * @file Flanger2.cpp
 * @brief 镶边音效处理器实现（第2版）
 *
 * 使用LFO调制的延迟线实现镶边（Flanger）效果。
 * LFO以正弦/三角/锯齿波形式调制延迟时间，产生
 * 梳状滤波器的扫频效果。支持反馈、混合比和波形选择。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp67/Flanger2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化镶边处理器
 * @param parent 父QObject对象指针
 */
Flanger2::Flanger2(QObject* parent)
    : QObject(parent)
{
    /* 初始化延迟缓冲区 */
    int maxDelay = static_cast<int>(20.0 * 44100.0 / 1000.0);
    m_buffer.resize(qMax(maxDelay, 1), 0.0);
    m_bufPos = 0;
}

/**
 * @brief 设置LFO速率
 * @param hz LFO频率（Hz），典型范围0.05~5Hz
 */
void Flanger2::setRate(double hz)
{
    m_rate = qBound(0.01, hz, 20.0);
}

/**
 * @brief 设置调制深度
 * @param ms 最大延迟偏移量（毫秒），典型1~10ms
 */
void Flanger2::setDepth(double ms)
{
    m_depth = qBound(0.1, ms, 20.0);
}

/**
 * @brief 设置反馈系数
 * @param fb 反馈量（0.0~0.99）
 */
void Flanger2::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
}

/**
 * @brief 设置LFO波形类型
 * @param wave 波形类型："sine"正弦, "triangle"三角, "sawtooth"锯齿
 */
void Flanger2::setWaveform(const QString& wave)
{
    if (wave == "sine" || wave == "triangle" || wave == "sawtooth") {
        m_wave = wave;
    }
}

/**
 * @brief 设置干湿混合比
 * @param mix 湿信号比例（0.0~1.0）
 */
void Flanger2::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

/**
 * @brief 计算LFO输出值
 * @param phase 当前相位（0~1）
 * @return LFO输出值（-1~1）
 */
double Flanger2::lfo(double phase) const
{
    if (m_wave == "sine") {
        return qSin(2.0 * M_PI * phase);
    } else if (m_wave == "triangle") {
        return 4.0 * qAbs(phase - qFloor(phase + 0.5)) - 1.0;
    } else {
        /* 锯齿波 */
        return 2.0 * (phase - qFloor(phase + 0.5));
    }
}

/**
 * @brief 处理输入音频信号，添加镶边效果
 *
 * LFO调制延迟时间，延迟信号与原始信号混合
 * 产生梳状滤波器效应。反馈回路增强效果的金属感。
 *
 * @param input 输入音频采样序列
 * @return 添加镶边效果后的音频序列
 */
QVector<double> Flanger2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) {
        emit processingCompleted(0);
        return output;
    }

    int n = input.size();
    output.resize(n);

    double phase = 0.0;
    double sampleRate = 44100.0;
    double phaseInc = m_rate / sampleRate;
    int maxDelaySamples = static_cast<int>(m_depth * sampleRate / 1000.0);
    maxDelaySamples = qBound(1, maxDelaySamples, m_buffer.size() - 1);

    for (int s = 0; s < n; ++s) {
        /* LFO调制延迟时间 */
        double lfoVal = lfo(phase);
        m_lfoVal = lfoVal;
        int delaySamples = static_cast<int>((1.0 + lfoVal) * 0.5 * maxDelaySamples);
        delaySamples = qBound(0, delaySamples, m_buffer.size() - 1);

        /* 从延迟缓冲区读取 */
        int readPos = (m_bufPos - delaySamples + m_buffer.size()) % m_buffer.size();
        double delayed = m_buffer[readPos];

        /* 写入输入+反馈到延迟缓冲区 */
        double writeVal = input[s] + delayed * m_feedback;
        m_buffer[m_bufPos] = writeVal;
        m_bufPos = (m_bufPos + 1) % m_buffer.size();

        /* 干湿混合 */
        output[s] = input[s] * (1.0 - m_mix) + delayed * m_mix;

        /* 更新相位 */
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n);
    return output;
}

/**
 * @brief 获取当前统计信息
 * @return 处理统计结构
 */
Flanger2::Stats Flanger2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void Flanger2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
