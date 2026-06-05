/**
 * @file Flanger.cpp
 * @brief 镶边器实现 — 延迟调制/反馈/LFO/延迟线插值
 */

#include "utils/dsp31/Flanger.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Flanger::Flanger(QObject* parent)
    : QObject(parent)
{
    /* 预分配延迟线缓冲: 最大延迟对应的采样数 */
    int maxSamples = static_cast<int>(m_maxDelay * m_sampleRate / 1000.0) + 2;
    m_delayBuf.resize(maxSamples, 0.0);
}

/** @brief 设置LFO速率 @param hz 速率(Hz) */
void Flanger::setRate(double hz)
{
    m_rate = qBound(0.01, hz, 20.0);
}

/** @brief 设置调制深度 @param depth 深度[0,1] */
void Flanger::setDepth(double depth)
{
    m_depth = qBound(0.0, depth, 1.0);
}

/** @brief 设置反馈量 @param fb 反馈[0,0.99] */
void Flanger::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
}

/** @brief 设置延迟范围 @param minMs 最小延迟(ms) @param maxMs 最大延迟(ms) */
void Flanger::setDelayRange(double minMs, double maxMs)
{
    m_minDelay = qBound(0.1, minMs, 20.0);
    m_maxDelay = qBound(m_minDelay + 0.1, maxMs, 50.0);

    /* 重新分配延迟线 */
    int maxSamples = static_cast<int>(m_maxDelay * m_sampleRate / 1000.0) + 2;
    m_delayBuf.resize(maxSamples, 0.0);
    m_delayPos = 0;
}

/** @brief 设置采样率 @param rate 采样率 */
void Flanger::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    int maxSamples = static_cast<int>(m_maxDelay * m_sampleRate / 1000.0) + 2;
    m_delayBuf.resize(maxSamples, 0.0);
}

/** @brief 处理单个采样点 @param sample 输入 @return 输出 */
double Flanger::processOne(double sample)
{
    int bufSize = m_delayBuf.size();
    if (bufSize < 2) return sample;

    /* 更新LFO相位 — 正弦波调制 */
    m_phase += m_rate / m_sampleRate;
    if (m_phase >= 1.0) m_phase -= 1.0;

    /* 计算当前延迟时间(采样点) */
    double lfoVal = 0.5 + 0.5 * qSin(2.0 * M_PI * m_phase);
    double delayMs = m_minDelay + m_depth * (m_maxDelay - m_minDelay) * lfoVal;
    double delaySamples = delayMs * m_sampleRate / 1000.0;

    /* 从延迟线读取(线性插值) */
    double readPos = static_cast<double>(m_delayPos) - delaySamples;
    while (readPos < 0) readPos += bufSize;
    while (readPos >= bufSize) readPos -= bufSize;

    int idx0 = static_cast<int>(readPos);
    int idx1 = (idx0 + 1) % bufSize;
    double frac = readPos - static_cast<double>(idx0);

    double delayed = interpolate(frac, m_delayBuf[idx0], m_delayBuf[idx1]);

    /* 写入延迟线(输入+反馈) */
    m_delayBuf[m_delayPos] = sample + delayed * m_feedback;
    m_delayPos = (m_delayPos + 1) % bufSize;

    /* 输出 = 干信号 + 湿信号混合 */
    return sample + delayed * 0.5;
}

/** @brief 处理采样序列 @param input 输入序列 @return 处理后的序列 */
QVector<double> Flanger::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i) {
        output[i] = processOne(input[i]);
    }

    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSamplesProcessed));

    emit processingComplete(input.size());
    return output;
}

/** @brief 重置延迟线 */
void Flanger::reset()
{
    std::fill(m_delayBuf.begin(), m_delayBuf.end(), 0.0);
    m_delayPos = 0;
    m_phase = 0.0;
}

/** @brief 线性插值 @param frac 小数部分 @param a 前采样 @param b 后采样 @return 插值结果 */
double Flanger::interpolate(double frac, double a, double b) const
{
    return a + frac * (b - a);
}

/** @brief 重置统计 */
void Flanger::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
