/**
 * @file Limiter.cpp
 * @brief 音频限幅器实现 — 峰值限制/lookahead/增益平滑/砖墙限制
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp29/Limiter.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
Limiter::Limiter(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置阈值(单位dB) @param thresholdDb 阈值分贝值 */
void Limiter::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/** @brief 设置释放时间(单位ms) @param ms 释放时间毫秒数 */
void Limiter::setReleaseTime(double ms)
{
    m_release = qMax(0.1, ms);
}

/** @brief 设置前看缓冲区大小 @param samples 前看采样数 */
void Limiter::setLookaheadSamples(int samples)
{
    m_lookahead = qMax(0, samples);
    m_delayBuffer.fill(0.0, m_lookahead);
    m_delayPos = 0;
}

/** @brief 设置输出上限(单位dB) @param ceilingDb 上限分贝值 */
void Limiter::setCeiling(double ceilingDb)
{
    m_ceiling = ceilingDb;
}

/** @brief 计算给定输入电平所需的增益衰减量(dB)
 *  @param inputLevel 输入电平(线性值)
 *  @return 所需增益衰减量(dB，负值)
 */
double Limiter::computeGainReduction(double inputLevel)
{
    double inputDb = (inputLevel > 1e-10) ? 20.0 * qLn(inputLevel) / qLn(10.0) : -120.0;
    if (inputDb <= m_threshold) {
        return 0.0;
    }
    /* 砖墙限制: 超过阈值的部分全部衰减 */
    double excess = inputDb - m_threshold;
    return -(excess + qMax(0.0, inputDb - m_ceiling));
}

/** @brief 处理单个采样点
 *  @param sample 输入采样值(-1.0 ~ 1.0)
 *  @return 限幅后的采样值
 */
double Limiter::processOne(double sample)
{
    QElapsedTimer timer;
    timer.start();

    double absVal = qAbs(sample);
    if (absVal > m_peakLevel) {
        m_peakLevel = absVal;
    }

    /* 计算输入电平 */
    double inputLevel = absVal;
    double targetGr = computeGainReduction(inputLevel);

    /* 增益平滑: 攻击瞬时，释放按时间常数衰减 */
    double releaseCoeff = qExp(-1.0 / (m_release * m_sampleRate / 1000.0));
    if (targetGr < m_gainReduction) {
        /* 攻击: 立即跟随 */
        m_gainReduction = targetGr;
    } else {
        /* 释放: 按指数平滑 */
        m_gainReduction = releaseCoeff * m_gainReduction +
                          (1.0 - releaseCoeff) * targetGr;
    }

    /* 前看延迟缓冲区 */
    double delayed = sample;
    if (m_lookahead > 0) {
        delayed = m_delayBuffer[m_delayPos];
        m_delayBuffer[m_delayPos] = sample;
        m_delayPos = (m_delayPos + 1) % m_lookahead;
    }

    /* 应用增益衰减 */
    double gainLin = qPow(10.0, m_gainReduction / 20.0);
    double output = delayed * gainLin;

    /* 砖墙裁剪 */
    double ceilingLin = qPow(10.0, m_ceiling / 20.0);
    if (qAbs(output) > ceilingLin) {
        output = (output > 0) ? ceilingLin : -ceilingLin;
        m_stats.totalClippingEvents++;
        emit clippingDetected(qAbs(output));
    }

    m_stats.totalSamplesProcessed++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalSamplesProcessed);

    return output;
}

/** @brief 批量处理采样数据 @param input 输入采样向量 @return 限幅后的采样向量 */
QVector<double> Limiter::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(input.size());

    double releaseCoeff = qExp(-1.0 / (m_release * m_sampleRate / 1000.0));
    double ceilingLin = qPow(10.0, m_ceiling / 20.0);

    for (double sample : input) {
        double absVal = qAbs(sample);
        if (absVal > m_peakLevel) {
            m_peakLevel = absVal;
        }

        double targetGr = computeGainReduction(absVal);
        if (targetGr < m_gainReduction) {
            m_gainReduction = targetGr;
        } else {
            m_gainReduction = releaseCoeff * m_gainReduction +
                              (1.0 - releaseCoeff) * targetGr;
        }

        double delayed = sample;
        if (m_lookahead > 0) {
            delayed = m_delayBuffer[m_delayPos];
            m_delayBuffer[m_delayPos] = sample;
            m_delayPos = (m_delayPos + 1) % m_lookahead;
        }

        double gainLin = qPow(10.0, m_gainReduction / 20.0);
        double out = delayed * gainLin;

        if (qAbs(out) > ceilingLin) {
            out = (out > 0) ? ceilingLin : -ceilingLin;
            m_stats.totalClippingEvents++;
        }
        output.append(out);
    }

    m_stats.totalSamplesProcessed += input.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalSamplesProcessed);

    if (m_stats.totalClippingEvents > 0) {
        double peakDb = (m_peakLevel > 1e-10) ?
            20.0 * qLn(m_peakLevel) / qLn(10.0) : -120.0;
        m_stats.peakReductionDb = peakDb - m_ceiling;
    }
    return output;
}

/** @brief 获取当前增益衰减量 @return 增益衰减(dB) */
double Limiter::gainReduction() const
{
    return m_gainReduction;
}

/** @brief 获取峰值电平 @return 峰值(线性值) */
double Limiter::peakLevel() const
{
    return m_peakLevel;
}

/** @brief 重置内部状态(延迟缓冲区、增益、峰值) */
void Limiter::reset()
{
    m_delayBuffer.fill(0.0);
    m_delayPos = 0;
    m_gainReduction = 0.0;
    m_peakLevel = 0.0;
}

/** @brief 重置所有统计计数器 */
void Limiter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
