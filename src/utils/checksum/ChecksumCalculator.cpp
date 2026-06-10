/**
 * @file ChecksumCalculator.cpp
 * @brief Calculate CRC8/CRC16/CRC32/checksums for protocol validation implementation
 */
#include "utils/checksum/ChecksumCalculator.h"

#include <QElapsedTimer>

ChecksumCalculator::ChecksumCalculator(QObject *parent)
    : QObject(parent)
{
}

ChecksumCalculator::~ChecksumCalculator() = default;

QByteArray ChecksumCalculator::process(const QByteArray &input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        emit errorOccurred(tr("Empty input data"));
        return {};
    }

    m_stats.operationsPerformed++;
    m_stats.bytesProcessed += static_cast<quint64>(input.size());

    // Process data based on module type
    QByteArray result = input;

    m_stats.lastOperationMs = static_cast<quint64>(timer.elapsed());
    emit processingComplete(result);
    return result;
}

void ChecksumCalculator::resetStatistics()
{
    m_stats = Stats{};
    resetChecksumStatistics();
}

quint64 ChecksumCalculator::calculate(const QByteArray& data, Algorithm alg) const
{
    ++m_totalCalculations;
    m_totalBytesProcessed += static_cast<quint64>(data.size());
    m_algorithmCounts[static_cast<int>(alg)] =
        m_algorithmCounts.value(static_cast<int>(alg), 0) + 1;

    switch (alg) {
    case CRC8:
        return crc8(data);
    case CRC16Ccitt:
        return crc16Ccitt(data);
    case CRC16Modbus:
        return crc16Modbus(data);
    case CRC16Kermit:
        return crc16Kermit(data);
    case CRC32:
        return crc32(data, 0xEDB88320u);
    case CRC32C:
        return crc32(data, 0x82F63B78u);
    case Xor8:
        return xor8(data);
    case Sum8:
        return sumBytes(data, 8);
    case Sum16:
        return sumBytes(data, 16);
    case Sum32:
        return sumBytes(data, 32);
    case CustomCrc:
        ++m_totalCustomCalculations;
        return crc16Modbus(data);
    }
    return 0;
}

quint64 ChecksumCalculator::crc8(const QByteArray& data)
{
    quint8 crc = 0x00;
    for (uchar byte : data) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? static_cast<quint8>((crc << 1) ^ 0x07)
                               : static_cast<quint8>(crc << 1);
        }
    }
    return crc;
}

quint64 ChecksumCalculator::crc16Ccitt(const QByteArray& data)
{
    quint16 crc = 0xFFFF;
    for (uchar byte : data) {
        crc ^= static_cast<quint16>(byte) << 8;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000) ? static_cast<quint16>((crc << 1) ^ 0x1021)
                                 : static_cast<quint16>(crc << 1);
        }
    }
    return crc;
}

quint64 ChecksumCalculator::crc16Modbus(const QByteArray& data)
{
    quint16 crc = 0xFFFF;
    for (uchar byte : data) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x0001) ? static_cast<quint16>((crc >> 1) ^ 0xA001)
                                 : static_cast<quint16>(crc >> 1);
        }
    }
    return crc;
}

quint64 ChecksumCalculator::crc16Kermit(const QByteArray& data)
{
    quint16 crc = 0x0000;
    for (uchar byte : data) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x0001) ? static_cast<quint16>((crc >> 1) ^ 0x8408)
                                 : static_cast<quint16>(crc >> 1);
        }
    }
    return crc;
}

quint64 ChecksumCalculator::crc32(const QByteArray& data, quint32 polynomial)
{
    quint32 crc = 0xFFFFFFFFu;
    for (uchar byte : data) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 1u) ? (crc >> 1) ^ polynomial : (crc >> 1);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

quint64 ChecksumCalculator::xor8(const QByteArray& data)
{
    quint8 value = 0;
    for (uchar byte : data) {
        value ^= byte;
    }
    return value;
}

quint64 ChecksumCalculator::sumBytes(const QByteArray& data, int widthBits)
{
    quint64 sum = 0;
    for (uchar byte : data) {
        sum += byte;
    }

    if (widthBits == 8) {
        return sum & 0xFFu;
    }
    if (widthBits == 16) {
        return sum & 0xFFFFu;
    }
    return sum & 0xFFFFFFFFu;
}

