/**
 * @file DelayLine2.cpp
 * @brief 延迟线增强实现 — 可变/分数延迟/多抽头/插值
 */

#include "utils/dsp32/DelayLine2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DelayLine2::DelayLine2(QObject* parent)
    : QObject(parent)
{
    m_buffer.resize(m_maxDelay, 0.0);
}

/** @brief 设置延迟采样数(支持小数) @param samples 延迟量(采样) */
void DelayLine2::setDelaySamples(double samples)
{
    m_delay = qBound(0.0, samples, static_cast<double>(m_maxDelay - 1));
    m_readPos = static_cast<double>(m_writePos) - m_delay;
    if (m_readPos < 0.0) {
        m_readPos += static_cast<double>(m_maxDelay);
    }
}

/** @brief 设置最大延迟 @param maxSamples 最大延迟采样数 */
void DelayLine2::setMaxDelay(int maxSamples)
{
    m_maxDelay = qMax(2, maxSamples);
    m_buffer.resize(m_maxDelay, 0.0);
    m_writePos = 0;
    m_readPos = 0.0;
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void DelayLine2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置插值类型(0=最近,1=线性,2=三次) @param type 插值类型 */
void DelayLine2::setInterpolation(int type)
{
    m_interpType = qBound(0, type, 2);
}

/**
 * @brief 处理单个采样
 * @param sample 输入采样
 * @return 延迟后的采样
 */
double DelayLine2::processOne(double sample)
{
    QElapsedTimer timer;
    timer.start();

    /* 写入新采样到环形缓冲 */
    m_buffer[m_writePos] = sample;

    /* 从读位置读取(可能含小数部分) */
    double output = interpolateRead(m_readPos);

    /* 推进写指针 */
    m_writePos = (m_writePos + 1) % m_maxDelay;

    /* 推进读指针 */
    m_readPos += 1.0;
    if (m_readPos >= static_cast<double>(m_maxDelay)) {
        m_readPos -= static_cast<double>(m_maxDelay);
    }

    m_stats.totalSamplesProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSamplesProcessed));

    return output;
}

/**
 * @brief 批量处理采样
 * @param input 输入采样序列
 * @return 延迟后的采样序列
 */
QVector<double> DelayLine2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return {};

    QVector<double> output;
    output.reserve(input.size());

    for (int i = 0; i < input.size(); ++i) {
        /* 写入 */
        m_buffer[m_writePos] = input[i];

        /* 读取延迟 */
        double delayed = interpolateRead(m_readPos);
        output.append(delayed);

        /* 推进指针 */
        m_writePos = (m_writePos + 1) % m_maxDelay;
        m_readPos += 1.0;
        if (m_readPos >= static_cast<double>(m_maxDelay)) {
            m_readPos -= static_cast<double>(m_maxDelay);
        }
    }

    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSamplesProcessed));

    emit processingComplete(input.size());
    return output;
}

/**
 * @brief 从指定偏移量读取延迟采样
 * @param offset 相对于当前写位置的偏移(正数=过去)
 * @return 采样值
 */
double DelayLine2::tap(int offset) const
{
    if (offset < 0 || offset >= m_maxDelay) return 0.0;

    int idx = m_writePos - offset - 1;
    if (idx < 0) idx += m_maxDelay;
    if (idx < 0 || idx >= m_maxDelay) return 0.0;

    return m_buffer[idx];
}

/**
 * @brief 多抽头读取
 * @param offsets 偏移量列表
 * @return 采样值列表
 */
QVector<double> DelayLine2::multiTap(const QVector<int>& offsets) const
{
    QVector<double> result;
    result.reserve(offsets.size());
    for (int off : offsets) {
        result.append(tap(off));
    }
    return result;
}

/** @brief 重置延迟线状态 */
void DelayLine2::reset()
{
    m_buffer.fill(0.0);
    m_writePos = 0;
    m_readPos = 0.0;
}

/** @brief 重置统计 */
void DelayLine2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 从小数位置插值读取
 * @param fracPos 小数位置
 * @return 插值结果
 */
double DelayLine2::interpolateRead(double fracPos) const
{
    /* 归一化到合法范围 */
    while (fracPos < 0.0) fracPos += static_cast<double>(m_maxDelay);
    while (fracPos >= static_cast<double>(m_maxDelay))
        fracPos -= static_cast<double>(m_maxDelay);

    int idx0 = static_cast<int>(fracPos);
    double frac = fracPos - static_cast<double>(idx0);

    switch (m_interpType) {
    case 0: {
        /* 最近邻插值 */
        int nearest = (frac < 0.5) ? idx0 : (idx0 + 1) % m_maxDelay;
        return m_buffer[nearest];
    }
    case 1: {
        /* 线性插值 */
        int idx1 = (idx0 + 1) % m_maxDelay;
        return m_buffer[idx0] * (1.0 - frac) + m_buffer[idx1] * frac;
    }
    case 2: {
        /* 三次Hermite插值(4点) */
        int idxM1 = (idx0 - 1 + m_maxDelay) % m_maxDelay;
        int idx0c = idx0;
        int idx1 = (idx0 + 1) % m_maxDelay;
        int idx2 = (idx0 + 2) % m_maxDelay;

        double y0 = m_buffer[idxM1];
        double y1 = m_buffer[idx0c];
        double y2 = m_buffer[idx1];
        double y3 = m_buffer[idx2];

        /* Hermite基函数 */
        double a = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
        double b = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
        double c = -0.5 * y0 + 0.5 * y2;
        double d = y1;

        return a * frac * frac * frac + b * frac * frac + c * frac + d;
    }
    default:
        return m_buffer[idx0];
    }
}
