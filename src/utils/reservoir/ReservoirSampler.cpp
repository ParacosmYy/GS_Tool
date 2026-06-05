/**
 * @file ReservoirSampler.cpp
 * @brief 水塘采样器实现 — Algorithm R
 */

#include "utils/reservoir/ReservoirSampler.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param reservoirSize 水塘大小 @param parent 父对象 */
ReservoirSampler::ReservoirSampler(int reservoirSize, QObject* parent)
    : QObject(parent)
    , m_reservoirSize(reservoirSize > 0 ? reservoirSize : 100)
    , m_rng(QRandomGenerator::global()->generate())
    , m_timeSum(0.0)
{
    m_reservoir.reserve(m_reservoirSize);
}

/** @brief 添加一个数据点 @param value 数据值 */
void ReservoirSampler::add(double value)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalProcessed++;

    if (static_cast<int>(m_reservoir.size()) < m_reservoirSize) {
        /* 水塘未满，直接加入 */
        m_reservoir.append(value);
    } else {
        /* 水塘已满，以 m_reservoirSize/N 的概率替换 */
        quint64 n = m_stats.totalProcessed;
        int j = m_rng.bounded(static_cast<qint32>(n));
        if (j < m_reservoirSize) {
            m_reservoir[j] = value;
        }
    }

    m_stats.totalSamples = m_reservoir.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessed > 0)
        ? m_timeSum / m_stats.totalProcessed : 0.0;

    emit sampleAdded(value);
}

/** @brief 批量添加数据点 @param values 数据值列表 */
void ReservoirSampler::addBatch(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    for (double val : values) {
        m_stats.totalProcessed++;

        if (static_cast<int>(m_reservoir.size()) < m_reservoirSize) {
            m_reservoir.append(val);
        } else {
            quint64 n = m_stats.totalProcessed;
            int j = m_rng.bounded(static_cast<qint32>(n));
            if (j < m_reservoirSize) {
                m_reservoir[j] = val;
            }
        }
    }

    m_stats.totalSamples = m_reservoir.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessed > 0)
        ? m_timeSum / m_stats.totalProcessed : 0.0;
}

/** @brief 获取当前水塘样本 @return 采样结果 */
QVector<double> ReservoirSampler::samples() const
{
    return m_reservoir;
}

/** @brief 重置采样器 */
void ReservoirSampler::reset()
{
    m_reservoir.clear();
    m_stats.totalSamples = 0;
    m_stats.totalProcessed = 0;
}

/** @brief 设置水塘大小 @param size 新大小 */
void ReservoirSampler::setReservoirSize(int size)
{
    if (size < 1) size = 1;
    m_reservoirSize = size;
    /* 改变大小需要清空已有样本 */
    m_reservoir.clear();
    m_reservoir.reserve(m_reservoirSize);
    m_stats.totalSamples = 0;
    m_stats.totalProcessed = 0;
}

/** @brief 重置统计 */
void ReservoirSampler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
