/**
 * @file EchoHash.cpp
 * @brief Echo 哈希计算引擎实现(AES-Based)
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/echohash/EchoHash.h"

/* AES S-box 查找表 */
static const quint8 AES_SBOX[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

/** @brief 64位循环左移 @param x 输入值 @param n 位数 @return 旋转结果 */
static inline quint64 rotl64(quint64 x, int n)
{
    return (x << n) | (x >> (64 - n));
}

/** @brief 构造函数 @param parent 父对象 */
EchoHash::EchoHash(QObject *parent)
    : QObject(parent)
    , m_seed(0x517CC1B727220A95ULL)
    , m_accumulatedTimeMs(0.0)
{
}

/** @brief 设置哈希种子值 @param seed 64位种子 */
void EchoHash::setSeed(quint64 seed)
{
    m_seed = seed;
}

/** @brief AES S-box 替换 @param byte 输入字节 @return S-box 输出 */
quint8 EchoHash::aesSbox(quint8 byte)
{
    return AES_SBOX[byte];
}

/** @brief 对64位值进行 AES SubBytes 替换 @param val 输入值 @return 替换结果 */
quint64 EchoHash::subBytes64(quint64 val)
{
    quint64 result = 0;
    for (int i = 0; i < 8; ++i) {
        quint8 byte = static_cast<quint8>((val >> (i * 8)) & 0xFF);
        result |= static_cast<quint64>(AES_SBOX[byte]) << (i * 8);
    }
    return result;
}

/** @brief 生成轮密钥 @param round 轮次 @return 轮密钥 */
quint64 EchoHash::deriveRoundKey(int round) const
{
    /* 基于种子和轮次派生密钥 */
    quint64 k = m_seed ^ (static_cast<quint64>(round) * 0x9E3779B97F4A7C15ULL);
    k = (k ^ (k >> 30)) * 0xBF58476D1CE4E5B9ULL;
    k = (k ^ (k >> 27)) * 0x94D049BB133111EBULL;
    return k ^ (k >> 31);
}

/** @brief Feistel 混合轮 @param left 左半部分 @param right 右半部分 @param roundKey 轮密钥 */
void EchoHash::feistelRound(quint64 &left, quint64 &right, quint64 roundKey)
{
    quint64 mixed = subBytes64(left ^ roundKey);
    mixed = rotl64(mixed, 13);
    mixed *= 0x517CC1B727220A95ULL;
    mixed = rotl64(mixed, 7);
    mixed ^= right;
    right = left;
    left = mixed;
    ++m_stats.totalRounds;
}

/** @brief 对一个数据块进行哈希处理 @param block 数据指针 @param size 大小 @param blockIndex 块索引 */
quint64 EchoHash::hashBlock64(const quint8 *block, int size, quint64 blockIndex)
{
    /* 初始化: 将块数据混入初始值 */
    quint64 h = m_seed ^ blockIndex;
    for (int i = 0; i + 7 < size; i += 8) {
        quint64 w = 0;
        for (int j = 0; j < 8; ++j)
            w = (w << 8) | block[i + j];
        h ^= w;
    }
    /* 处理尾部不足8字节 */
    int tail = size % 8;
    if (tail > 0) {
        quint64 w = 0;
        int start = size - tail;
        for (int j = 0; j < tail; ++j)
            w = (w << 8) | block[start + j];
        w <<= (8 - tail) * 8;
        h ^= w;
    }

    /* 8轮 Feistel 混合 */
    quint64 left = h;
    quint64 right = m_seed ^ rotl64(h, 17);
    for (int round = 0; round < 8; ++round) {
        feistelRound(left, right, deriveRoundKey(round));
    }

    return left ^ right;
}

/** @brief 计算64位哈希值 @param data 输入数据 @return 64位哈希值 */
quint64 EchoHash::hash64(const QByteArray &data)
{
    m_timer.start();
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += data.size();

    if (data.isEmpty()) {
        m_accumulatedTimeMs += m_timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalHashes > 0)
            ? m_accumulatedTimeMs / m_stats.totalHashes : 0.0;
        emit hashComputed(0);
        return m_seed;
    }

    const quint8 *ptr = reinterpret_cast<const quint8 *>(data.constData());
    int remaining = data.size();
    quint64 result = m_seed;
    quint64 blockIndex = 0;

    /* 每64字节一块 */
    while (remaining > 0) {
        int chunkSize = qMin(remaining, 64);
        quint64 blockHash = hashBlock64(ptr, chunkSize, blockIndex);
        result ^= blockHash;
        /* 额外混合 */
        result = subBytes64(result);
        result = rotl64(result, 17);
        ptr += chunkSize;
        remaining -= chunkSize;
        ++blockIndex;
    }

    /* 最终混合 */
    result ^= static_cast<quint64>(data.size()) * 0x9E3779B97F4A7C15ULL;
    result = subBytes64(result);
    result ^= deriveRoundKey(99);

    m_accumulatedTimeMs += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalHashes > 0)
        ? m_accumulatedTimeMs / m_stats.totalHashes : 0.0;

    emit hashComputed(data.size());
    return result;
}

/** @brief 计算128位哈希值 @param data 输入数据 @return QPair(高64位, 低64位) */
QPair<quint64, quint64> EchoHash::hash128(const QByteArray &data)
{
    m_timer.start();
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += data.size();

    /* 使用两种不同种子产生两个独立的64位哈希 */
    quint64 origSeed = m_seed;

    quint64 high = hash64(data);

    m_seed = origSeed ^ 0xFFFFFFFFFFFFFFFFULL;
    /* 重置统计(避免双重计数) */
    --m_stats.totalHashes;
    m_stats.totalBytesProcessed -= data.size();

    quint64 low = hash64(data);

    /* 恢复种子 */
    m_seed = origSeed;

    /* 交叉混合确保两半独立 */
    high ^= rotl64(low, 23);
    low ^= rotl64(high, 37);

    return qMakePair(high, low);
}

/** @brief 获取统计信息 @return 统计结构的常引用 */
const EchoHash::Stats &EchoHash::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void EchoHash::resetStatistics()
{
    m_stats = Stats{};
    m_accumulatedTimeMs = 0.0;
}
