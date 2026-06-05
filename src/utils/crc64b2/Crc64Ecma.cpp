/**
 * @file Crc64Ecma.cpp
 * @brief CRC-64/ECMA-182校验引擎实现 — 查找表加速
 */

#include "Crc64Ecma.h"

#include <QElapsedTimer>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

Crc64Ecma::Crc64Ecma(QObject* parent)
    : QObject(parent)
{
    buildTable();
    reset();
}

Crc64Ecma::~Crc64Ecma() = default;

// ═══════════════════════════════════════════════════════════
// 核心接口
// ═══════════════════════════════════════════════════════════

quint64 Crc64Ecma::compute(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    reset();

    const int size = data.size();
    const char* ptr = data.constData();
    for (int i = 0; i < size; ++i) {
        const quint8 index = static_cast<quint8>(
            (m_crc >> 56) ^ static_cast<quint8>(ptr[i]));
        m_crc = (m_crc << 8) ^ m_table[index];
    }

    const quint64 result = m_crc;

    /* 更新统计 */
    m_stats.totalComputations++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalComputations;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit computationCompleted(result);
    return result;
}

void Crc64Ecma::update(quint8 byte)
{
    const quint8 index = static_cast<quint8>((m_crc >> 56) ^ byte);
    m_crc = (m_crc << 8) ^ m_table[index];
}

void Crc64Ecma::reset()
{
    m_crc = 0x0000000000000000ULL;
}

quint64 Crc64Ecma::currentValue() const
{
    return m_crc;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

Crc64Ecma::Stats Crc64Ecma::stats() const
{
    return m_stats;
}

void Crc64Ecma::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

void Crc64Ecma::buildTable()
{
    for (quint32 i = 0; i < 256; ++i) {
        quint64 crc = static_cast<quint64>(i) << 56;

        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000000000000000ULL) {
                crc = (crc << 1) ^ POLY;
            } else {
                crc <<= 1;
            }
        }

        m_table[i] = crc;
    }
}
