/**
 * @file Sha3Hash.cpp
 * @brief SHA-3 (Keccak) 哈希计算引擎实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/sha3/Sha3Hash.h"

#include <QtMath>

/* Keccak-f[1600] 轮常数 (共24个) */
static const quint64 KECCAK_RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808AULL,
    0x8000000080008000ULL, 0x000000000000808BULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008AULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000AULL,
    0x000000008000808BULL, 0x800000000000008BULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800AULL, 0x800000008000000AULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

/* Rho 旋转偏移量表 */
static const int RHO_OFFSETS[25] = {
    0,  1, 62, 28, 27,
    36, 44,  6, 55, 20,
    3, 10, 43, 25, 39,
    41, 45, 15, 21,  8,
    18,  2, 61, 56, 14
};

/* Pi 位置映射: pi_lane[x] = 原位置 */
static const int PI_MAP[5] = {0, 3, 1, 4, 2};

/** @brief 64位循环左移 @param x 输入值 @param n 左移位数 @return 循环左移结果 */
static inline quint64 rotl64(quint64 x, int n)
{
    return (x << n) | (x >> (64 - n));
}

/** @brief 构造函数 @param parent 父对象 */
Sha3Hash::Sha3Hash(QObject *parent)
    : QObject(parent)
    , m_bufferPos(0)
    , m_hashBits(256)
    , m_finalized(false)
    , m_totalInputSize(0)
    , m_accumulatedTimeMs(0.0)
{
    memset(m_state, 0, sizeof(m_state));
    memset(m_buffer, 0, sizeof(m_buffer));
}

/** @brief 计算海绵容量 @return 容量值(比特) */
int Sha3Hash::capacity() const
{
    return 2 * m_hashBits;
}

/** @brief 计算海绵速率(字节) @return 速率值 */
int Sha3Hash::rateBytes() const
{
    return (1600 - capacity()) / 8;
}

/** @brief 初始化哈希状态 @param hashBits 摘要位数 */
void Sha3Hash::init(int hashBits)
{
    m_hashBits = hashBits;
    m_bufferPos = 0;
    m_finalized = false;
    m_totalInputSize = 0;
    memset(m_state, 0, sizeof(m_state));
    memset(m_buffer, 0, sizeof(m_buffer));
}

/** @brief 追加数据到哈希计算 @param data 输入数据 */
void Sha3Hash::update(const QByteArray &data)
{
    if (m_finalized || data.isEmpty()) return;

    m_timer.start();
    ++m_stats.totalUpdates;

    const quint8 *ptr = reinterpret_cast<const quint8 *>(data.constData());
    int remaining = data.size();
    int rate = rateBytes();

    while (remaining > 0) {
        int chunk = qMin(remaining, rate - m_bufferPos);
        memcpy(m_buffer + m_bufferPos, ptr, chunk);
        m_bufferPos += chunk;
        ptr += chunk;
        remaining -= chunk;

        if (m_bufferPos == rate) {
            absorb(m_buffer, 0, rate);
            m_bufferPos = 0;
        }
    }

    m_totalInputSize += data.size();
    m_stats.totalBytesProcessed += data.size();
    m_accumulatedTimeMs += m_timer.elapsed();
}

/** @brief 完成哈希计算并返回摘要 */
QByteArray Sha3Hash::finalize()
{
    if (m_finalized) return QByteArray();

    m_timer.start();
    m_finalized = true;
    int rate = rateBytes();

    /* SHA-3 填充: 追加 0x06，然后填充 0x00，最后字节或 0x80 */
    m_buffer[m_bufferPos] ^= 0x06;
    m_buffer[rate - 1] ^= 0x80;
    absorb(m_buffer, 0, rate);

    int digestBytes = m_hashBits / 8;
    QByteArray result = squeeze(digestBytes);

    m_accumulatedTimeMs += m_timer.elapsed();
    ++m_stats.totalHashes;
    m_stats.avgProcessingTimeMs = (m_stats.totalHashes > 0)
        ? m_accumulatedTimeMs / m_stats.totalHashes : 0.0;

    emit hashComputed(m_hashBits, m_totalInputSize);
    return result;
}

