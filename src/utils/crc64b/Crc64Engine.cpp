/**
 * @file Crc64Engine.cpp
 * @brief CRC-64校验引擎实现 — ECMA-182 / WE 双多项式 + 查找表加速
 */

#include "Crc64Engine.h"

#include <QElapsedTimer>

// ═══════════════════════════════════════════════════════════
// 常量定义
// ═══════════════════════════════════════════════════════════

/** @brief CRC-64/ECMA-182 和 CRC-64/WE 共享多项式 */
static constexpr quint64 POLY_CRC64 = 0x42F0E1EBA9EA3693ULL;

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

Crc64Engine::Crc64Engine(Polynomial poly, QObject* parent)
    : QObject(parent)
    , m_poly(poly)
    , m_polynomial(POLY_CRC64)
{
    buildLookupTable();
    reset();
}

Crc64Engine::~Crc64Engine() = default;

// ═══════════════════════════════════════════════════════════
// 流式接口
// ═══════════════════════════════════════════════════════════

void Crc64Engine::reset()
{
    if (m_poly == Polynomial::ECMA182) {
        m_crc = 0x0000000000000000ULL;
    } else {
        m_crc = 0xFFFFFFFFFFFFFFFFULL;
    }
    m_finalized = false;
}

void Crc64Engine::update(const QByteArray& data)
{
    const int size = data.size();
    const char* ptr = data.constData();

    for (int i = 0; i < size; ++i) {
        const quint8 index = static_cast<quint8>(
            (m_crc >> 56) ^ static_cast<quint8>(ptr[i]));
        m_crc = (m_crc << 8) ^ m_table[index];
    }

    m_stats.totalBytes += static_cast<quint64>(size);
}

void Crc64Engine::update(quint8 byte)
{
    const quint8 index = static_cast<quint8>(
        (m_crc >> 56) ^ byte);
    m_crc = (m_crc << 8) ^ m_table[index];
    m_stats.totalBytes += 1;
}

quint64 Crc64Engine::finalValue()
{
    if (m_poly == Polynomial::WE) {
        m_crc ^= 0xFFFFFFFFFFFFFFFFULL;
    }
    m_finalized = true;
    return m_crc;
}

// ═══════════════════════════════════════════════════════════
// 便捷接口
// ═══════════════════════════════════════════════════════════

quint64 Crc64Engine::checksum(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    reset();
    update(data);
    const quint64 result = finalValue();

    m_stats.totalChecksums += 1;
    const qint64 elapsed = timer.nsecsElapsed() / 1000;
    const auto n = m_stats.totalChecksums;
    if (n == 1) {
        m_stats.avgTime = static_cast<double>(elapsed);
    } else {
        m_stats.avgTime =
            m_stats.avgTime * static_cast<double>(n - 1) /
                static_cast<double>(n) +
            static_cast<double>(elapsed) / static_cast<double>(n);
    }

    emit checksumReady(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

Crc64Engine::Polynomial Crc64Engine::polynomial() const
{
    return m_poly;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

Crc64Engine::Stats Crc64Engine::stats() const
{
    return m_stats;
}

void Crc64Engine::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

void Crc64Engine::buildLookupTable()
{
    for (quint32 i = 0; i < 256; ++i) {
        quint64 crc = static_cast<quint64>(i) << 56;

        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000000000000000ULL) {
                crc = (crc << 1) ^ m_polynomial;
            } else {
                crc <<= 1;
            }
        }

        m_table[i] = crc;
    }
}
