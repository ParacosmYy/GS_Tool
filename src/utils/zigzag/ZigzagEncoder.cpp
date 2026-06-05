/**
 * @file ZigzagEncoder.cpp
 * @brief ZigZag编码器实现
 */

#include "ZigzagEncoder.h"
#include <QElapsedTimer>

ZigzagEncoder::ZigzagEncoder(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

quint32 ZigzagEncoder::encode32(qint32 value) const
{
    QElapsedTimer timer;
    timer.start();

    quint32 result = static_cast<quint32>((value << 1) ^ (value >> 31));

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

qint32 ZigzagEncoder::decode32(quint32 value) const
{
    QElapsedTimer timer;
    timer.start();

    qint32 result = static_cast<qint32>((value >> 1) ^ -(static_cast<qint32>(value) & 1));

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

quint64 ZigzagEncoder::encode64(qint64 value) const
{
    QElapsedTimer timer;
    timer.start();

    quint64 result = static_cast<quint64>((value << 1) ^ (value >> 63));

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

qint64 ZigzagEncoder::decode64(quint64 value) const
{
    QElapsedTimer timer;
    timer.start();

    qint64 result = static_cast<qint64>((value >> 1) ^ -(static_cast<qint64>(value) & 1));

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

QVector<quint32> ZigzagEncoder::encodeBatch32(const QVector<qint32>& values) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint32> result;
    result.reserve(values.size());
    for (qint32 v : values)
        result.append(static_cast<quint32>((v << 1) ^ (v >> 31)));

    m_stats.totalEncoded += values.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encodeCompleted(values.size());
    return result;
}

QVector<qint32> ZigzagEncoder::decodeBatch32(const QVector<quint32>& values) const
{
    QVector<qint32> result;
    result.reserve(values.size());
    for (quint32 v : values)
        result.append(static_cast<qint32>((v >> 1) ^ -(static_cast<qint32>(v) & 1)));

    m_stats.totalDecoded += values.size();
    return result;
}

QByteArray ZigzagEncoder::encodeVarint(qint64 value) const
{
    QElapsedTimer timer;
    timer.start();

    quint64 zigzag = encode64(value);
    QByteArray result;

    while (zigzag > 0x7F) {
        result.append(static_cast<char>((zigzag & 0x7F) | 0x80));
        zigzag >>= 7;
    }
    result.append(static_cast<char>(zigzag & 0x7F));

    m_stats.totalEncodedBytes += result.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

qint64 ZigzagEncoder::decodeVarint(const QByteArray& data, int* bytesRead) const
{
    QElapsedTimer timer;
    timer.start();

    quint64 result = 0;
    int shift = 0;
    int idx = 0;

    while (idx < data.size()) {
        quint8 byte = static_cast<quint8>(data[idx]);
        result |= static_cast<quint64>(byte & 0x7F) << shift;
        shift += 7;
        idx++;
        if (!(byte & 0x80)) break;
    }

    if (bytesRead) *bytesRead = idx;

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return decode64(result);
}

ZigzagEncoder::Stats ZigzagEncoder::stats() const { return m_stats; }

void ZigzagEncoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
