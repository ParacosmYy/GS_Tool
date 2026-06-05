/**
 * @file MetroHash.cpp
 * @brief MetroHash实现 — 高性能64位整数操作哈希
 */

#include "utils/metro/MetroHash.h"

#include <QElapsedTimer>
#include <QtEndian>

/* ═══════════════════════════════════════════════════════════════
 *  MetroHash 常量
 * ═══════════════════════════════════════════════════════════════ */
static constexpr quint64 K0 = 0xD6D018F5ULL;
static constexpr quint64 K1 = 0xA2AA033BULL;
static constexpr quint64 K2 = 0x62992FC1ULL;
static constexpr quint64 K3 = 0x30BC5B29ULL;

/* ═══════════════════════════════════════════════════════════════
 *  构造 / 配置
 * ═══════════════════════════════════════════════════════════════ */

/** @brief 构造函数 @param parent 父对象 */
MetroHash::MetroHash(QObject* parent)
    : QObject(parent)
    , m_seed(0)
    , m_timeSumMs(0.0)
{
}

/** @brief 设置哈希种子 @param seed 64位种子值 */
void MetroHash::setSeed(quint64 seed)
{
    m_seed = seed;
}

/* ═══════════════════════════════════════════════════════════════
 *  内部函数
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief MetroHash内部轮函数 — 乘法+旋转+混合
 * @param v 输入值
 * @return 混合后的值
 */
quint64 MetroHash::metroRound(quint64 v)
{
    v *= K0;
    v = (v << 31) | (v >> 33); /* rotl64(v, 31) */
    v *= K1;
    return v;
}

/**
 * @brief 从字节数组读取64位小端值
 * @param p 数据指针(至少8字节)
 * @return 64位值
 */
quint64 MetroHash::readU64(const char* p)
{
    return qFromLittleEndian<quint64>(reinterpret_cast<const quint8*>(p));
}

/**
 * @brief 从字节数组读取32位小端值
 * @param p 数据指针(至少4字节)
 * @return 32位值
 */
quint32 MetroHash::readU32(const char* p)
{
    return qFromLittleEndian<quint32>(reinterpret_cast<const quint8*>(p));
}

/* ═══════════════════════════════════════════════════════════════
 *  64位哈希
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 计算64位MetroHash
 *
 * 分块策略: 32字节块 -> 16字节块 -> 尾部字节 -> 最终混合
 *
 * @param data 输入数据
 * @return 64位哈希值
 */
quint64 MetroHash::hash64(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    const char* ptr = data.constData();
    const quint64 seed = m_seed;

    /* 初始化状态 */
    quint64 h = static_cast<quint64>(len) * K1 + seed;
    quint64 v1 = seed;
    quint64 v2 = seed;
    quint64 v3 = seed;
    quint64 v4 = seed;

    /* 32字节块处理 */
    int offset = 0;
    while (len - offset >= 32) {
        v1 = metroRound(v1 + readU64(ptr + offset));
        v2 = metroRound(v2 + readU64(ptr + offset + 8));
        v3 = metroRound(v3 + readU64(ptr + offset + 16));
        v4 = metroRound(v4 + readU64(ptr + offset + 24));
        offset += 32;
    }

    /* 累积块结果 */
    h += v1; h = metroRound(h);
    h += v2; h = metroRound(h);
    h += v3; h = metroRound(h);
    h += v4; h = metroRound(h);

    /* 16字节块 */
    while (len - offset >= 16) {
        h = metroRound(h + readU64(ptr + offset));
        h = metroRound(h + readU64(ptr + offset + 8));
        offset += 16;
    }

    /* 8字节块 */
    if (len - offset >= 8) {
        h = metroRound(h + readU64(ptr + offset));
        offset += 8;
    }

    /* 尾部字节处理 */
    switch (len - offset) {
    case 7: h += static_cast<quint64>(static_cast<quint8>(ptr[offset + 6])) << 48;
        Q_FALLTHROUGH();
    case 6: h += static_cast<quint64>(static_cast<quint8>(ptr[offset + 5])) << 40;
        Q_FALLTHROUGH();
    case 5: h += static_cast<quint64>(static_cast<quint8>(ptr[offset + 4])) << 32;
        Q_FALLTHROUGH();
    case 4:
        h += static_cast<quint64>(readU32(ptr + offset));
        h = metroRound(h);
        break;
    case 3: h += static_cast<quint64>(static_cast<quint8>(ptr[offset + 2])) << 16;
        Q_FALLTHROUGH();
    case 2:
        h += static_cast<quint64>(static_cast<quint8>(ptr[offset]))
           + (static_cast<quint64>(static_cast<quint8>(ptr[offset + 1])) << 8);
        h = metroRound(h);
        break;
    case 1:
        h += static_cast<quint64>(static_cast<quint8>(ptr[offset]));
        h = metroRound(h);
        break;
    default: break;
    }

    /* 最终雪崩混合 */
    h ^= h >> 27;
    h *= K2;
    h ^= h >> 33;

    /* 更新统计 */
    qint64 elapsed = timer.nsecsElapsed();
    double ms = static_cast<double>(elapsed) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return h;
}

/* ═══════════════════════════════════════════════════════════════
 *  32位哈希
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 计算32位MetroHash
 *
 * 使用类似64位算法但输出截断到32位。
 *
 * @param data 输入数据
 * @return 32位哈希值
 */
quint32 MetroHash::hash32(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    const char* ptr = data.constData();
    const quint32 seed32 = static_cast<quint32>(m_seed);

    quint32 h = static_cast<quint32>(len) * static_cast<quint32>(K0) + seed32;

    int offset = 0;
    /* 16字节块 */
    while (len - offset >= 16) {
        quint32 v1 = readU32(ptr + offset);
        quint32 v2 = readU32(ptr + offset + 4);
        quint32 v3 = readU32(ptr + offset + 8);
        quint32 v4 = readU32(ptr + offset + 12);
        h += v1; h = (h << 19) | (h >> 13); h ^= v2;
        h += v3; h = (h << 17) | (h >> 15); h ^= v4;
        offset += 16;
    }

    /* 4字节块 */
    while (len - offset >= 4) {
        h += readU32(ptr + offset);
        h = (h << 15) | (h >> 17);
        h *= static_cast<quint32>(K1);
        offset += 4;
    }

    /* 尾部字节 */
    switch (len - offset) {
    case 3: h += static_cast<quint32>(static_cast<quint8>(ptr[offset + 2])) << 16;
        Q_FALLTHROUGH();
    case 2: h += static_cast<quint32>(static_cast<quint8>(ptr[offset + 1])) << 8;
        Q_FALLTHROUGH();
    case 1: h += static_cast<quint32>(static_cast<quint8>(ptr[offset]));
        h = (h << 11) | (h >> 21);
        h *= static_cast<quint32>(K0);
        break;
    default: break;
    }

    /* 最终混合 */
    h ^= h >> 16;
    h *= static_cast<quint32>(K2);
    h ^= h >> 16;

    /* 更新统计 */
    qint64 elapsed = timer.nsecsElapsed();
    double ms = static_cast<double>(elapsed) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return h;
}

/** @brief 重置所有统计计数器 */
void MetroHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
