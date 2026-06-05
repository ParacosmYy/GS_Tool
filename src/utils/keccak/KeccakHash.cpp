/**
 * @file KeccakHash.cpp
 * @brief SHA-3 / Keccak 海绵构造哈希算法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/keccak/KeccakHash.h"

#include <QtMath>

/**
 * @brief Keccak-f[1600] 置换中的旋转偏移量表
 * RC[i] 是第 i 轮的轮常量(64位)
 */
static const quint64 KECCAK_RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL,
    0x800000000000808AULL, 0x8000000080008000ULL,
    0x000000000000808BULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008AULL, 0x0000000000000088ULL,
    0x0000000080008009ULL, 0x000000008000000AULL,
    0x000000008000808BULL, 0x800000000000008BULL,
    0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800AULL, 0x800000008000000AULL,
    0x8000000080008081ULL, 0x8000000000008080ULL,
    0x0000000080000001ULL, 0x8000000080008008ULL
};

/** @brief 旋转偏移量 [x][y] */
static const int KECCAK_ROT[5][5] = {
    { 0,  1, 62, 28, 27},
    {36, 44,  6, 55, 20},
    { 3, 10, 43, 25, 39},
    {41, 45, 15, 21,  8},
    {18,  2, 61, 56, 14}
};

/** @brief 构造函数 @param parent 父对象 */
KeccakHash::KeccakHash(QObject* parent)
    : QObject(parent)
    , m_hashBits(256)
    , m_absorbedBytes(0)
    , m_finalized(false)
    , m_timeSum(0.0)
{
    memset(m_state, 0, STATE_SIZE);
    init(256);
}

/** @brief 初始化哈希状态 @param hashBits 输出位宽 */
void KeccakHash::init(int hashBits)
{
    m_hashBits = hashBits;
    /* SHA-3 标准: rate = 1600 - 2*输出位宽 */
    int capacityBits = 2 * hashBits;
    m_rate = (1600 - capacityBits) / 8; /* 转换为字节 */
    memset(m_state, 0, STATE_SIZE);
    m_buffer.clear();
    m_absorbedBytes = 0;
    m_finalized = false;
}

/** @brief 追加数据到哈希计算 @param data 输入数据 */
void KeccakHash::update(const QByteArray& data)
{
    if (data.isEmpty() || m_finalized) return;

    m_buffer.append(data);
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    ++m_stats.totalUpdates;

    /* 尽可能吸收完整的 rate 大小块 */
    while (m_buffer.size() >= m_rate) {
        QByteArray block = m_buffer.left(m_rate);
        m_buffer.remove(0, m_rate);
        absorb(block, 0, block.size());
    }
}

/** @brief 完成哈希计算并返回摘要 @return 哈希摘要字节串 */
QByteArray KeccakHash::finalize()
{
    if (m_finalized) return QByteArray();
    m_finalized = true;

    m_timer.start();

    /* 对剩余数据执行填充 */
    pad();

    QByteArray result;
    squeeze(result, m_hashBits / 8);

    qint64 elapsed = m_timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalHashes;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalHashes);

    emit hashComputed(m_hashBits, static_cast<int>(m_stats.totalBytesProcessed));
    return result;
}

/** @brief 一次性计算哈希 @param data 输入数据 @param bits 输出位宽 @return 哈希摘要 */
QByteArray KeccakHash::hash(const QByteArray& data, int bits)
{
    m_timer.start();
    init(bits);
    update(data);
    QByteArray result = finalize();

    qint64 elapsed = m_timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalHashes);

    return result;
}

/** @brief 重置所有统计计数器 */
void KeccakHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 海绵吸收阶段 */
void KeccakHash::absorb(const QByteArray& input, int offset, int length)
{
    /* XOR 输入数据到状态 */
    for (int i = 0; i < length && (offset + i) < input.size(); ++i) {
        m_state[i] ^= static_cast<quint8>(input[offset + i]);
    }
    keccakF();
}

/** @brief 海绵挤压阶段 */
void KeccakHash::squeeze(QByteArray& output, int outputBytes)
{
    int offset = 0;
    while (offset < outputBytes) {
        int blockSize = qMin(m_rate, outputBytes - offset);
        for (int i = 0; i < blockSize; ++i) {
            output.append(static_cast<char>(m_state[i]));
        }
        offset += blockSize;
        if (offset < outputBytes) {
            keccakF();
        }
    }
}

/** @brief 对状态应用填充规则(SHA-3: 0x06...0x80) */
void KeccakHash::pad()
{
    if (m_buffer.size() >= m_rate) return;

    /* SHA-3 填充: 追加 0x06，然后填充0x00，最后一个字节OR 0x80 */
    m_buffer.append(static_cast<char>(0x06));
    while (m_buffer.size() < m_rate) {
        m_buffer.append(static_cast<char>(0x00));
    }
    m_buffer[m_rate - 1] |= static_cast<char>(0x80);

    /* 吸收填充后的最后一个块 */
    for (int i = 0; i < m_rate; ++i) {
        m_state[i] ^= static_cast<quint8>(m_buffer[i]);
    }
    keccakF();
    m_buffer.clear();
}

/** @brief Keccak-f[1600] 置换函数 */
void KeccakHash::keccakF()
{
    /* 将状态解释为 5x5 的 64位通道数组 */
    quint64 A[5][5];
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            int idx = 8 * (5 * y + x);
            A[x][y] = 0;
            for (int b = 0; b < 8; ++b) {
                A[x][y] |= static_cast<quint64>(m_state[idx + b]) << (8 * b);
            }
        }
    }

    for (int round = 0; round < NUM_ROUNDS; ++round) {
        /* Theta 步骤 */
        quint64 C[5], D[5];
        for (int x = 0; x < 5; ++x) {
            C[x] = A[x][0] ^ A[x][1] ^ A[x][2] ^ A[x][3] ^ A[x][4];
        }
        for (int x = 0; x < 5; ++x) {
            D[x] = C[(x + 4) % 5] ^ ((C[(x + 1) % 5] << 1) | (C[(x + 1) % 5] >> 63));
        }
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                A[x][y] ^= D[x];
            }
        }

        /* Rho + Pi 步骤 */
        quint64 B[5][5];
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                int r = KECCAK_ROT[x][y];
                B[y][(2 * x + 3 * y) % 5] =
                    (A[x][y] << r) | (A[x][y] >> (64 - r));
            }
        }

        /* Chi 步骤 */
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                A[x][y] = B[x][y] ^ (~B[(x + 1) % 5][y] & B[(x + 2) % 5][y]);
            }
        }

        /* Iota 步骤 */
        A[0][0] ^= KECCAK_RC[round];
    }

    /* 将通道数组写回状态 */
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            int idx = 8 * (5 * y + x);
            for (int b = 0; b < 8; ++b) {
                m_state[idx + b] = static_cast<quint8>(A[x][y] >> (8 * b));
            }
        }
    }
}
