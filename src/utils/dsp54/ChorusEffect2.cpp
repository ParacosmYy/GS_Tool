/**
 * @file ChorusEffect2.cpp
 * @brief 合唱效果器实现，多声部LFO调制延迟线
 *
 * 合唱效果通过多个延迟声部的叠加实现空间感和丰满度。
 * 每个声部使用独立的正弦LFO调制延迟时间，产生微妙的
 * 音高和时间变化，模拟多个乐器同时演奏的效果。
 *
 * 参数说明：
 * - voices: 声部数量，每个声部有独立的LFO相位
 * - rate: LFO频率(Hz)，控制调制速度
 * - depth: 延迟调制深度(ms)，控制音高变化幅度
 * - mix: 干湿比(0.0~1.0)
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp54/ChorusEffect2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化延迟线和LFO
 * @param voices 声部数量，默认3
 * @param parent 父QObject对象指针
 */
ChorusEffect2::ChorusEffect2(int voices, QObject* parent)
    : QObject(parent), m_voices(qMax(1, voices))
{
    /* 初始化每个声部的延迟缓冲区 */
    int maxDelaySamples = static_cast<int>(m_sampleRate * 0.05); /* 最大50ms延迟 */
    m_buffers.resize(m_voices);
    m_writeIdx.resize(m_voices, 0);
    m_phase.resize(m_voices, 0.0);

    for (int v = 0; v < m_voices; ++v) {
        m_buffers[v].resize(maxDelaySamples, 0.0);
        /* 每个声部使用不同的初始相位 */
        m_phase[v] = (2.0 * M_PI * v) / m_voices;
    }
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)
 */
void ChorusEffect2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);

    /* 重新初始化延迟缓冲区 */
    int maxDelaySamples = static_cast<int>(m_sampleRate * 0.05);
    for (int v = 0; v < m_voices; ++v) {
        m_buffers[v].resize(maxDelaySamples, 0.0);
        m_writeIdx[v] = 0;
    }
}

/**
 * @brief 设置LFO调制速率
 * @param hz LFO频率(Hz)，典型值0.5~3.0
 */
void ChorusEffect2::setRate(double hz)
{
    m_rate = qBound(0.01, hz, 20.0);
}

/**
 * @brief 设置延迟调制深度
 * @param ms 调制深度(ms)，典型值1.0~10.0
 */
void ChorusEffect2::setDepth(double ms)
{
    m_depth = qBound(0.1, ms, 30.0);
}

/**
 * @brief 设置干湿混合比
 * @param wet 湿信号比例(0.0=全干, 1.0=全湿)
 */
void ChorusEffect2::setMix(double wet)
{
    m_mix = qBound(0.0, wet, 1.0);
}

/**
 * @brief 处理输入音频信号，添加合唱效果
 *
 * 处理流程（对每个采样点）：
 * 1. 将输入写入各延迟线
 * 2. 根据LFO计算各声部的当前延迟时间
 * 3. 使用线性插值从延迟线中读取采样
 * 4. 混合所有声部的输出并与干信号叠加
 *
 * @param input 输入音频采样序列
 * @return 添加合唱效果后的音频序列
 */
QVector<double> ChorusEffect2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int len = input.size();
    QVector<double> output(len, 0.0);
    int maxBufSize = m_buffers[0].size();

    for (int i = 0; i < len; ++i) {
        double wet = 0.0;

        for (int v = 0; v < m_voices; ++v) {
            /* 将输入写入延迟缓冲区 */
            m_buffers[v][m_writeIdx[v]] = input[i];

            /* 计算当前延迟时间（基础延迟 + LFO调制） */
            double baseDelay = 5.0; /* 基础延迟5ms */
            double mod = m_depth * qSin(m_phase[v]);
            double delayMs = baseDelay + mod;
            double delaySamples = delayMs * m_sampleRate / 1000.0;

            /* 确保延迟在有效范围内 */
            delaySamples = qBound(1.0, delaySamples, maxBufSize - 1.0);

            /* 计算读取位置（线性插值） */
            double readPos = m_writeIdx[v] - delaySamples;
            while (readPos < 0) readPos += maxBufSize;
            while (readPos >= maxBufSize) readPos -= maxBufSize;

            int idx0 = static_cast<int>(readPos);
            int idx1 = (idx0 + 1) % maxBufSize;
            double frac = readPos - idx0;

            /* 线性插值读取 */
            double sample = m_buffers[v][idx0] * (1.0 - frac) + m_buffers[v][idx1] * frac;
            wet += sample;

            /* 更新LFO相位 */
            m_phase[v] += 2.0 * M_PI * m_rate / m_sampleRate;
            if (m_phase[v] >= 2.0 * M_PI)
                m_phase[v] -= 2.0 * M_PI;

            /* 更新写入指针 */
            m_writeIdx[v] = (m_writeIdx[v] + 1) % maxBufSize;
        }

        /* 归一化湿信号并混合 */
        wet /= m_voices;
        output[i] = input[i] * (1.0 - m_mix) + wet * m_mix;
    }

    /* 更新统计 */
    m_stats.totalProcessCalls++;
    m_stats.totalSamples += len;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(len);
    return output;
}

/**
 * @brief 重置所有统计数据和延迟缓冲区
 */
void ChorusEffect2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;

    /* 清空延迟缓冲区 */
    for (int v = 0; v < m_voices; ++v) {
        m_buffers[v].fill(0.0);
        m_writeIdx[v] = 0;
        m_phase[v] = (2.0 * M_PI * v) / m_voices;
    }
}

/**
 * @brief 获取当前所有声部的LFO相位值
 * @return 各声部的相位值向量(弧度)
 *
 * 用于可视化LFO调制状态，帮助调试和参数调优。
 */
QVector<double> ChorusEffect2::phases() const
{
    return m_phase;
}

/**
 * @brief 获取指定声部的延迟缓冲区当前状态快照
 * @param voice 声部索引
 * @return 延迟缓冲区内容的副本
 *
 * 用于分析各声部的延迟线状态，帮助理解和调试效果器行为。
 * 返回的缓冲区以写入指针之后的数据开头（即最早进入的样本）。
 */
QVector<double> ChorusEffect2::delayBuffer(int voice) const
{
    if (voice < 0 || voice >= m_voices) return {};
    return m_buffers[voice];
}

/**
 * @brief 设置声部的LFO相位偏移
 * @param voice 声部索引
 * @param phase 相位偏移值(弧度)
 *
 * 允许精细控制各声部的LFO相位关系。
 * 不同相位偏移产生不同的空间效果：
 * - 均匀分布：经典合唱效果
 * - 随机分布：更自然的空间感
 * - 同相：类似Flanger效果
 */
void ChorusEffect2::setVoicePhase(int voice, double phase)
{
    if (voice >= 0 && voice < m_voices) {
        m_phase[voice] = phase;
    }
}

/**
 * @brief 获取当前效果器的参数配置摘要
 * @return 参数描述字符串
 *
 * 返回当前所有参数的格式化字符串，便于日志记录和调试。
 * 包含：声部数、采样率、速率、深度、混合比。
 */
QString ChorusEffect2::parameterSummary() const
{
    return QString("Chorus: voices=%1, sr=%2Hz, rate=%3Hz, depth=%4ms, mix=%5")
        .arg(m_voices)
        .arg(m_sampleRate, 0, 'f', 0)
        .arg(m_rate, 0, 'f', 2)
        .arg(m_depth, 0, 'f', 1)
        .arg(m_mix, 0, 'f', 2);
}
