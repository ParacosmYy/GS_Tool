/**
 * @file FrequencyCounter2.cpp
 * @brief 高精度频率计数器实现 — 边沿检测+频率/占空比测量
 */

#include "utils/freq/FrequencyCounter2.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
FrequencyCounter2::FrequencyCounter2(QObject* parent)
    : QObject(parent)
    , m_nextId(1)
{
}

/** @brief 添加通道 @param config 配置 @return 通道ID */
int FrequencyCounter2::addChannel(const ChannelConfig& config)
{
    int id = m_nextId++;
    ChannelState state;
    state.config = config;
    state.lastValue = 0.0;
    state.lastRisingEdge = 0;
    state.lastFallingEdge = 0;
    state.lastHigh = false;
    state.pulseCount = 0;
    m_channels[id] = state;
    return id;
}

/** @brief 移除通道 @param id 通道ID */
void FrequencyCounter2::removeChannel(int id)
{
    m_channels.remove(id);
}

/** @brief 输入采样 @param channel 通道 @param value 值 @param timestampUs 时间戳 */
void FrequencyCounter2::feed(int channel, double value, qint64 timestampUs)
{
    auto it = m_channels.find(channel);
    if (it == m_channels.end()) return;

    ChannelState& state = it.value();
    double threshold = state.config.triggerLevel;
    double hyst = state.config.hysteresis;

    bool nowHigh = state.lastHigh;

    if (!state.lastHigh) {
        if (value > threshold + hyst) {
            nowHigh = true;
        }
    } else {
        if (value < threshold - hyst) {
            nowHigh = false;
        }
    }

    if (nowHigh && !state.lastHigh) {
        /* 上升沿 */
        if (state.lastRisingEdge > 0) {
            qint64 period = timestampUs - state.lastRisingEdge;
            if (period > 0) {
                state.lastMeasurement.period = static_cast<double>(period) / 1e6;
                state.lastMeasurement.frequency = 1e6 / static_cast<double>(period);
            }
        }
        state.lastRisingEdge = timestampUs;
        state.lastMeasurement.riseTime = 0.0;
        state.lastMeasurement.edgeCount++;

        ++m_stats.totalEdges;
        ++m_stats.edgesByChannel[channel];
        ++m_stats.totalMeasurements;

        emit edgeDetected(channel, true);
        if (state.lastMeasurement.frequency > 0) {
            emit frequencyUpdated(channel, state.lastMeasurement.frequency);
        }
    }

    if (!nowHigh && state.lastHigh) {
        /* 下降沿 */
        if (state.lastRisingEdge > 0) {
            qint64 pulseW = timestampUs - state.lastRisingEdge;
            state.lastMeasurement.pulseWidth = static_cast<double>(pulseW) / 1e6;

            if (state.lastMeasurement.period > 0) {
                state.lastMeasurement.dutyCycle =
                    state.lastMeasurement.pulseWidth / state.lastMeasurement.period * 100.0;
            }
        }
        state.lastFallingEdge = timestampUs;
        state.lastMeasurement.edgeCount++;
        emit edgeDetected(channel, false);
    }

    state.lastValue = value;
    state.lastHigh = nowHigh;
}

/** @brief 获取测量结果 @param channel 通道 @return 测量 */
FrequencyCounter2::Measurement FrequencyCounter2::measurement(int channel) const
{
    auto it = m_channels.constFind(channel);
    if (it != m_channels.constEnd()) {
        return it.value().lastMeasurement;
    }
    return Measurement{};
}

/** @brief 重置统计 */
void FrequencyCounter2::resetStatistics()
{
    m_stats = Stats{};
    for (auto& state : m_channels) {
        state.lastMeasurement = Measurement{};
        state.pulseCount = 0;
    }
}
