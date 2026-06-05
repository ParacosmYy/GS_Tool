/**
 * @file Crc64Engine.cpp
 * @brief CRC-64引擎实现
 */

#include "utils/crc64b/Crc64Engine.h"
#include <QElapsedTimer>

Crc64Engine::Crc64Engine(QObject* parent)
    : QObject(parent), m_polynomial(Polynomial::ECMA_182),
      m_runningCrc(0), m_streaming(false), m_timeSum(0.0)
{
    buildTable();
}

void Crc64Engine::setPolynomial(Polynomial poly)
{
    m_polynomial = poly;
    buildTable();
}

void Crc64Engine::buildTable()
{
    quint64 poly = 0;
    switch (m_polynomial) {
    case Polynomial::ECMA_182:
        poly = 0x42F0E1EBA9EA3693ULL;
        break;
    case Polynomial::WE:
        poly = 0x42F0E1EBA9EA3693ULL; // Same polynomial, different init
        break;
    case Polynomial::ISO:
        poly = 0x000000000000001BULL;
        break;
    }

    for (int i = 0; i < 256; ++i) {
        quint64 crc = static_cast<quint64>(i) << 56;
        for (int j = 0; j < 8; ++j) {
            if (crc & (1ULL << 63)) crc = (crc << 1) ^ poly;
            else crc <<= 1;
        }
        m_table[i] = crc;
    }
}

quint64 Crc64Engine::compute(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    quint64 crc = 0xFFFFFFFFFFFFFFFFULL;
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        crc = (crc << 8) ^ m_table[((crc >> 56) ^ byte) & 0xFF];
    }
    crc ^= 0xFFFFFFFFFFFFFFFFULL;

    double elapsed = timer.elapsed();
    ++m_stats.totalChecksums;
    m_stats.totalBytesProcessed += data.size();
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalChecksums;

    emit checksumComputed(crc, data.size());
    return crc;
}

void Crc64Engine::begin()
{
    m_runningCrc = 0xFFFFFFFFFFFFFFFFULL;
    m_streaming = true;
}

void Crc64Engine::update(const QByteArray& data)
{
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        m_runningCrc = (m_runningCrc << 8) ^ m_table[((m_runningCrc >> 56) ^ byte) & 0xFF];
    }
}

quint64 Crc64Engine::end()
{
    m_streaming = false;
    m_runningCrc ^= 0xFFFFFFFFFFFFFFFFULL;
    return m_runningCrc;
}

void Crc64Engine::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
