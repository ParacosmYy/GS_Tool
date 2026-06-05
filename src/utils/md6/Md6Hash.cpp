/**
 * @file Md6Hash.cpp
 * @brief MD6 Merkle 树哈希计算引擎实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/md6/Md6Hash.h"

#include <QtMath>

/* MD6 算法常量 */
static const int MD6_WORD_SIZE = 64;    ///< 字大小(位)
static const int MD6_COMPRESS_WORDS = 64; ///< 压缩函数内部状态字数
static const int MD6_CHUNK_BYTES = 64;  ///< 每个数据块的字节数
static const int MD6_OUTPUT_BYTES = 64; ///< 压缩函数输出字节数

/** @brief MD6 简化的非线性函数 @param x 输入1 @param y 输入2 @param z 输入3 @return 混合结果 */
static inline quint64 md6F(quint64 x, quint64 y, quint64 z)
{
    return (x & y) ^ (~x & z) ^ (x & z);
}

/** @brief 左移位旋转 @param x 输入值 @param n 位数 @return 旋转结果 */
static inline quint64 rotl(quint64 x, int n)
{
    return (x << n) | (x >> (64 - n));
}

/** @brief MD6 内部扩散常量 */
static const quint64 MD6_DIFF_CONST[4] = {
    0x7311C2812425CFA0ULL, 0x6B9E9AB60424B45FULL,
    0xD9B5BF2C4FD95B7DULL, 0xA67E27F8B1E2B839ULL
};

/** @brief 构造函数 @param parent 父对象 */
Md6Hash::Md6Hash(QObject *parent)
    : QObject(parent)
    , m_levels(0)
    , m_chunkSize(MD6_CHUNK_BYTES)
    , m_accumulatedTimeMs(0.0)
{
}

/** @brief 设置 Merkle 树层级数 @param levels 层级数 */
void Md6Hash::setLevels(int levels)
{
    m_levels = qMax(0, levels);
}

/** @brief 设置密钥 @param key 密钥数据(最多64字节) */
void Md6Hash::setKey(const QByteArray &key)
{
    m_key = key.left(64);
}

/** @brief 计算数据的 MD6 哈希 @param data 输入数据 @param digestBits 摘要位数 */
QByteArray Md6Hash::hash(const QByteArray &data, int digestBits)
{
    m_timer.start();
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += data.size();

    QByteArray result;
    if (m_levels == 0) {
        /* 串行模式: 逐块压缩，链接输出 */
        result = treeHash(data, 0);
    } else {
        result = treeHash(data, m_levels);
    }

    result = truncate(result, digestBits);

    m_accumulatedTimeMs += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalHashes > 0)
        ? m_accumulatedTimeMs / m_stats.totalHashes : 0.0;

    emit hashComputed(digestBits, data.size());
    return result;
}

/** @brief Merkle 树递归哈希 @param data 输入数据 @param level 当前层级 */
QByteArray Md6Hash::treeHash(const QByteArray &data, int level)
{
    if (data.size() <= m_chunkSize) {
        /* 叶子节点: 直接压缩 */
        QByteArray padded = data;
        while (padded.size() < m_chunkSize)
            padded.append(static_cast<char>(0x00));
        return compress(padded, 0, true);
    }

    /* 分块处理 */
    QList<QByteArray> chunks;
    for (int i = 0; i < data.size(); i += m_chunkSize) {
        QByteArray chunk = data.mid(i, m_chunkSize);
        while (chunk.size() < m_chunkSize)
            chunk.append(static_cast<char>(0x00));
        chunks.append(chunk);
    }

    if (level <= 1) {
        /* 串行链接模式 */
        QByteArray chained;
        for (int i = 0; i < chunks.size(); ++i) {
            bool isLast = (i == chunks.size() - 1);
            QByteArray compressed = compress(chunks[i], i, isLast);
            chained.append(compressed);
        }
        if (chained.size() > m_chunkSize) {
            chained = chained.left(m_chunkSize);
        }
        return chained;
    }

    /* 并行树模式: 递归合并 */
    QList<QByteArray> results;
    for (int i = 0; i < chunks.size(); ++i) {
        QByteArray compressed = compress(chunks[i], i, false);
        results.append(compressed);
    }

    /* 合并相邻结果 */
    QByteArray combined;
    for (const auto &r : results)
        combined.append(r);

    if (combined.size() <= m_chunkSize) {
        return compress(combined.leftJustified(m_chunkSize, 0), 0, true);
    }

    return treeHash(combined, level - 1);
}

