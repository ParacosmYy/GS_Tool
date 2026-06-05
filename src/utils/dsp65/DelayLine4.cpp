/**
 * @file DelayLine4.cpp
 * @brief 延迟线音效处理器实现（第4版）
 *
 * 实现可配置的音频延迟线效果，支持多种延迟模式（普通、乒乓、
 * 多抽头）。具有反馈回路和干湿比控制。延迟缓冲区使用环形缓冲
 * 实现高效读写。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp65/DelayLine4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化延迟线处理器
 * @param parent 父QObject对象指针
 */
DelayLine4::DelayLine4(QObject* parent)
    : QObject(parent)
{
    /* 初始化默认缓冲区（100ms @ 44.1kHz） */
    int bufSize = static_cast<int>(m_delayMs * m_sampleRate / 1000.0);
    m_buffer.resize(qMax(bufSize, 1), 0.0);
    m_writePos = 0;
}

/**
 * @brief 设置延迟时间
 * @param ms 延迟时间（毫秒），范围1ms~5000ms
 */
void DelayLine4::setDelayTime(double ms)
{
    m_delayMs = qBound(1.0, ms, 5000.0);
    /* 重新分配缓冲区 */
    int newSize = static_cast<int>(m_delayMs * m_sampleRate / 1000.0);
    newSize = qMax(newSize, 1);
    m_buffer.resize(newSize, 0.0);
    m_writePos = m_writePos % m_buffer.size();
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz），影响缓冲区大小计算
 */
void DelayLine4::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
    /* 重新分配缓冲区 */
    int newSize = static_cast<int>(m_delayMs * m_sampleRate / 1000.0);
    newSize = qMax(newSize, 1);
    m_buffer.resize(newSize, 0.0);
    m_writePos = m_writePos % m_buffer.size();
}

/**
 * @brief 设置反馈系数
 * @param fb 反馈量（0.0~0.99），越大延迟尾音越长
 */
void DelayLine4::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
}

/**
 * @brief 设置干湿混合比例
 * @param mix 湿信号比例（0.0=全干, 1.0=全湿）
 */
void DelayLine4::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

/**
 * @brief 设置延迟模式
 * @param mode 延迟模式："normal"标准延迟, "pingpong"乒乓延迟, "multi"多抽头
 */
void DelayLine4::setMode(const QString& mode)
{
    if (mode == "normal" || mode == "pingpong" || mode == "multi") {
        m_mode = mode;
    }
}

/**
 * @brief 从延迟缓冲区中读取指定延迟量的样本
 * @param delaySamples 延迟采样数
 * @return 延迟后的样本值
 */
double DelayLine4::readSample(int delaySamples)
{
    int bufSize = m_buffer.size();
    delaySamples = qBound(0, delaySamples, bufSize - 1);
    int readPos = (m_writePos - delaySamples + bufSize) % bufSize;
    return m_buffer[readPos];
}

/**
 * @brief 向缓冲区写入一个样本
 * @param sample 输入样本值
 */
void DelayLine4::writeSample(double sample)
{
    m_buffer[m_writePos] = sample;
    m_writePos = (m_writePos + 1) % m_buffer.size();
}

/**
 * @brief 处理输入音频信号，添加延迟效果
 *
 * 根据当前模式处理音频：
 * - normal: 标准单延迟线
 * - pingpong: 左右交替的乒乓延迟
 * - multi: 多个抽头的多延迟效果
 *
 * @param input 输入音频采样序列
 * @return 添加延迟效果后的音频序列
 */
QVector<double> DelayLine4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) {
        emit processingCompleted(0, m_delayMs);
        return output;
    }

    int n = input.size();
    output.resize(n);

    int delaySamples = static_cast<int>(m_delayMs * m_sampleRate / 1000.0);
    delaySamples = qBound(1, delaySamples, m_buffer.size() - 1);

    if (m_mode == "normal") {
        /* 标准延迟模式 */
        for (int i = 0; i < n; ++i) {
            double delayed = readSample(delaySamples);
            double feedback = input[i] + delayed * m_feedback;
            writeSample(feedback);
            output[i] = input[i] * (1.0 - m_mix) + delayed * m_mix;
        }
    } else if (m_mode == "pingpong") {
        /* 乒乓延迟：交替发送到左右通道模拟 */
        int halfDelay = delaySamples / 2;
        for (int i = 0; i < n; ++i) {
            double delayed = readSample(halfDelay);
            double pingpong = (i % 2 == 0) ? delayed : readSample(delaySamples);
            double feedback = input[i] + pingpong * m_feedback;
            writeSample(feedback);
            output[i] = input[i] * (1.0 - m_mix) + pingpong * m_mix;
        }
    } else {
        /* 多抽头延迟模式 */
        int taps = 4;
        for (int i = 0; i < n; ++i) {
            double delayedSum = 0.0;
            for (int t = 1; t <= taps; ++t) {
                int tapDelay = delaySamples * t / taps;
                double tapVal = readSample(qMax(1, tapDelay));
                delayedSum += tapVal * (1.0 / t);
            }
            double feedback = input[i] + delayedSum * m_feedback;
            writeSample(feedback);
            output[i] = input[i] * (1.0 - m_mix) + delayedSum * m_mix;
        }
    }

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, m_delayMs);
    return output;
}

/**
 * @brief 获取当前统计信息
 * @return 处理统计结构
 */
DelayLine4::Stats DelayLine4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void DelayLine4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
