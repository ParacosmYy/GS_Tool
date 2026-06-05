/**
 * @file WyHash.cpp
 * @brief WyHash 快速哈希函数实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/wyhash/WyHash.h"

/** @brief 构造函数 @param parent 父对象 */
WyHash::WyHash(QObject* parent)
    : QObject(parent)
    , m_seed(0)
    , m_timeSum(0.0)
{
}

/** @brief 设置哈希种子 @param seed 64位种子值 */
void WyHash::setSeed(quint64 seed)
{
    m_seed = seed;
}

/** @brief 64位无符号乘法高位混合 */
inline quint64 WyHash::wymix(quint64 a, quint64 b)
{
    /* 使用128位乘法的高64位和低64位进行混合 */
    /* 在64位平台上利用 __uint128_t 或手动拆分 */
    quint64 ha = a >> 32, la = a & 0xFFFFFFFFULL;
    quint64 hb = b >> 32, lb = b & 0xFFFFFFFFULL;
    quint64 rh = ha * hb;
    quint64 rm0 = ha * lb;
    quint64 rm1 = la * hb;
    quint64 rl = la * lb;
    quint64 t = rl + (rm0 << 32);
    quint64 carry = (t < rl) ? 1ULL : 0ULL;
    quint64 lo = t + (rm1 << 32);
    carry += (lo < t) ? 1ULL : 0ULL;
    quint64 hi = rh + (rm0 >> 32) + (rm1 >> 32) + carry;
    return hi ^ lo;
}

/** @brief 从字节数组读取小端64位整数 */
quint64 WyHash::readLE64(const QByteArray& data, int offset)
{
    quint64 val = 0;
    int end = qMin(offset + 8, data.size());
    for (int i = offset; i < end; ++i) {
        val |= static_cast<quint64>(static_cast<quint8>(data[i]))
               << (8 * (i - offset));
    }
    return val;
}

/** @brief 从字节数组读取小端32位整数 */
quint32 WyHash::readLE32(const QByteArray& data, int offset)
{
    quint32 val = 0;
    int end = qMin(offset + 4, data.size());
    for (int i = offset; i < end; ++i) {
        val |= static_cast<quint32>(static_cast<quint8>(data[i]))
               << (8 * (i - offset));
    }
    return val;
}

/** @brief wyrand 伪随机数生成 */
quint64 WyHash::wyrand(quint64& seed)
{
    seed += 0x60bee2bee120fc15ULL;
    return wymix(seed, seed ^ 0x60bee2bee120fc15ULL);
}

/** @brief 计算哈希值(WyHash 最终版) @param data 输入数据 @return 64位哈希值 */
quint64 WyHash::hash(const QByteArray& data)
{
    m_timer.start();

    int len = data.size();
    quint64 seed = m_seed;
    quint64 a, b;

    seed ^= wymix(seed ^ 0x60bee2bee120fc15ULL, seed);

    if (len <= 16) {
        /* 短输入路径 (≤16 字节) */
        a = (len <= 8) ? readLE64(data, 0)
                        : readLE64(data, 0);
        b = (len <= 8) ? (static_cast<quint64>(len) << 56)
                          | (static_cast<quint64>(readLE32(data, 0)) << 16)
                        : readLE64(data, 8);

        if (len >= 4 && len <= 8) {
            a = (static_cast<quint64>(readLE32(data, 0)) << 16)
                | (len <= 4 ? static_cast<quint64>(len)
                            : static_cast<quint64>(readLE32(data, len - 4)));
            a <<= 16;
            a |= readLE32(data, (len >> 1) & 3);
        }

        quint64 result = wymix(a ^ 0x60bee2bee120fc15ULL, b ^ seed);
        result = wymix(result ^ len, seed ^ 0x60bee2bee120fc15ULL);

        updateStats(len);
        return result;
    }

    /* 长输入路径: 处理 48 字节步长 */
    quint64 see1 = seed, see2 = seed;
    int i = 0;
    for (; i + 48 <= len; i += 48) {
        seed = wymix(readLE64(data, i) ^ seed,
                      readLE64(data, i + 8) ^ see1);
        see1 = wymix(readLE64(data, i + 16) ^ see1,
                      readLE64(data, i + 24) ^ see2);
        see2 = wymix(readLE64(data, i + 32) ^ see2,
                      readLE64(data, i + 40) ^ seed);
    }

    /* 剩余数据(16~47 字节) */
    seed ^= see1;
    see2 += see1;

    int remaining = len - i;
    if (remaining <= 16) {
        a = (remaining >= 8) ? readLE64(data, i)
                             : static_cast<quint64>(remaining);
        b = (remaining >= 8) ? readLE64(data, len - 8)
                             : static_cast<quint64>(remaining);
    } else {
        a = readLE64(data, i);
        b = readLE64(data, i + 8);
        seed = wymix(a ^ seed, b ^ see2);
        a = (remaining >= 24) ? readLE64(data, i + 16) : a;
        b = (remaining >= 24) ? readLE64(data, len - 8) : b;
    }

    quint64 result = wymix(a ^ seed, b ^ see2);
    result = wymix(result ^ len, 0x60bee2bee120fc15ULL);

    updateStats(len);
    return result;
}

/** @brief 计算哈希值(64位变体) @param data 输入数据 @return 64位哈希值 */
quint64 WyHash::hash64(const QByteArray& data)
{
    m_timer.start();

    int len = data.size();
    quint64 s = m_seed;

    /* 使用 wyrand 生成初始种子 */
    s = wyrand(s);

    if (len <= 16) {
        quint64 a = (len >= 8) ? readLE64(data, 0)
                               : static_cast<quint64>(len);
        quint64 b = (len >= 8) ? readLE64(data, len - 8)
                               : static_cast<quint64>(len);
        quint64 result = wymix(a ^ s, b);

        ++m_stats.totalHashes;
        m_stats.totalBytesProcessed += static_cast<quint64>(len);
        qint64 elapsed = m_timer.elapsed();
        m_timeSum += static_cast<double>(elapsed);
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalHashes);
        emit hashComputed(static_cast<qint64>(len));
        return result;
    }

    quint64 s1 = s, s2 = s;
    int i = 0;
    for (; i + 48 <= len; i += 48) {
        s = wymix(readLE64(data, i) ^ s, readLE64(data, i + 8) ^ s1);
        s1 = wymix(readLE64(data, i + 16) ^ s1, readLE64(data, i + 24) ^ s2);
        s2 = wymix(readLE64(data, i + 32) ^ s2, readLE64(data, i + 40) ^ s);
    }

    s ^= s1;
    s2 += s1;
    int rem = len - i;
    quint64 a = (rem >= 8) ? readLE64(data, i) : static_cast<quint64>(rem);
    quint64 b = (rem >= 8) ? readLE64(data, len - 8) : static_cast<quint64>(rem);
    quint64 result = wymix(a ^ s, b ^ s2) ^ len;

    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    qint64 elapsed = m_timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalHashes);

    emit hashComputed(static_cast<qint64>(len));
    return result;
}

/** @brief 更新统计信息(内部辅助) @param len 输入长度 */
void WyHash::updateStats(int len)
{
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    qint64 elapsed = m_timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalHashes);
    emit hashComputed(static_cast<qint64>(len));
}

/** @brief 重置所有统计计数器 */
void WyHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
