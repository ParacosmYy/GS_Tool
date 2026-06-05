/**
 * @file Sha512Hash.cpp
 * @brief SHA-512 安全哈希计算引擎实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/sha512/Sha512Hash.h"

/* SHA-512 初始哈希值 (前8个素数的平方根小数部分) */
static const quint64 H_INIT[8] = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
    0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

/* SHA-512 轮常数 K[0..79] (前80个素数的立方根小数部分) */
static const quint64 K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL,
    0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
    0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL,
    0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL,
    0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
    0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 0x2de92c6f592b0275ULL,
    0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL,
    0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
    0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL,
    0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL,
    0x92722c851482353bULL, 0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
    0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL,
    0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL,
    0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
    0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL,
    0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL,
    0xc67178f2e372532bULL, 0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
    0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL,
    0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL,
    0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
    0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

/** @brief 64位循环右移 @param x 输入值 @param n 右移位数 @return 循环右移结果 */
static inline quint64 rotr64(quint64 x, int n)
{
    return (x >> n) | (x << (64 - n));
}

/** @brief SHA-512 Sigma0 函数 @param x 输入值 @return 变换结果 */
static inline quint64 sigma0(quint64 x)
{
    return rotr64(x, 28) ^ rotr64(x, 34) ^ rotr64(x, 39);
}

/** @brief SHA-512 Sigma1 函数 @param x 输入值 @return 变换结果 */
static inline quint64 sigma1(quint64 x)
{
    return rotr64(x, 14) ^ rotr64(x, 18) ^ rotr64(x, 41);
}

/** @brief SHA-512 sigma0 小写(消息调度) @param x 输入值 @return 变换结果 */
static inline quint64 lsigma0(quint64 x)
{
    return rotr64(x, 1) ^ rotr64(x, 8) ^ (x >> 7);
}

/** @brief SHA-512 sigma1 小写(消息调度) @param x 输入值 @return 变换结果 */
static inline quint64 lsigma1(quint64 x)
{
    return rotr64(x, 19) ^ rotr64(x, 61) ^ (x >> 6);
}

/** @brief Ch 函数: 条件选择 @param x 条件 @param y 真值 @param z 假值 @return 结果 */
static inline quint64 ch(quint64 x, quint64 y, quint64 z)
{
    return (x & y) ^ (~x & z);
}

