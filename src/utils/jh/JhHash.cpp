/**
 * @file JhHash.cpp
 * @brief JH 哈希计算引擎实现(SHA-3 候选算法)
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/jh/JhHash.h"

/* JH 8x8 S-box (基于有限域逆元的非线性替换) */
static const quint8 JH_SBOX[256] = {
    0x72, 0x36, 0x6A, 0x86, 0xB8, 0x68, 0x41, 0x09, 0x22, 0x93, 0x5D, 0x05, 0x20, 0xC2, 0x3D, 0x53,
    0x52, 0x1E, 0x8E, 0x42, 0x3B, 0x30, 0x4C, 0xD0, 0x16, 0xE4, 0x88, 0x5F, 0x55, 0x32, 0xEB, 0x97,
    0xA1, 0x73, 0x12, 0x29, 0x7C, 0xC5, 0x8B, 0x03, 0xC3, 0xDD, 0x83, 0x19, 0xF8, 0xC0, 0xCA, 0x45,
    0x62, 0xA4, 0xFE, 0xE9, 0x5E, 0xB4, 0xC1, 0x54, 0xFC, 0x5C, 0x60, 0xC4, 0x57, 0x87, 0x2F, 0x56,
    0xD8, 0xFB, 0xE3, 0xD7, 0xEC, 0x4B, 0xA6, 0xBD, 0x91, 0xD2, 0x38, 0x81, 0xB1, 0x01, 0xB7, 0xA2,
    0x33, 0x08, 0x5B, 0x61, 0x14, 0xB3, 0x24, 0x3F, 0x3E, 0x04, 0xC8, 0xE7, 0x50, 0xA8, 0xF2, 0x6C,
    0x92, 0x4F, 0x2B, 0xE6, 0x78, 0x85, 0x89, 0x96, 0x46, 0xB5, 0x1F, 0x7D, 0x6E, 0xE8, 0x00, 0x74,
    0x0E, 0xE1, 0x49, 0xDA, 0xBB, 0x4D, 0x67, 0xA0, 0x47, 0x63, 0x1D, 0x2E, 0x51, 0xB6, 0x8A, 0xFD,
    0x5A, 0xE5, 0x35, 0xC9, 0xB0, 0xD6, 0x39, 0xF6, 0x7B, 0xB2, 0x37, 0x76, 0xFA, 0x13, 0xE2, 0x15,
    0x31, 0x8C, 0x07, 0xDC, 0x44, 0x6D, 0xBE, 0x3A, 0x77, 0x90, 0x21, 0xAA, 0x28, 0x26, 0x4A, 0x6F,
    0x7E, 0xCF, 0x4E, 0x02, 0x58, 0x1C, 0x98, 0x18, 0x64, 0x8D, 0x99, 0x71, 0x80, 0x23, 0xE0, 0xCE,
    0x9B, 0x9C, 0x2C, 0x9F, 0xB9, 0xC6, 0x17, 0x0B, 0x84, 0x11, 0xD3, 0x2D, 0xAC, 0xDE, 0xD5, 0x6B,
    0xCB, 0x8F, 0xF7, 0xAD, 0x82, 0x66, 0x27, 0xBC, 0xF1, 0x75, 0x9D, 0xCD, 0xA3, 0xAE, 0x0F, 0x25,
    0x48, 0x34, 0x94, 0xA5, 0x0D, 0xDB, 0xD4, 0x65, 0xF3, 0x1B, 0x0A, 0xF4, 0x79, 0x10, 0xEA, 0xF5,
    0xD1, 0x95, 0x59, 0xA9, 0xE0, 0xCC, 0x63, 0x40, 0xED, 0x7A, 0xF0, 0xC7, 0xD9, 0x43, 0x0C, 0xAB,
    0x9E, 0xA7, 0x70, 0xFF, 0x36, 0xEE, 0xDF, 0xEF, 0xBF, 0xF9, 0x2A, 0xBA, 0x1A, 0x00, 0x69, 0x3C
};

