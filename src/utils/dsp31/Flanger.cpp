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

/**
 * @brief 全通插值(改进的延迟线读取)
 * @param frac 小数部分
 * @param a 前采样
 * @param b 后采样
 * @return 插值结果
 *
 * 使用Thiran全通插值替代线性插值，
 * 在高频区域提供更好的延迟调制质量。
 */
double Flanger::allpassInterpolate(double frac, double a, double b) const
{
    /* 一阶Thiran全通插值 */
    double coeff = (1.0 - frac) / (1.0 + frac);
    return a + coeff * (b - a);
}

/**
 * @brief 批量处理并应用干湿比混合
 * @param input 输入音频序列
 * @param dryWet 干湿比[0=全干, 1=全湿]
 * @return 混合后的音频序列
 *
 * 提供干湿比控制用于将镶边效果与原始信号混合:
 * - dryWet=0: 仅输出原始信号
 * - dryWet=1: 仅输出镶边后信号
 */
QVector<double> Flanger::processWithMix(const QVector<double>& input, double dryWet)
{
    QElapsedTimer timer;
    timer.start();

    dryWet = qBound(0.0, dryWet, 1.0);
    QVector<double> output(input.size());

    for (int i = 0; i < input.size(); ++i) {
        double wet = processOne(input[i]);
        output[i] = input[i] * (1.0 - dryWet) + wet * dryWet;
    }

    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSamplesProcessed));

    emit processingComplete(input.size());
    return output;
}

/**
 * @brief 获取当前延迟时间(毫秒)
 * @return 当前LFO调制下的延迟时间
 *
 * 返回当前相位下LFO调制的实际延迟时间，
 * 可用于可视化显示调制波形。
 */
double Flanger::currentDelayMs() const
{
    double lfoVal = 0.5 + 0.5 * qSin(2.0 * M_PI * m_phase);
    return m_minDelay + m_depth * (m_maxDelay - m_minDelay) * lfoVal;
}

/**
 * @brief 计算镶边器的梳状滤波器频率响应
 * @param freq 频率(Hz)
 * @return 幅度响应
 *
 * 镶边器本质上是延迟调制型梳状滤波器，
 * 其频率响应在等间隔频率处产生峰值和谷值。
 * 峰值频率 = n / delay_time (n=0,1,2,...)
 */
double Flanger::frequencyResponse(double freq) const
{
    double delayMs = currentDelayMs();
    double delaySec = delayMs / 1000.0;
    double phase = 2.0 * M_PI * freq * delaySec;

    /* 简化梳状滤波器响应: |1 + gain * exp(-j*phase)| */
    double realPart = 1.0 + 0.5 * qCos(phase);
    double imagPart = -0.5 * qSin(phase);
    return qSqrt(realPart * realPart + imagPart * imagPart);
}

/** @brief 重置统计 */
void Flanger::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