/** @brief Maj 函数: 多数选择 @param x 值1 @param y 值2 @param z 值3 @return 结果 */
static inline quint64 maj(quint64 x, quint64 y, quint64 z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

/** @brief 构造函数 @param parent 父对象 */
Sha512Hash::Sha512Hash(QObject *parent)
    : QObject(parent)
    , m_bufferPos(0)
    , m_totalBits(0)
    , m_finalized(false)
    , m_totalInputSize(0)
    , m_accumulatedTimeMs(0.0)
{
    init();
}

/** @brief 初始化哈希状态，设置初始哈希值 */
void Sha512Hash::init()
{
    memcpy(m_state, H_INIT, sizeof(H_INIT));
    m_bufferPos = 0;
    m_totalBits = 0;
    m_finalized = false;
    m_totalInputSize = 0;
    memset(m_buffer, 0, sizeof(m_buffer));
}

/** @brief 追加数据到哈希计算 @param data 输入数据 */
void Sha512Hash::update(const QByteArray &data)
{
    if (m_finalized || data.isEmpty()) return;

    m_timer.start();
    ++m_stats.totalUpdates;

    const quint8 *ptr = reinterpret_cast<const quint8 *>(data.constData());
    int remaining = data.size();

    while (remaining > 0) {
        int chunk = qMin(remaining, 128 - m_bufferPos);
        memcpy(m_buffer + m_bufferPos, ptr, chunk);
        m_bufferPos += chunk;
        ptr += chunk;
        remaining -= chunk;

        if (m_bufferPos == 128) {
            processBlock(m_buffer);
            m_bufferPos = 0;
        }
    }

    m_totalBits += static_cast<quint64>(data.size()) * 8;
    m_totalInputSize += data.size();
    m_stats.totalBytesProcessed += data.size();
    m_accumulatedTimeMs += m_timer.elapsed();
}

/** @brief 完成哈希计算并返回64字节摘要 */
QByteArray Sha512Hash::finalize()
{
    if (m_finalized) return QByteArray();

    m_timer.start();
    m_finalized = true;

    /* 添加填充位: 1后跟0，再追加128位长度 */
    quint64 msgBits = m_totalBits;
    quint64 padLen = (msgBits % 1024);
    quint64 needed = (padLen <= 896) ? (896 - padLen) : (1920 - padLen);
    quint64 totalPad = needed + 1;

    /* 构造填充数据 */
    QByteArray padding;
    padding.append(static_cast<char>(0x80));
    padding.append(QByteArray(static_cast<int>(totalPad / 8 - 1), 0x00));

    /* 追加128位消息长度(大端) */
    padding.append(QByteArray(8, 0x00));
    for (int i = 0; i < 8; ++i) {
        padding.append(static_cast<char>((msgBits >> (56 - 8 * i)) & 0xFF));
    }

    /* 临时解除 finalized 标志以处理填充 */
    m_finalized = false;
    const quint8 *padPtr = reinterpret_cast<const quint8 *>(padding.constData());
    int padRemaining = padding.size();

    while (padRemaining > 0) {
        int chunk = qMin(padRemaining, 128 - m_bufferPos);
        memcpy(m_buffer + m_bufferPos, padPtr, chunk);
        m_bufferPos += chunk;
        padPtr += chunk;
        padRemaining -= chunk;
        if (m_bufferPos == 128) {
            processBlock(m_buffer);
            m_bufferPos = 0;
        }
    }
    m_finalized = true;

    /* 输出8个64位状态字(大端) */
    QByteArray result;
    result.reserve(64);
    for (int i = 0; i < 8; ++i) {
        for (int j = 7; j >= 0; --j) {
            result.append(static_cast<char>((m_state[i] >> (j * 8)) & 0xFF));
        }
    }

    m_accumulatedTimeMs += m_timer.elapsed();
    ++m_stats.totalHashes;
    m_stats.avgProcessingTimeMs = (m_stats.totalHashes > 0)
        ? m_accumulatedTimeMs / m_stats.totalHashes : 0.0;

    emit hashComputed(m_totalInputSize);
    return result;
}

/** @brief 一次性计算 SHA-512 哈希 @param data 输入数据 @return 哈希摘要 */
QByteArray Sha512Hash::hash(const QByteArray &data)
{
    Sha512Hash h;
    h.init();
    h.update(data);
    return h.finalize();
}

/** @brief 将 8 字节转为 quint64 (大端) @param data 数据指针 @return 64位值 */
quint64 Sha512Hash::toUInt64(const quint8 *data)
{
    return (static_cast<quint64>(data[0]) << 56)
         | (static_cast<quint64>(data[1]) << 48)
         | (static_cast<quint64>(data[2]) << 40)
         | (static_cast<quint64>(data[3]) << 32)
         | (static_cast<quint64>(data[4]) << 24)
         | (static_cast<quint64>(data[5]) << 16)
         | (static_cast<quint64>(data[6]) << 8)
         | static_cast<quint64>(data[7]);
}

/** @brief 处理一个1024位(128字节)消息块 @param block 消息块指针 */
void Sha512Hash::processBlock(const quint8 *block)
{
    quint64 W[80];

    /* 消息调度: 前16个字直接从块中读取 */
    for (int t = 0; t < 16; ++t)
        W[t] = toUInt64(block + t * 8);

    /* 扩展到80个字 */
    for (int t = 16; t < 80; ++t)
        W[t] = lsigma1(W[t - 2]) + W[t - 7] + lsigma0(W[t - 15]) + W[t - 16];

    /* 初始化工作变量 */
    quint64 a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
    quint64 e = m_state[4], f = m_state[5], g = m_state[6], h = m_state[7];

    /* 80 轮压缩 */
    for (int t = 0; t < 80; ++t) {
        quint64 T1 = h + sigma1(e) + ch(e, f, g) + K[t] + W[t];
        quint64 T2 = sigma0(a) + maj(a, b, c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }

    /* 更新哈希状态 */
    m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
    m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
}

/** @brief 获取统计信息 @return 统计结构的常引用 */
const Sha512Hash::Stats &Sha512Hash::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void Sha512Hash::resetStatistics()
{
    m_stats = Stats{};
    m_accumulatedTimeMs = 0.0;
}