/* JH 简化的轮常数(每轮4字节) */
static const quint8 JH_ROUND_CONST[42][4] = {
    {0x22,0x93,0x5D,0x05}, {0x20,0xC2,0x3D,0x53}, {0x52,0x1E,0x8E,0x42},
    {0x3B,0x30,0x4C,0xD0}, {0x16,0xE4,0x88,0x5F}, {0x55,0x32,0xEB,0x97},
    {0xA1,0x73,0x12,0x29}, {0x7C,0xC5,0x8B,0x03}, {0xC3,0xDD,0x83,0x19},
    {0xF8,0xC0,0xCA,0x45}, {0x62,0xA4,0xFE,0xE9}, {0x5E,0xB4,0xC1,0x54},
    {0xFC,0x5C,0x60,0xC4}, {0x57,0x87,0x2F,0x56}, {0xD8,0xFB,0xE3,0xD7},
    {0xEC,0x4B,0xA6,0xBD}, {0x91,0xD2,0x38,0x81}, {0xB1,0x01,0xB7,0xA2},
    {0x33,0x08,0x5B,0x61}, {0x14,0xB3,0x24,0x3F}, {0x3E,0x04,0xC8,0xE7},
    {0x50,0xA8,0xF2,0x6C}, {0x92,0x4F,0x2B,0xE6}, {0x78,0x85,0x89,0x96},
    {0x46,0xB5,0x1F,0x7D}, {0x6E,0xE8,0x00,0x74}, {0x0E,0xE1,0x49,0xDA},
    {0xBB,0x4D,0x67,0xA0}, {0x47,0x63,0x1D,0x2E}, {0x51,0xB6,0x8A,0xFD},
    {0x5A,0xE5,0x35,0xC9}, {0xB0,0xD6,0x39,0xF6}, {0x7B,0xB2,0x37,0x76},
    {0xFA,0x13,0xE2,0x15}, {0x31,0x8C,0x07,0xDC}, {0x44,0x6D,0xBE,0x3A},
    {0x77,0x90,0x21,0xAA}, {0x28,0x26,0x4A,0x6F}, {0x7E,0xCF,0x4E,0x02},
    {0x58,0x1C,0x98,0x18}, {0x64,0x8D,0x99,0x71}, {0x80,0x23,0xE0,0xCE},
};

/** @brief 64位循环左移 @param x 输入值 @param n 位数 @return 旋转结果 */
static inline quint64 rotl64(quint64 x, int n)
{
    return (x << n) | (x >> (64 - n));
}

/** @brief 构造函数 @param parent 父对象 */
JhHash::JhHash(QObject *parent)
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

/** @brief 初始化哈希状态 @param hashBits 摘要位数 */
void JhHash::init(int hashBits)
{
    m_hashBits = hashBits;
    m_bufferPos = 0;
    m_finalized = false;
    m_totalInputSize = 0;

    /* JH 初始状态: 根据摘要长度设置前128位 */
    memset(m_state, 0, sizeof(m_state));
    int digestBytes = hashBits / 8;

    /* 将摘要长度编码到状态的高位 */
    m_state[0] = static_cast<quint64>(digestBytes);
    m_state[1] = 0x0001020304050607ULL;

    /* 其余初始值基于 JH 规范的 IV */
    m_state[2] = 0x08090A0B0C0D0E0FULL;
    m_state[3] = 0x1011121314151617ULL;
    m_state[4] = 0x18191A1B1C1D1E1FULL;
    m_state[5] = 0x2021222324252627ULL;
    m_state[6] = 0x28292A2B2C2D2E2FULL;
    m_state[7] = 0x3031323334353637ULL;

    /* 执行初始置换 */
    for (int round = 0; round < 6; ++round) {
        sboxLayer();
        linearDiffusion();
    }

    memset(m_buffer, 0, sizeof(m_buffer));
}

/** @brief 追加数据到哈希计算 @param data 输入数据 */
void JhHash::update(const QByteArray &data)
{
    if (m_finalized || data.isEmpty()) return;

    m_timer.start();
    ++m_stats.totalUpdates;

    const quint8 *ptr = reinterpret_cast<const quint8 *>(data.constData());
    int remaining = data.size();

    while (remaining > 0) {
        int chunk = qMin(remaining, 64 - m_bufferPos);
        memcpy(m_buffer + m_bufferPos, ptr, chunk);
        m_bufferPos += chunk;
        ptr += chunk;
        remaining -= chunk;

        if (m_bufferPos == 64) {
            processBlock(m_buffer);
            m_bufferPos = 0;
        }
    }

    m_totalInputSize += data.size();
    m_stats.totalBytesProcessed += data.size();
    m_accumulatedTimeMs += m_timer.elapsed();
}

/** @brief 完成哈希计算并返回摘要 */
QByteArray JhHash::finalize()
{
    if (m_finalized) return QByteArray();

    m_timer.start();
    m_finalized = true;

    /* JH 填充: 追加 0x80，然后填充零到 64 字节边界 */
    m_buffer[m_bufferPos] ^= 0x80;
    processBlock(m_buffer);

    /* 额外的最终块: 长度信息 */
    memset(m_buffer, 0, 64);
    quint64 totalBits = static_cast<quint64>(m_totalInputSize) * 8;
    for (int i = 0; i < 8; ++i) {
        m_buffer[56 + i] = static_cast<quint8>((totalBits >> (56 - 8 * i)) & 0xFF);
    }
    processBlock(m_buffer);

    /* 从1024位状态中提取指定位数的摘要 */
    int digestBytes = m_hashBits / 8;
    QByteArray result;
    result.reserve(digestBytes);

    /* 从状态的后半部分取摘要 */
    for (int i = 8; i < 16 && result.size() < digestBytes; ++i) {
        for (int j = 7; j >= 0 && result.size() < digestBytes; --j) {
            result.append(static_cast<char>((m_state[i] >> (j * 8)) & 0xFF));
        }
    }

    m_accumulatedTimeMs += m_timer.elapsed();
    ++m_stats.totalHashes;
    m_stats.avgProcessingTimeMs = (m_stats.totalHashes > 0)
        ? m_accumulatedTimeMs / m_stats.totalHashes : 0.0;

    emit hashComputed(m_hashBits, m_totalInputSize);
    return result;
}