/** @brief 一次性计算 SHA-3 哈希 @param data 输入数据 @param bits 摘要位数 @return 哈希摘要 */
QByteArray Sha3Hash::hash(const QByteArray &data, int bits)
{
    Sha3Hash h;
    h.init(bits);
    h.update(data);
    return h.finalize();
}

/** @brief 吸收数据到状态 @param data 数据指针 @param offset 偏移量 @param length 长度 */
void Sha3Hash::absorb(const quint8 *data, int offset, int length)
{
    int rate = rateBytes();
    for (int i = 0; i < length; ++i) {
        int lane = i / 8;
        int bytePos = i % 8;
        m_state[lane] ^= static_cast<quint64>(data[offset + i]) << (8 * bytePos);
    }
    if (length == rate) {
        keccakF();
    }
}

/** @brief 挤出哈希摘要 @param digestBytes 需要挤出的字节数 */
QByteArray Sha3Hash::squeeze(int digestBytes)
{
    QByteArray result;
    result.resize(digestBytes);
    int rate = rateBytes();
    int offset = 0;

    while (offset < digestBytes) {
        int chunk = qMin(rate, digestBytes - offset);
        for (int i = 0; i < chunk; ++i) {
            int lane = i / 8;
            int bytePos = i % 8;
            result[offset + i] = static_cast<char>(
                (m_state[lane] >> (8 * bytePos)) & 0xFF);
        }
        offset += chunk;
        if (offset < digestBytes) keccakF();
    }
    return result;
}

/** @brief Keccak-f[1600] 完整置换，24轮 */
void Sha3Hash::keccakF()
{
    for (int round = 0; round < 24; ++round) {
        theta();
        rho();
        pi();
        chi();
        iota(round);
    }
}

/** @brief Theta 步: 列奇偶校验扩散 */
void Sha3Hash::theta()
{
    quint64 C[5], D[5];
    for (int x = 0; x < 5; ++x)
        C[x] = m_state[x] ^ m_state[x + 5] ^ m_state[x + 10]
               ^ m_state[x + 15] ^ m_state[x + 20];
    for (int x = 0; x < 5; ++x)
        D[x] = C[(x + 4) % 5] ^ rotl64(C[(x + 1) % 5], 1);
    for (int x = 0; x < 5; ++x)
        for (int y = 0; y < 5; ++y)
            m_state[5 * y + x] ^= D[x];
}

/** @brief Rho 步: 位平面旋转 */
void Sha3Hash::rho()
{
    quint64 tmp[25];
    for (int i = 0; i < 25; ++i)
        tmp[i] = rotl64(m_state[i], RHO_OFFSETS[i]);
    memcpy(m_state, tmp, sizeof(m_state));
}

/** @brief Pi 步: 位置置换 */
void Sha3Hash::pi()
{
    quint64 tmp[25];
    for (int x = 0; x < 5; ++x)
        for (int y = 0; y < 5; ++y)
            tmp[5 * y + x] = m_state[5 * PI_MAP[x] + y];
    memcpy(m_state, tmp, sizeof(m_state));
}

/** @brief Chi 步: 非线性变换 */
void Sha3Hash::chi()
{
    quint64 tmp[25];
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            tmp[5 * y + x] = m_state[5 * y + x]
                ^ ((~m_state[5 * y + (x + 1) % 5])
                   & m_state[5 * y + (x + 2) % 5]);
        }
    }
    memcpy(m_state, tmp, sizeof(m_state));
}

/** @brief Iota 步: 轮常数异或 @param round 当前轮次 */
void Sha3Hash::iota(int round)
{
    m_state[0] ^= KECCAK_RC[round];
}

/** @brief 获取统计信息 @return 统计结构的常引用 */
const Sha3Hash::Stats &Sha3Hash::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void Sha3Hash::resetStatistics()
{
    m_stats = Stats{};
    m_accumulatedTimeMs = 0.0;
}
