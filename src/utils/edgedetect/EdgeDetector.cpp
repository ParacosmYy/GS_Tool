/**
 * @file EdgeDetector.cpp
 * @brief 信号边沿检测器实现 — 上升/下降沿检测+消抖
 */

#include "utils/edgedetect/EdgeDetector.h"

/** @brief 构造函数 @param type 边沿类型 @param parent 父对象 */
EdgeDetector::EdgeDetector(EdgeType type, QObject* parent)
    : QObject(parent)
    , m_edgeType(type)
    , m_debounceSamples(1)
    , m_lastStableLevel(false)
    , m_sampleIndex(0)
{
}

/** @brief 处理一个采样值 @param value 当前采样值 @param threshold 阈值 */
void EdgeDetector::process(double value, double threshold)
{
    bool currentLevel = (value >= threshold);

    /* 写入消抖缓冲区 */
    m_debounceBuffer.push_back(currentLevel);
    if (static_cast<int>(m_debounceBuffer.size()) > m_debounceSamples) {
        m_debounceBuffer.pop_front();
    }

    /* 消抖缓冲区填满后才判断 */
    if (static_cast<int>(m_debounceBuffer.size()) < m_debounceSamples) {
        m_sampleIndex++;
        m_stats.totalSamples++;
        return;
    }

    /* 检查缓冲区是否全部为同一电平 */
    bool allHigh = checkDebounce(true);
    bool allLow = checkDebounce(false);

    if (allHigh && !m_lastStableLevel) {
        /* 上升沿: 从低变高 */
        m_lastStableLevel = true;
        m_stats.totalRisingEdges++;
        m_risingEdgeTimes.append(m_sampleIndex);
        if (m_edgeType & Rising) {
            emit risingEdge(m_sampleIndex);
        }
    } else if (allLow && m_lastStableLevel) {
        /* 下降沿: 从高变低 */
        m_lastStableLevel = false;
        m_stats.totalFallingEdges++;
        m_fallingEdgeTimes.append(m_sampleIndex);
        if (m_edgeType & Falling) {
            emit fallingEdge(m_sampleIndex);
        }
    }

    m_sampleIndex++;
    m_stats.totalSamples++;
}

/** @brief 设置消抖采样数 @param samples 消抖采样数(≥1) */
void EdgeDetector::setDebounceSamples(int samples)
{
    m_debounceSamples = (samples < 1) ? 1 : samples;
    m_debounceBuffer.clear();
}

/** @brief 获取所有边沿发生的采样索引 @return 边沿时刻索引列表 */
QVector<int> EdgeDetector::getEdgeTimes() const
{
    QVector<int> all;
    all.reserve(m_risingEdgeTimes.size() + m_fallingEdgeTimes.size());
    all.append(m_risingEdgeTimes);
    all.append(m_fallingEdgeTimes);
    /* 按时间排序 */
    std::sort(all.begin(), all.end());
    return all;
}

/** @brief 重置统计 */
void EdgeDetector::resetStatistics()
{
    m_stats = Stats{};
    m_sampleIndex = 0;
    m_lastStableLevel = false;
    m_debounceBuffer.clear();
    m_risingEdgeTimes.clear();
    m_fallingEdgeTimes.clear();
}

/** @brief 检查消抖缓冲区是否全部为指定电平 @param level 目标电平 @return 是否满足 */
bool EdgeDetector::checkDebounce(bool level) const
{
    for (bool val : m_debounceBuffer) {
        if (val != level) return false;
    }
    return true;
}
