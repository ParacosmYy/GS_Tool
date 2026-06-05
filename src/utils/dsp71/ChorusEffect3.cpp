/**
 * @file ChorusEffect3.cpp
 * @brief 合唱效果器实现
 *
 * 实现多声部合唱效果，通过LFO调制的延迟线产生
 * 丰富的合唱/颤音效果。支持多声部、反馈和干湿比控制。
 */

#include "utils/dsp71/ChorusEffect3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
ChorusEffect3::ChorusEffect3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置LFO速率
 * @param hz LFO频率(Hz)
 */
void ChorusEffect3::setRate(double hz)
{
    m_rate = qBound(0.01, hz, 20.0);
}

/**
 * @brief 设置调制深度
 * @param ms 深度(ms)，延迟线的最大调制偏移
 */
void ChorusEffect3::setDepth(double ms)
{
    m_depth = qBound(0.1, ms, 50.0);
}

/**
 * @brief 设置声部数量
 * @param v 声部数
 */
void ChorusEffect3::setVoices(int v)
{
    m_voices = qBound(1, v, 8);
    m_buffers.resize(m_voices);
    m_bufPos.resize(m_voices, 0);
}

/**
 * @brief 设置反馈量
 * @param fb 反馈增益，范围[0, 0.95]
 */
void ChorusEffect3::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.95);
}

/**
 * @brief 设置干湿混合比
 * @param mix 混合比，0.0=全干，1.0=全湿
 */
void ChorusEffect3::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

/**
 * @brief 处理音频信号
 * @param input 输入音频采样
 * @return 处理后的音频采样
 *
 * 每个声部使用独立的延迟线，延迟时间由LFO调制。
 * 声部间通过相位偏移产生去相关效果，反馈增加丰富度。
 * 使用线性插值读取延迟线避免量化噪声。
 */
QVector<double> ChorusEffect3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return QVector<double>();

    const int N = input.size();
    const double sr = 44100.0;
    const int maxDelay = qRound(sr * (m_depth + 5.0) / 1000.0) + 2;

    /* 初始化延迟线缓冲区 */
    if (m_buffers.size() != m_voices) {
        m_buffers.resize(m_voices);
        m_bufPos.resize(m_voices, 0);
    }
    for (int v = 0; v < m_voices; ++v) {
        if (m_buffers[v].size() < maxDelay) {
            m_buffers[v].resize(maxDelay, 0.0);
        }
        if (m_bufPos[v] >= maxDelay) m_bufPos[v] = 0;
    }

    QVector<double> output(N, 0.0);
    double phase = 0.0;
    double phaseInc = 2.0 * M_PI * m_rate / sr;

    for (int i = 0; i < N; ++i) {
        double wet = 0.0;

        for (int v = 0; v < m_voices; ++v) {
            /* 步骤1: 计算当前延迟(LFO调制) */
            double lfoVal = lfo(phase + v * 2.0 * M_PI / m_voices, v);
            double delaySamples = m_depth * sr / 1000.0 * (1.0 + lfoVal) * 0.5;
            delaySamples = qBound(1.0, delaySamples + 1.0, maxDelay - 2.0);

            /* 步骤2: 写入延迟线(含反馈) */
            int prevPos = (m_bufPos[v] + maxDelay - 1) % maxDelay;
            m_buffers[v][m_bufPos[v]] = input[i] + m_feedback * m_buffers[v][prevPos];

            /* 步骤3: 线性插值读取延迟线 */
            int intDelay = static_cast<int>(delaySamples);
            double frac = delaySamples - intDelay;
            int readPos1 = (m_bufPos[v] + maxDelay - intDelay) % maxDelay;
            int readPos2 = (readPos1 + maxDelay - 1) % maxDelay;
            double sample = (1.0 - frac) * m_buffers[v][readPos1] + frac * m_buffers[v][readPos2];

            wet += sample;
            m_bufPos[v] = (m_bufPos[v] + 1) % maxDelay;
        }

        /* 步骤4: 平均多声部并混合 */
        wet /= m_voices;
        output[i] = input[i] * (1.0 - m_mix) + wet * m_mix;

        /* 步骤5: 更新LFO相位 */
        phase += phaseInc;
        if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N);
    return output;
}

/**
 * @brief 重置统计信息
 */
void ChorusEffect3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief LFO波形函数
 * @param phase 当前相位
 * @param voice 声部编号
 * @return LFO输出值[-1, 1]
 *
 * 使用正弦波作为基本LFO波形，各声部通过微小偏移
 * 产生去相关效果，增加合唱的丰富度。
 */
double ChorusEffect3::lfo(double phase, int voice) const
{
    /* 正弦波LFO + 声部间微调 */
    double p = phase + voice * 0.1;
    return qSin(p);
}