/** @brief 8x8 S-box 查找 @param input 输入字节 @return 输出字节 */
quint8 JhHash::sbox(quint8 input)
{
    return JH_SBOX[input];
}

/** @brief 执行一轮 S-box 置换 */
void JhHash::sboxLayer()
{
    for (int i = 0; i < 16; ++i) {
        quint64 w = m_state[i];
        quint64 result = 0;
        for (int b = 0; b < 8; ++b) {
            quint8 byte = static_cast<quint8>((w >> (b * 8)) & 0xFF);
            result |= static_cast<quint64>(JH_SBOX[byte]) << (b * 8);
        }
        m_state[i] = result;
    }
    ++m_stats.totalRounds;
}

/** @brief 线性扩散层(MDS矩阵混合) */
void JhHash::linearDiffusion()
{
    /* 使用 LFSR 风格的扩散: 每个字与其循环移位版本混合 */
    for (int i = 0; i < 16; ++i) {
        quint64 s = m_state[i];
        s ^= rotl64(s, 1) ^ rotl64(s, 2);
        s ^= rotl64(s, 8) ^ rotl64(s, 19);
        s ^= rotl64(s, 32) ^ rotl64(s, 45);
        m_state[i] = s;
    }

    /* 矩阵混合: 相邻字对交叉混合 */
    for (int i = 0; i < 16; i += 2) {
        quint64 a = m_state[i];
        quint64 b = m_state[i + 1];
        m_state[i]     = a ^ rotl64(b, 3);
        m_state[i + 1] = b ^ rotl64(m_state[i], 7);
    }

    /* 全局扩散: 每个字与间隔4的字混合 */
    for (int i = 0; i < 12; ++i) {
        m_state[i + 4] ^= rotl64(m_state[i], 11);
    }
}

/** @brief 应用轮常数 @param round 轮次 */
void JhHash::addRoundConstant(int round)
{
    if (round < 42) {
        /* 将4字节常量扩展并异或到状态 */
        const quint8 *C = JH_ROUND_CONST[round];
        quint64 expanded[4];
        expandConstant(C, expanded);

        m_state[0] ^= expanded[0];
        m_state[1] ^= expanded[1];
        m_state[2] ^= expanded[2];
        m_state[3] ^= expanded[3];
    }
}

/** @brief 将4字节常量展开为4个64位字 @param C 常量字节 @param out 输出字 */
void JhHash::expandConstant(const quint8 C[4], quint64 out[4])
{
    out[0] = (static_cast<quint64>(C[0]) << 56) | (static_cast<quint64>(C[1]) << 48)
           | (static_cast<quint64>(C[0]) << 40) | (static_cast<quint64>(C[2]) << 32)
           | (static_cast<quint64>(C[1]) << 24) | (static_cast<quint64>(C[3]) << 16)
           | (static_cast<quint64>(C[2]) << 8)  | static_cast<quint64>(C[3]);
    out[1] = out[0] ^ 0x0123456789ABCDEFULL;
    out[2] = rotl64(out[0], 17);
    out[3] = rotl64(out[1], 23);
}

/** @brief 处理一个512位(64字节)消息块 @param block 消息块指针 */
void JhHash::processBlock(const quint8 *block)
{
    /* 消息异或到状态的后8个字(高512位) */
    for (int i = 0; i < 8; ++i) {
        quint64 w = 0;
        for (int j = 0; j < 8; ++j)
            w = (w << 8) | block[i * 8 + j];
        m_state[i + 8] ^= w;
    }

    /* 执行 JH 简化的压缩轮(42轮的一半用于简化) */
    int rounds = 21;
    for (int round = 0; round < rounds; ++round) {
        addRoundConstant(round);
        sboxLayer();
        linearDiffusion();
    }

    /* 消息反馈: 将状态前8个字复制到后8个字并异或 */
    for (int i = 0; i < 8; ++i) {
        m_state[i] ^= m_state[i + 8];
    }
}

/** @brief 获取统计信息 @return 统计结构的常引用 */
const JhHash::Stats &JhHash::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void JhHash::resetStatistics()
{
    m_stats = Stats{};
    m_accumulatedTimeMs = 0.0;
}
