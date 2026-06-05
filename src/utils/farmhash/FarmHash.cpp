/**
 * @file FarmHash.cpp
 * @brief FarmHash指纹实现 — Google快速哈希算法
 */

#include "utils/farmhash/FarmHash.h"

#include <QElapsedTimer>
#include <QtEndian>

/* ═══════════════════════════════════════════════════════════════
 *  CityHash/FarmHash 常量
 * ═══════════════════════════════════════════════════════════════ */
static constexpr quint64 K0 = 0xc3a5c85c97b5ad56ULL;
static constexpr quint64 K1 = 0xb492b66fbe98f273ULL;
static constexpr quint64 K2 = 0x9ae16a3b2f90404fULL;
static constexpr quint64 K3 = 0xc949d7c7509e6557ULL;

/* ═══════════════════════════════════════════════════════════════
 *  构造
 * ═══════════════════════════════════════════════════════════════ */

/** @brief 构造函数 @param parent 父对象 */
FarmHash::FarmHash(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/* ═══════════════════════════════════════════════════════════════
 *  内部辅助
 * ═══════════════════════════════════════════════════════════════ */

quint64 FarmHash::rotl64(quint64 x, qint8 r)
{
    return (x << static_cast<int>(r)) | (x >> (64 - static_cast<int>(r)));
}

/** @brief 将128位哈希压缩为64位 (CityHash风格) */
quint64 FarmHash::hash128to64(const Hash128& x)
{
    /* Murmur-like finalizer */
    quint64 a = (x.first ^ x.second) * K3;
    a ^= (a >> 47);
    quint64 b = (x.second ^ a) * K3;
    b ^= (b >> 47);
    return b;
}

/** @brief 弱哈希混合 (用于种子生成) */
quint64 FarmHash::weakHash32Seeds(quint64 a, quint64 b, quint64 c)
{
    a += b; a = rotl64(a, 21); a *= K2;
    b += c; b = rotl64(b, 17); b *= K3;
    c += a; c = rotl64(c, 37); c *= K1;
    a += b; a = rotl64(a, 13); a *= K2;
    return a ^ b ^ c;
}

/**
 * @brief 中等长度数据哈希 (17~64字节)
 * 使用类似于CityHash的逐步累积策略
 */
quint64 FarmHash::hashMedium(const char* data, int len) const
{
    quint64 x = qFromLittleEndian<quint64>(
        reinterpret_cast<const quint8*>(data));
    quint64 y = qFromLittleEndian<quint64>(
        reinterpret_cast<const quint8*>(data + len - 8));

    x *= K2; x = rotl64(x, 31); x *= K3;
    y = rotl64(y, 17); y *= K1;
    quint64 z = rotl64(x ^ y, 47);

    if (len > 32) {
        quint64 w = qFromLittleEndian<quint64>(
            reinterpret_cast<const quint8*>(data + 16));
        quint64 v = qFromLittleEndian<quint64>(
            reinterpret_cast<const quint8*>(data + len - 24));
        w *= K2; w = rotl64(w, 31); w *= K3;
        z += w;
        v = rotl64(v, 37); v *= K1;
        z ^= v;
        z = rotl64(z, 25);
    }

    /* 累积中间块 */
    quint64 seed = static_cast<quint64>(len) * K2;
    seed ^= rotl64(x, 23) + rotl64(y, 37);
    return hash128to64({z, seed});
}

/**
 * @brief 长数据哈希 (>64字节)
 * 分段处理，每段64字节
 */
quint64 FarmHash::hashLong(const char* data, int len) const
{
    /* 初始化种子 */
    quint64 x = qFromLittleEndian<quint64>(
        reinterpret_cast<const quint8*>(data));
    quint64 y = qFromLittleEndian<quint64>(
        reinterpret_cast<const quint8*>(data + len - 16));
    quint64 z = qFromLittleEndian<quint64>(
        reinterpret_cast<const quint8*>(data + len - 56));

    quint64 w = x * K2 + y * K3 + z;
    w = rotl64(w, 17); w *= K1;

    /* 尾部累积 */
    quint64 v = qFromLittleEndian<quint64>(
        reinterpret_cast<const quint8*>(data + len - 32));
    v = rotl64(v, 31); v *= K1;

    quint64 a = qFromLittleEndian<quint64>(
        reinterpret_cast<const quint8*>(data + len - 8));
    a = rotl64(a, 47); a *= K2;

    quint64 seed = (w + v) ^ a;
    seed = hash128to64({seed, z});

    return hash128to64({seed, static_cast<quint64>(len) * K2});
}

/**
 * @brief 短数据32位哈希 (<=24字节)
 */
quint32 FarmHash::hash32Len0to24(const char* data, int len)
{
    if (len >= 8) {
        quint64 a = qFromLittleEndian<quint64>(
            reinterpret_cast<const quint8*>(data));
        quint64 b = qFromLittleEndian<quint64>(
            reinterpret_cast<const quint8*>(data + len - 8));
        quint64 c = a ^ rotl64(b, 37);
        c *= K2;
        c = rotl64(c, 25);
        return static_cast<quint32>(
            (c ^ rotl64(a, 17)) >> 32);
    }
    if (len >= 4) {
        quint32 a = qFromLittleEndian<quint32>(
            reinterpret_cast<const quint8*>(data));
        quint32 b = qFromLittleEndian<quint32>(
            reinterpret_cast<const quint8*>(data + len - 4));
        return (a * K3 + b) >> 32;
    }
    if (len > 0) {
        quint8 a = static_cast<quint8>(data[0]);
        quint8 b = static_cast<quint8>(data[len >> 1]);
        quint8 c = static_cast<quint8>(data[len - 1]);
        quint32 y = static_cast<quint32>(a) + (static_cast<quint32>(b) << 8);
        quint32 z = static_cast<quint32>(len) + (static_cast<quint32>(c) << 2);
        return (y * K3 ^ z * K1) >> 32;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 *  公开接口
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 计算64位FarmHash指纹
 * @param data 输入数据
 * @return 64位哈希值
 */
quint64 FarmHash::hash64(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    const char* ptr = data.constData();
    quint64 result = 0;

    if (len <= 16) {
        /* 极短数据: 使用简单的乘法混合 */
        if (len >= 8) {
            quint64 a = qFromLittleEndian<quint64>(
                reinterpret_cast<const quint8*>(ptr));
            quint64 b = qFromLittleEndian<quint64>(
                reinterpret_cast<const quint8*>(ptr + len - 8));
            result = hash128to64({a, rotl64(b, 37) + K2});
        } else if (len >= 4) {
            quint32 a = qFromLittleEndian<quint32>(
                reinterpret_cast<const quint8*>(ptr));
            quint32 b = qFromLittleEndian<quint32>(
                reinterpret_cast<const quint8*>(ptr + len - 4));
            result = (static_cast<quint64>(a) * K2)
                   ^ (static_cast<quint64>(b) * K3);
            result = rotl64(result, 23);
        } else if (len > 0) {
            quint8 a = static_cast<quint8>(ptr[0]);
            quint8 b = static_cast<quint8>(ptr[len >> 1]);
            quint8 c = static_cast<quint8>(ptr[len - 1]);
            quint64 y = a + (static_cast<quint64>(b) << 8);
            quint64 z = len + (static_cast<quint64>(c) << 2);
            result = (y * K2) ^ (z * K3);
        }
    } else if (len <= 64) {
        result = hashMedium(ptr, len);
    } else {
        result = hashLong(ptr, len);
    }

    /* 更新统计 */
    qint64 elapsed64 = timer.nsecsElapsed();
    double ms64 = static_cast<double>(elapsed64) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms64;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return result;
}

/**
 * @brief 计算32位FarmHash指纹
 * @param data 输入数据
 * @return 32位哈希值
 */
quint32 FarmHash::hash32(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    quint32 result = hash32Len0to24(data.constData(), len);

    /* 更新统计 */
    qint64 elapsed32 = timer.nsecsElapsed();
    double ms32 = static_cast<double>(elapsed32) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms32;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return result;
}

/**
 * @brief 计算128位FarmHash指纹 (不依赖hash64，独立实现避免双重计数)
 *
 * 策略: 使用与hash64相同的分段逻辑，直接计算两个独立的64位值
 *
 * @param data 输入数据
 * @return 128位哈希值 (QPair<高位, 低位>)
 */
FarmHash::Hash128 FarmHash::hash128(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    const char* ptr = data.constData();
    Hash128 result;

    if (len <= 64) {
        /* 低位: 正向哈希 */
        if (len <= 16) {
            if (len >= 8) {
                quint64 a = qFromLittleEndian<quint64>(
                    reinterpret_cast<const quint8*>(ptr));
                quint64 b = qFromLittleEndian<quint64>(
                    reinterpret_cast<const quint8*>(ptr + len - 8));
                result.second = hash128to64({a, rotl64(b, 37) + K2});
            } else {
                result.second = K0 ^ static_cast<quint64>(len);
            }
        } else if (len <= 64) {
            result.second = hashMedium(ptr, len);
        } else {
            result.second = hashLong(ptr, len);
        }
        /* 高位: 使用不同种子做偏移哈希 */
        QByteArray shifted = data;
        shifted.prepend('\x5a');
        if (len <= 16) {
            if (len >= 8) {
                quint64 a = qFromLittleEndian<quint64>(
                    reinterpret_cast<const quint8*>(ptr));
                quint64 b = qFromLittleEndian<quint64>(
                    reinterpret_cast<const quint8*>(ptr + len - 8));
                result.first = hash128to64({rotl64(a, 13), b + K3});
            } else {
                result.first = K1 ^ rotl64(result.second, 17);
            }
        } else {
            result.first = hash128to64({result.second, K2 + static_cast<quint64>(len)});
        }
    } else {
        /* 长数据: 前半段和后半段分别哈希 */
        int mid = len / 2;
        result.second = hashLong(ptr, mid);
        result.first  = hashLong(ptr + mid, len - mid);
    }

    /* 更新统计 (只计一次) */
    qint64 elapsed128 = timer.nsecsElapsed();
    double ms128 = static_cast<double>(elapsed128) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms128;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return result;
}

/** @brief 重置所有统计计数器 */
void FarmHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
