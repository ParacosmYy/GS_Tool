/**
 * @file FcsChecker.cpp
 * @brief FCS帧校验序列实现
 */

#include "utils/fcs/FcsChecker.h"

FcsChecker::FcsChecker(Type type, QObject* parent)
    : QObject(parent), m_type(type) {}

quint32 FcsChecker::compute(const QByteArray& data)
{
    quint32 result = (m_type == Type::FCS16) ? computeCRC16(data) : computeCRC32(data);

    m_stats.totalComputations++;
    m_stats.totalBytesProcessed += data.size();

    emit computed(result);
    return result;
}

bool FcsChecker::verify(const QByteArray& data)
{
    quint32 fcs = compute(data);
    return fcs == 0;
}

quint32 FcsChecker::computeCRC16(const QByteArray& data)
{
    /* CRC-16/CCITT (HDLC FCS-16) */
    quint32 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) crc = (crc >> 1) ^ 0x8408;
            else crc >>= 1;
        }
    }
    return crc ^ 0xFFFF;
}

quint32 FcsChecker::computeCRC32(const QByteArray& data)
{
    /* CRC-32 (PPP FCS-32) */
    quint32 crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

void FcsChecker::resetStatistics() { m_stats = Stats{}; }
