/**
 * @file SipHash.cpp
 * @brief SipHash-2-4 快速 MAC/哈希函数实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/siphash/SipHash.h"

/** @brief 构造函数 @param parent 父对象 */
SipHash::SipHash(QObject* parent)
    : QObject(parent)
    , m_k0(0)
    , m_k1(0)
    , m_timeSum(0.0)
{
}

/** @brief 设置密钥 @param k0 密钥低64位 @param k1 密钥高64位 */
void SipHash::setKey(quint64 k0, quint64 k1)
{
    m_k0 = k0;
    m_k1 = k1;
}

/** @brief 左旋 @param x 值 @param b 位数 */
constexpr quint64 SipHash::rotl(quint64 x, int b)
{
    return (x << b) | (x >> (64 - b));
}

/** @brief SipRound 内联函数(压缩轮) */
inline void SipHash::sipRound(quint64& v0, quint64& v1,
                              quint64& v2, quint64& v3)
{
    v0 += v1; v1 = rotl(v1, 13); v1 ^= v0; v0 = rotl(v0, 32);
    v2 += v3; v3 = rotl(v3, 16); v3 ^= v2;
    v0 += v3; v3 = rotl(v3, 21); v3 ^= v0;
    v2 += v1; v1 = rotl(v1, 17); v1 ^= v2; v2 = rotl(v2, 32);
}

/** @brief 从字节串读取小端64位整数 */
quint64 SipHash::readLE64(const QByteArray& data, int offset)
{
    quint64 val = 0;
    int end = qMin(offset + 8, data.size());
    for (int i = offset; i < end; ++i) {
        val |= static_cast<quint64>(static_cast<quint8>(data[i]))
               << (8 * (i - offset));
    }
    return val;
}

/** @brief 通用 SipHash 计算 @param data 输入数据 @param cRound 压缩轮数 @param dFinal 终止轮数 */
quint64 SipHash::sipHashImpl(const QByteArray& data, int cRound, int dFinal)
{
    m_timer.start();

    /* 初始化状态 */
    quint64 v0 = 0x736f6d6570736575ULL ^ m_k0;
    quint64 v1 = 0x646f72616e646f6dULL ^ m_k1;
    quint64 v2 = 0x6c7967656e657261ULL ^ m_k0;
    quint64 v3 = 0x7465646279746573ULL ^ m_k1;

    int len = data.size();
    int blocks = len / 8;

    /* 处理完整的 8 字节消息块 */
    for (int i = 0; i < blocks; ++i) {
        quint64 m = readLE64(data, i * 8);
        v3 ^= m;
        for (int c = 0; c < cRound; ++c) {
            sipRound(v0, v1, v2, v3);
        }
        v0 ^= m;
    }

    /* 处理最后一个不完整块(包含长度字节) */
    quint64 last = static_cast<quint64>(len) << 56;
    int remaining = len & 7;
    int base = blocks * 8;
    for (int i = 0; i < remaining; ++i) {
        last |= static_cast<quint64>(static_cast<quint8>(data[base + i]))
                << (8 * i);
    }

    v3 ^= last;
    for (int c = 0; c < cRound; ++c) {
        sipRound(v0, v1, v2, v3);
    }
    v0 ^= last;

    /* 最终化标志 */
    v2 ^= 0xFF;
    for (int d = 0; d < dFinal; ++d) {
        sipRound(v0, v1, v2, v3);
    }

    quint64 result = v0 ^ v1 ^ v2 ^ v3;

    /* 更新统计 */
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    qint64 elapsed = m_timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalHashes);

    emit hashComputed(static_cast<qint64>(len));
    return result;
}

/** @brief 计算 SipHash-2-4 @param data 输入数据 @return 64位哈希值 */
quint64 SipHash::compute(const QByteArray& data)
{
    return sipHashImpl(data, 2, 4);
}

/** @brief 计算 SipHash-1-3 (HalfSipHash) @param data 输入数据 @return 64位哈希值 */
quint64 SipHash::computeHalf(const QByteArray& data)
{
    return sipHashImpl(data, 1, 3);
}

/** @brief 重置所有统计计数器 */
void SipHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
