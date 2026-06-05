/**
 * @file CircularPowerBuffer.cpp
 * @brief 2的幂次环形缓冲区实现 — O(1)位掩码读写
 */

#include "utils/circularbuffer2/CircularPowerBuffer.h"

#include <QtMath>

/** @brief 构造函数
 *  @param powerSize 容量为2^powerSize
 *  @param parent 父对象 */
CircularPowerBuffer::CircularPowerBuffer(int powerSize, QObject* parent)
    : QObject(parent)
    , m_head(0)
    , m_tail(0)
    , m_count(0)
{
    /* 确保powerSize在合理范围 */
    powerSize = qBound(1, powerSize, 24);
    m_capacity = 1 << powerSize;
    m_mask = m_capacity - 1;
    m_buffer.resize(m_capacity);
    m_buffer.fill(0.0);
}

/** @brief 写入单个值 @param value 数值 */
void CircularPowerBuffer::write(double value)
{
    m_buffer[m_head] = value;

    if (m_count == m_capacity) {
        /* 缓冲区满，推进tail(覆盖最旧数据) */
        m_tail = (m_tail + 1) & m_mask;
        ++m_stats.totalOverflows;
        emit overflowWarning(1);
    } else {
        ++m_count;
    }

    m_head = (m_head + 1) & m_mask;
    ++m_stats.totalWrites;
}

/** @brief 批量写入 @param values 数据数组 */
void CircularPowerBuffer::writeBatch(const QVector<double>& values)
{
    int writeSize = values.size();
    if (writeSize <= 0) return;

    int availableSpace = m_capacity - m_count;

    if (writeSize > availableSpace) {
        /* 溢出: 跳过被覆盖的数据 */
        int lostSamples = writeSize - availableSpace;
        m_stats.totalOverflows += static_cast<quint64>(lostSamples);
        emit overflowWarning(lostSamples);

        /* 推进tail跳过被覆盖的数据 */
        m_tail = (m_tail + lostSamples) & m_mask;
        m_count = m_capacity;
    } else {
        m_count += writeSize;
    }

    /* 写入数据 */
    for (int i = 0; i < writeSize; ++i) {
        m_buffer[m_head] = values[i];
        m_head = (m_head + 1) & m_mask;
    }

    m_stats.totalWrites += static_cast<quint64>(writeSize);
}

/** @brief 读取单个值 @return 数值(缓冲区空返回NaN) */
double CircularPowerBuffer::read()
{
    if (m_count <= 0) return qQNaN();

    double value = m_buffer[m_tail];
    m_tail = (m_tail + 1) & m_mask;
    --m_count;
    ++m_stats.totalReads;

    return value;
}

/** @brief 批量读取 @param count 读取数量 @return 数据数组 */
QVector<double> CircularPowerBuffer::readBatch(int count)
{
    int toRead = qMin(count, m_count);
    QVector<double> result;
    result.reserve(toRead);

    for (int i = 0; i < toRead; ++i) {
        result.append(m_buffer[m_tail]);
        m_tail = (m_tail + 1) & m_mask;
    }

    m_count -= toRead;
    m_stats.totalReads += static_cast<quint64>(toRead);

    return result;
}

/** @brief 可读数据量 @return 可读样本数 */
int CircularPowerBuffer::available() const
{
    return m_count;
}

/** @brief 缓冲区容量 @return 容量 */
int CircularPowerBuffer::capacity() const
{
    return m_capacity;
}

/** @brief 清空缓冲区 */
void CircularPowerBuffer::clear()
{
    m_head = 0;
    m_tail = 0;
    m_count = 0;
    m_buffer.fill(0.0);
}

/** @brief 重置统计 */
void CircularPowerBuffer::resetStatistics()
{
    m_stats = Stats{};
}
