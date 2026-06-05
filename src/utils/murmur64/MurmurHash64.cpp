/**
 * @file MurmurHash64.cpp
 * @brief MurmurHash3 64位变体实现
 */

#include "utils/murmur64/MurmurHash64.h"

#include <QElapsedTimer>
#include <QtEndian>

/* ═══════════════════════════════════════════════════════════════
 *  MurmurHash3 常量
 * ═══════════════════════════════════════════════════════════════ */
static constexpr quint64 C1 = 0x87c37b91114253d5ULL;
static constexpr quint64 C2 = 0x4cf5ad432745937fULL;

/* ═══════════════════════════════════════════════════════════════
 *  构造 / 配置
 * ═══════════════════════════════════════════════════════════════ */

/** @brief 构造函数 @param parent 父对象 */
MurmurHash64::MurmurHash64(QObject* parent)
    : QObject(parent)
    , m_seed(0)
    , m_timeSumMs(0.0)
{
}

/** @brief 设置哈希种子 @param seed 种子值 */
void MurmurHash64::setSeed(quint32 seed)
{
    m_seed = seed;
}

/* ═══════════════════════════════════════════════════════════════
 *  64位哈希
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 计算64位MurmurHash
 *
 * 基于MurmurHash3_x64_64变体，处理16字节块，尾字节特殊处理。
 *
 * @param data 输入数据
 * @return 64位哈希值
 */
quint64 MurmurHash64::hash64(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    const quint8* ptr = reinterpret_cast<const quint8*>(data.constData());
    const int nblocks = len / 16;

    quint64 h1 = m_seed;
    quint64 h2 = m_seed;

    /* 处理16字节块 */
    for (int i = 0; i < nblocks; ++i) {
        quint64 k1 = qFromLittleEndian<quint64>(ptr + i * 16);
        quint64 k2 = qFromLittleEndian<quint64>(ptr + i * 16 + 8);

        k1 *= C1; k1 = rotl64(k1, 31); k1 *= C2; h1 ^= k1;
        h1 = rotl64(h1, 27); h1 += h2; h1 = h1 * 5 + 0x52dce729;

        k2 *= C2; k2 = rotl64(k2, 33); k2 *= C1; h2 ^= k2;
        h2 = rotl64(h2, 31); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
    }

    /* 处理尾部字节 */
    const quint8* tail = ptr + nblocks * 16;
    quint64 k1 = 0;
    quint64 k2 = 0;

    switch (len & 15) {
    case 15: k2 ^= static_cast<quint64>(tail[14]) << 48; Q_FALLTHROUGH();
    case 14: k2 ^= static_cast<quint64>(tail[13]) << 40; Q_FALLTHROUGH();
    case 13: k2 ^= static_cast<quint64>(tail[12]) << 32; Q_FALLTHROUGH();
    case 12: k2 ^= static_cast<quint64>(tail[11]) << 24; Q_FALLTHROUGH();
    case 11: k2 ^= static_cast<quint64>(tail[10]) << 16; Q_FALLTHROUGH();
    case 10: k2 ^= static_cast<quint64>(tail[9])  << 8;  Q_FALLTHROUGH();
    case  9: k2 ^= static_cast<quint64>(tail[8]);
        k2 *= C2; k2 = rotl64(k2, 33); k2 *= C1; h2 ^= k2;
        Q_FALLTHROUGH();
    case  8: k1 ^= static_cast<quint64>(tail[7]) << 56; Q_FALLTHROUGH();
    case  7: k1 ^= static_cast<quint64>(tail[6]) << 48; Q_FALLTHROUGH();
    case  6: k1 ^= static_cast<quint64>(tail[5]) << 40; Q_FALLTHROUGH();
    case  5: k1 ^= static_cast<quint64>(tail[4]) << 32; Q_FALLTHROUGH();
    case  4: k1 ^= static_cast<quint64>(tail[3]) << 24; Q_FALLTHROUGH();
    case  3: k1 ^= static_cast<quint64>(tail[2]) << 16; Q_FALLTHROUGH();
    case  2: k1 ^= static_cast<quint64>(tail[1]) << 8;  Q_FALLTHROUGH();
    case  1: k1 ^= static_cast<quint64>(tail[0]);
        k1 *= C1; k1 = rotl64(k1, 31); k1 *= C2; h1 ^= k1;
        break;
    default: break;
    }

    /* 最终雪崩混合 */
    h1 ^= static_cast<quint64>(len);
    h2 ^= static_cast<quint64>(len);
    h1 += h2;
    h2 += h1;
    h1 = fmix64(h1);
    h2 = fmix64(h2);
    h1 += h2;

    /* 更新统计 */
    qint64 elapsed = timer.nsecsElapsed();
    double ms = static_cast<double>(elapsed) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return h1;
}

/* ═══════════════════════════════════════════════════════════════
 *  32位哈希
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 计算32位MurmurHash
 *
 * 基于MurmurHash3_x86_32变体，处理4字节块。
 *
 * @param data 输入数据
 * @return 32位哈希值
 */
quint32 MurmurHash64::hash32(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    const quint8* ptr = reinterpret_cast<const quint8*>(data.constData());
    const int nblocks = len / 4;
    quint32 h1 = m_seed;

    static constexpr quint32 c1_32 = 0xcc9e2d51U;
    static constexpr quint32 c2_32 = 0x1b873593U;

    /* 处理4字节块 */
    for (int i = 0; i < nblocks; ++i) {
        quint32 k1 = qFromLittleEndian<quint32>(ptr + i * 4);
        k1 *= c1_32;
        k1 = (k1 << 15) | (k1 >> 17); /* rotl32 */
        k1 *= c2_32;
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xe6546b64U;
    }

    /* 尾部 */
    const quint8* tail = ptr + nblocks * 4;
    quint32 k1 = 0;
    switch (len & 3) {
    case 3: k1 ^= static_cast<quint32>(tail[2]) << 16; Q_FALLTHROUGH();
    case 2: k1 ^= static_cast<quint32>(tail[1]) << 8;  Q_FALLTHROUGH();
    case 1: k1 ^= static_cast<quint32>(tail[0]);
        k1 *= c1_32;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2_32;
        h1 ^= k1;
        break;
    default: break;
    }

    /* 最终雪崩 */
    h1 ^= static_cast<quint32>(len);
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6bU;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35U;
    h1 ^= h1 >> 16;

    /* 更新统计 */
    qint64 elapsed32 = timer.nsecsElapsed();
    double ms32 = static_cast<double>(elapsed32) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms32;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return h1;
}

/* ═══════════════════════════════════════════════════════════════
 *  内部函数
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 64位最终混合 (雪崩)
 * @param k 输入值
 * @return 充分混合后的值
 */
quint64 MurmurHash64::fmix64(quint64 k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

/**
 * @brief 64位循环左移
 * @param x 输入值
 * @param r 左移位数
 * @return 旋转结果
 */
quint64 MurmurHash64::rotl64(quint64 x, qint8 r)
{
    return (x << static_cast<int>(r)) | (x >> (64 - static_cast<int>(r)));
}

/** @brief 重置所有统计计数器 */
void MurmurHash64::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
