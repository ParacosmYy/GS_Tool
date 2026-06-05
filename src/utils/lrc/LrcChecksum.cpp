/**
 * @file LrcChecksum.cpp
 * @brief LRC校验和实现
 */

#include "utils/lrc/LrcChecksum.h"

LrcChecksum::LrcChecksum(QObject* parent) : QObject(parent) {}

quint8 LrcChecksum::compute(const QByteArray& data)
{
    quint8 lrc = 0;
    for (char c : data) lrc ^= static_cast<quint8>(c);
    lrc = (~lrc + 1) & 0xFF;

    m_stats.totalComputations++;
    m_stats.totalBytesProcessed += data.size();

    emit computed(lrc);
    return lrc;
}

bool LrcChecksum::verify(const QByteArray& data)
{
    if (data.isEmpty()) return false;
    quint8 lrc = 0;
    for (char c : data) lrc ^= static_cast<quint8>(c);
    return lrc == 0;
}

void LrcChecksum::resetStatistics() { m_stats = Stats{}; }
