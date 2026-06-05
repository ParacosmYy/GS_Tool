/**
 * @file LogicSampler.cpp
 * @brief 数字信号采集引擎实现 -- 多通道采样、触发检测、预触发缓冲
 *
 * 实现细节:
 *   - 原始数据通过 feedData() 馈入，每字节对应一个采样点
 *   - 时间戳按配置采样率自动递增(ns精度)
 *   - 触发检测: 在 feedData 流中逐样本匹配通道电平条件
 *   - 预触发缓冲: 环形缓冲区保存触发前的历史样本
 *   - 达到目标采样数后自动停止采集
 */

#include "protocol/logic/LogicSampler.h"

#include <QElapsedTimer>

/** @brief 构造数字信号采集引擎 @param parent 父QObject指针 */
LogicSampler::LogicSampler(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 配置采集参数
 * @param channelCount 通道数量(1~8)
 * @param sampleRateHz 采样率(Hz)
 * @param sampleCount 目标采样数量
 */
void LogicSampler::configure(int channelCount, double sampleRateHz,
                              quint64 sampleCount)
{
    m_channelCount = qBound(1, channelCount, 8);
    m_sampleRateHz = qMax(1.0, sampleRateHz);
    m_sampleCount  = qMax<quint64>(1, sampleCount);

    /* 预触发缓冲区大小: 目标采样数的10%，最小1024 */
    const int preSize = qMax(1024,
        static_cast<int>(m_sampleCount / 10));
    m_preTriggerBuf.resize(preSize);
    m_preTriggerIdx = 0;
}

/**
 * @brief 启动采集会话
 * @return 配置有效且当前未在采集时返回true
 */
bool LogicSampler::start()
{
    if (m_running) {
        return false;
    }
    if (m_channelCount < 1 || m_sampleRateHz <= 0.0) {
        return false;
    }

    m_running         = true;
    m_triggerArmed    = true;
    m_triggerMatched  = false;
    m_nextTimestamp    = 0;
    m_captureStartNs  = 0;
    m_samples.clear();
    m_preTriggerIdx   = 0;

    QElapsedTimer timer;
    timer.start();
    m_captureStartNs = static_cast<quint64>(timer.nsecsElapsed());

    ++m_totalCaptureSessions;
    emit captureStarted();
    return true;
}

/** @brief 停止当前采集会话 */
void LogicSampler::stop()
{
    if (!m_running) {
        return;
    }

    m_running = false;

    /* 计算采集持续时间(ms) */
    const double nsPerSample = 1e9 / m_sampleRateHz;
    m_captureDurationMs =
        (static_cast<double>(m_samples.size()) * nsPerSample) / 1e6;

    emit captureStopped();
}

/** @brief 查询采集是否正在进行 */
bool LogicSampler::isRunning() const
{
    return m_running;
}

/**
 * @brief 馈入原始采样数据
 * @param rawSamples 原始字节流，每字节低N位对应N个通道电平
 *
 * 数据处理流程:
 *   1. 解析原始字节为LogicSample序列
 *   2. 若触发已激活且未匹配，逐样本检测触发条件
 *   3. 触发匹配后，追加预触发缓冲区 + 后续样本到输出缓冲
 *   4. 达到目标采样数后自动停止
 */
void LogicSampler::feedData(const QByteArray& rawSamples)
{
    if (!m_running || rawSamples.isEmpty()) {
        return;
    }

    m_totalBytesReceived += static_cast<quint64>(rawSamples.size());

    const QVector<LogicSample> parsed = parseRawData(rawSamples);
    if (parsed.isEmpty()) {
        return;
    }

    m_totalSamplesProcessed += static_cast<quint64>(parsed.size());

    /* 推进时间戳: 下批数据从当前末尾继续 */
    if (!parsed.isEmpty()) {
        const quint64 tsStep = static_cast<quint64>(1e9 / m_sampleRateHz);
        m_nextTimestamp = parsed.last().timestamp + tsStep;
    }

    QVector<LogicSample> newSamples;
    newSamples.reserve(parsed.size());

    for (const LogicSample& sample : parsed) {
        if (!m_triggerMatched) {
            /* 触发匹配前: 写入预触发环形缓冲区 */
            if (!m_preTriggerBuf.isEmpty()) {
                m_preTriggerBuf[m_preTriggerIdx] = sample;
                m_preTriggerIdx =
                    (m_preTriggerIdx + 1) % m_preTriggerBuf.size();
            }

            /* 检测触发条件 */
            if (m_triggerArmed && checkTrigger(sample)) {
                m_triggerMatched = true;
                m_triggerArmed   = false;
                ++m_totalTriggersFired;

                emit triggerFired(
                    static_cast<quint64>(m_samples.size()));

                /* 将预触发缓冲区的内容按序追加到输出 */
                const int bufSize = m_preTriggerBuf.size();
                for (int i = 0; i < bufSize; ++i) {
                    const int idx =
                        (m_preTriggerIdx + i) % bufSize;
                    m_samples.append(m_preTriggerBuf[idx]);
                }
                m_samples.append(sample);
                newSamples.append(sample);
            }
        } else {
            /* 触发匹配后: 直接追加样本 */
            m_samples.append(sample);
            newSamples.append(sample);

            /* 达到目标采样数: 自动停止 */
            if (static_cast<quint64>(m_samples.size()) >= m_sampleCount) {
                if (!newSamples.isEmpty()) {
                    emit dataAvailable(newSamples);
                }
                stop();
                return;
            }
        }
    }

    if (!newSamples.isEmpty()) {
        emit dataAvailable(newSamples);
    }
}

/** @brief 获取已采集的样本列表 */
QVector<LogicSample> LogicSampler::samples() const
{
    return m_samples;
}

/** @brief 获取配置的通道数量 */
int LogicSampler::channelCount() const
{
    return m_channelCount;
}

/** @brief 获取配置的采样率(Hz) */
double LogicSampler::sampleRate() const
{
    return m_sampleRateHz;
}

/** @brief 获取配置的目标采样数量 */
quint64 LogicSampler::sampleCount() const
{
    return m_sampleCount;
}

/** @brief 获取已采集的样本数量 */
quint64 LogicSampler::capturedSamples() const
{
    return static_cast<quint64>(m_samples.size());
}

/** @brief 设置触发条件 */
void LogicSampler::setTrigger(const LogicTrigger& trigger)
{
    m_trigger = trigger;
}

/** @brief 获取当前触发条件 */
LogicTrigger LogicSampler::trigger() const
{
    return m_trigger;
}

/**
 * @brief 检测触发条件
 * @param sample 当前采样点
 * @return true=触发通道的电平匹配触发条件
 */
bool LogicSampler::checkTrigger(const LogicSample& sample) const
{
    if (m_trigger.channelIndex < 0 ||
        m_trigger.channelIndex >= m_channelCount) {
        return false;
    }

    const quint8 bitMask = static_cast<quint8>(1)
                           << m_trigger.channelIndex;
    const bool isHigh = (sample.channelMask & bitMask) != 0;

    switch (m_trigger.level) {
    case LogicLevel::High:      return isHigh;
    case LogicLevel::Low:       return !isHigh;
    case LogicLevel::Undefined: return false;
    }
    return false;
}

/**
 * @brief 将原始字节解析为LogicSample序列
 * @param rawBytes 原始字节流
 * @return 解析后的样本列表，时间戳按采样率间隔递增
 *
 * 每字节的低 channelCount 位作为通道掩码，
 * 时间戳增量为 1e9 / sampleRateHz 纳秒。
 */
QVector<LogicSample> LogicSampler::parseRawData(
    const QByteArray& rawBytes) const
{
    QVector<LogicSample> result;
    result.reserve(rawBytes.size());

    const quint8 chanMask =
        (m_channelCount >= 8)
            ? 0xFF
            : static_cast<quint8>((1 << m_channelCount) - 1);

    const quint64 tsStep = static_cast<quint64>(
        1e9 / m_sampleRateHz);

    quint64 ts = m_nextTimestamp;
    for (int i = 0; i < rawBytes.size(); ++i) {
        LogicSample s;
        s.timestamp   = ts;
        s.channelMask = static_cast<quint8>(rawBytes[i]) & chanMask;
        result.append(s);
        ts += tsStep;
    }

    /* 更新下一个时间戳(通过const_cast跳过const) --
       注意: 本方法为const，时间戳推进由调用方(feedData)保证
       这里仅做解析，不修改状态 */
    return result;
}