/** @brief 构建控制字 @param nodeID 节点标识 @param isLast 最后块标志 @param level 层级 @param ctrl 输出控制字 */
void Md6Hash::buildControlWord(quint64 nodeID, bool isLast, int level,
                               quint64 ctrl[4])
{
    ctrl[0] = static_cast<quint64>(level) | (isLast ? (1ULL << 8) : 0);
    ctrl[1] = nodeID;
    ctrl[2] = static_cast<quint64>(m_key.size()) << 48;
    ctrl[3] = MD6_DIFF_CONST[level % 4];
}

/** @brief MD6 压缩函数 @param data 输入块(64字节) @param nodeID 节点标识 @param isLast 最后块 */
QByteArray Md6Hash::compress(const QByteArray &data, quint64 nodeID, bool isLast)
{
    ++m_stats.totalCompressions;

    quint64 S[MD6_COMPRESS_WORDS];
    memset(S, 0, sizeof(S));

    /* 加载控制字 */
    quint64 ctrl[4];
    buildControlWord(nodeID, isLast, 0, ctrl);
    S[0] = ctrl[0]; S[1] = ctrl[1]; S[2] = ctrl[2]; S[3] = ctrl[3];

    /* 加载密钥 */
    int keyWords = qMin(m_key.size() / 8, 8);
    const quint8 *keyPtr = reinterpret_cast<const quint8 *>(m_key.constData());
    for (int i = 0; i < keyWords; ++i) {
        quint64 w = 0;
        for (int j = 0; j < 8; ++j)
            w = (w << 8) | keyPtr[i * 8 + j];
        S[4 + i] = w;
    }

    /* 加载数据字 */
    const quint8 *dataPtr = reinterpret_cast<const quint8 *>(data.constData());
    for (int i = 0; i < 8 && (i * 8) < data.size(); ++i) {
        quint64 w = 0;
        for (int j = 0; j < 8 && (i * 8 + j) < data.size(); ++j)
            w = (w << 8) | dataPtr[i * 8 + j];
        S[12 + i] = w;
    }

    /* 简化的压缩轮(多轮扩散) */
    for (int round = 0; round < 40; ++round) {
        for (int i = 0; i < MD6_COMPRESS_WORDS - 2; ++i) {
            S[i] = S[i] ^ md6F(S[i + 1], S[i + 2], S[(i + 3) % MD6_COMPRESS_WORDS]);
            S[i] = rotl(S[i], 1 + (round % 7));
            S[i] ^= MD6_DIFF_CONST[round % 4];
        }
        /* 最后一字混合回开头 */
        S[MD6_COMPRESS_WORDS - 1] = rotl(
            S[MD6_COMPRESS_WORDS - 1] ^ S[0], 3);
    }

    /* 输出8个字(64字节) */
    QByteArray out;
    out.reserve(MD6_OUTPUT_BYTES);
    for (int i = 0; i < 8; ++i) {
        for (int j = 7; j >= 0; --j)
            out.append(static_cast<char>((S[i + 56] >> (j * 8)) & 0xFF));
    }
    return out;
}

/** @brief 截断或扩展输出到指定位数 @param data 输入数据 @param bits 目标位数 */
QByteArray Md6Hash::truncate(const QByteArray &data, int bits)
{
    int bytes = bits / 8;
    if (data.size() >= bytes) {
        return data.left(bytes);
    }
    /* 数据不足时补零并混淆 */
    QByteArray result = data;
    quint64 ctr = 0xA5A5A5A5A5A5A5A5ULL;
    while (result.size() < bytes) {
        ctr ^= static_cast<quint64>(result.size()) * 0x9E3779B97F4A7C15ULL;
        result.append(static_cast<char>(ctr & 0xFF));
    }
    return result;
}

/** @brief 获取统计信息 @return 统计结构的常引用 */
const Md6Hash::Stats &Md6Hash::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void Md6Hash::resetStatistics()
{
    m_stats = Stats{};
    m_accumulatedTimeMs = 0.0;
}
