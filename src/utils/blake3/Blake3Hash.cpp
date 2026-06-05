/**
 * @file Blake3Hash.cpp
 * @brief BLAKE3 哈希算法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/blake3/Blake3Hash.h"

/** @brief 构造函数 @param parent 父对象 */
Blake3Hash::Blake3Hash(QObject* parent)
    : QObject(parent)
    , m_chunkCounter(0)
    , m_flags(0)
    , m_bufPos(0)
    , m_finalized(false)
    , m_timeSum(0.0)
{
    init();
}

/** @brief 初始化哈希状态 */
void Blake3Hash::init()
{
    /* 使用 BLAKE3 标准初始向量作为链值 */
    m_key[0] = IV_0; m_key[1] = IV_1;
    m_key[2] = IV_2; m_key[3] = IV_3;
    m_key[4] = IV_4; m_key[5] = IV_5;
    m_key[6] = IV_6; m_key[7] = IV_7;

    m_chunkCounter = 0;
    m_flags = 0;
    m_buffer.resize(CHUNK_BYTES);
    m_bufPos = 0;
    m_finalized = false;
}

/** @brief 追加数据到哈希计算 @param data 输入数据 */
void Blake3Hash::update(const QByteArray& data)
{
    if (data.isEmpty() || m_finalized) return;

    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());

    int srcPos = 0;
    while (srcPos < data.size()) {
        /* 填充缓冲区到 CHUNK_BYTES */
        int space = CHUNK_BYTES - m_bufPos;
        int toCopy = qMin(space, data.size() - srcPos);
        memcpy(m_buffer.data() + m_bufPos, data.constData() + srcPos, toCopy);
        m_bufPos += toCopy;
        srcPos += toCopy;

        /* 缓冲区满时处理整个 chunk */
        if (m_bufPos == CHUNK_BYTES) {
            processChunk();
            m_bufPos = 0;
            ++m_chunkCounter;
            ++m_stats.totalChunks;
        }
    }
}

/** @brief 处理一个完整的 chunk(1024字节) */
void Blake3Hash::processChunk()
{
    quint32 cv[8];
    memcpy(cv, m_key, sizeof(cv));

    int blocks = CHUNK_BYTES / BLOCK_BYTES; /* 16 个块 */
    for (int i = 0; i < blocks; ++i) {
        quint32 block[16];
        loadWords(m_buffer, i * BLOCK_BYTES, block);

        quint32 flags = CHUNK_START;
        if (i == blocks - 1) flags |= CHUNK_END;

        quint32 out[16];
        compress(cv, block, m_chunkCounter, flags, out);
        memcpy(cv, out, 8 * sizeof(quint32));
    }

    /* 更新链值 */
    memcpy(m_key, cv, sizeof(m_key));
}

/** @brief 完成哈希计算并返回摘要 @param outputBytes 输出字节数 @return 哈希摘要 */
QByteArray Blake3Hash::finalize(int outputBytes)
{
    if (m_finalized) return QByteArray();
    m_finalized = true;

    m_timer.start();

    QByteArray result;
    result.reserve(outputBytes);

    /* 处理缓冲区中剩余不足一个 chunk 的数据 */
    if (m_bufPos > 0) {
        quint32 cv[8];
        memcpy(cv, m_key, sizeof(cv));

        int blocks = (m_bufPos + BLOCK_BYTES - 1) / BLOCK_BYTES;
        for (int i = 0; i < blocks; ++i) {
            int offset = i * BLOCK_BYTES;
            int len = qMin(BLOCK_BYTES, m_bufPos - offset);
            QByteArray padded(BLOCK_BYTES, static_cast<char>(0));
            memcpy(padded.data(), m_buffer.constData() + offset, len);

            quint32 block[16];
            loadWords(padded, 0, block);

            quint32 flags = CHUNK_START;
            if (i == blocks - 1) flags |= CHUNK_END | ROOT;
            else if (i == 0) flags = CHUNK_START;

            quint32 out[16];
            compress(cv, block, m_chunkCounter, flags, out);
            memcpy(cv, out, 8 * sizeof(quint32));
        }

        /* 从链值生成输出 */
        QByteArray cvBytes(32, static_cast<char>(0));
        storeChain(cv, cvBytes);

        int copied = qMin(outputBytes, 32);
        result.append(cvBytes.constData(), copied);

        /* 如果需要更多输出字节，使用压缩函数扩展 */
        int outIdx = 1;
        while (result.size() < outputBytes) {
            quint32 block[16];
            memset(block, 0, sizeof(block));
            block[0] = static_cast<quint32>(outIdx);

            quint32 out[16];
            compress(cv, block, 0, ROOT, out);

            QByteArray outBytes(64, static_cast<char>(0));
            for (int i = 0; i < 16; ++i) {
                outBytes[4 * i]     = static_cast<char>(out[i] & 0xFF);
                outBytes[4 * i + 1] = static_cast<char>((out[i] >> 8) & 0xFF);
                outBytes[4 * i + 2] = static_cast<char>((out[i] >> 16) & 0xFF);
                outBytes[4 * i + 3] = static_cast<char>((out[i] >> 24) & 0xFF);
            }

            int toCopy = qMin(64, outputBytes - result.size());
            result.append(outBytes.constData(), toCopy);
            ++outIdx;
        }
    } else {
        /* 空输入或刚好整数 chunk */
        quint32 cv[8];
        memcpy(cv, m_key, sizeof(cv));

        quint32 block[16];
        memset(block, 0, sizeof(block));

        quint32 out[16];
        compress(cv, block, 0, ROOT, out);

        QByteArray outBytes(64, static_cast<char>(0));
        for (int i = 0; i < 16; ++i) {
            outBytes[4 * i]     = static_cast<char>(out[i] & 0xFF);
            outBytes[4 * i + 1] = static_cast<char>((out[i] >> 8) & 0xFF);
            outBytes[4 * i + 2] = static_cast<char>((out[i] >> 16) & 0xFF);
            outBytes[4 * i + 3] = static_cast<char>((out[i] >> 24) & 0xFF);
        }
        int toCopy = qMin(outputBytes, 64);
        result.append(outBytes.constData(), toCopy);
    }

    result.resize(outputBytes);

    ++m_stats.totalHashes;
    qint64 elapsed = m_timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalHashes);

    emit hashComputed(outputBytes, static_cast<qint64>(m_stats.totalBytesProcessed));
    return result;
}

/** @brief 一次性计算哈希(静态便捷接口) @param data 输入数据 @param outputBytes 输出字节数 @return 哈希摘要 */
QByteArray Blake3Hash::hash(const QByteArray& data, int outputBytes)
{
    Blake3Hash hasher;
    hasher.update(data);
    return hasher.finalize(outputBytes);
}

/** @brief 重置所有统计计数器 */
void Blake3Hash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief G 混合函数(BLAKE2b-style) */
void Blake3Hash::g(quint32& a, quint32& b, quint32& c, quint32& d,
                   quint32 x, quint32 y)
{
    a = a + b + x;
    d = (d ^ a) << 16 | (d ^ a) >> 16;
    c = c + d;
    b = (b ^ c) << 12 | (b ^ c) >> 20;
    a = a + b + y;
    d = (d ^ a) << 8 | (d ^ a) >> 24;
    c = c + d;
    b = (b ^ c) << 7 | (b ^ c) >> 25;
}

/** @brief 压缩函数核心 */
void Blake3Hash::compress(const quint32 chaining[8], const quint32 block[16],
                          quint64 counter, quint32 flags, quint32 out[16])
{
    /* 初始化工作矩阵 */
    quint32 v[16] = {
        chaining[0], chaining[1], chaining[2], chaining[3],
        chaining[4], chaining[5], chaining[6], chaining[7],
        IV_0, IV_1, IV_2, IV_3,
        static_cast<quint32>(counter & 0xFFFFFFFF),
        static_cast<quint32>(counter >> 32),
        IV_6, flags
    };

    quint32 m[16];
    memcpy(m, block, 16 * sizeof(quint32));

    /* 7 轮混合 */
    for (int round = 0; round < 7; ++round) {
        /* 列混合 */
        g(v[0], v[4], v[ 8], v[12], m[0], m[1]);
        g(v[1], v[5], v[ 9], v[13], m[2], m[3]);
        g(v[2], v[6], v[10], v[14], m[4], m[5]);
        g(v[3], v[7], v[11], v[15], m[6], m[7]);
        /* 对角混合 */
        g(v[0], v[5], v[10], v[15], m[8], m[9]);
        g(v[1], v[6], v[11], v[12], m[10], m[11]);
        g(v[2], v[7], v[ 8], v[13], m[12], m[13]);
        g(v[3], v[4], v[ 9], v[14], m[14], m[15]);

        /* BLAKE3 的消息调度(简单的置换) */
        if (round == 0) {
            quint32 tmp[16];
            tmp[0]=m[2]; tmp[1]=m[6]; tmp[2]=m[3]; tmp[3]=m[10];
            tmp[4]=m[7]; tmp[5]=m[0]; tmp[6]=m[4]; tmp[7]=m[13];
            tmp[8]=m[1]; tmp[9]=m[11]; tmp[10]=m[12]; tmp[11]=m[5];
            tmp[12]=m[9]; tmp[13]=m[14]; tmp[14]=m[15]; tmp[15]=m[8];
            memcpy(m, tmp, sizeof(tmp));
        } else if (round % 2 == 1) {
            quint32 tmp[16];
            tmp[0]=m[3]; tmp[1]=m[7]; tmp[2]=m[11]; tmp[3]=m[15];
            tmp[4]=m[0]; tmp[5]=m[4]; tmp[6]=m[8]; tmp[7]=m[12];
            tmp[8]=m[1]; tmp[9]=m[5]; tmp[10]=m[9]; tmp[11]=m[13];
            tmp[12]=m[2]; tmp[13]=m[6]; tmp[14]=m[10]; tmp[15]=m[14];
            memcpy(m, tmp, sizeof(tmp));
        } else {
            quint32 tmp[16];
            tmp[0]=m[11]; tmp[1]=m[12]; tmp[2]=m[5]; tmp[3]=m[15];
            tmp[4]=m[10]; tmp[5]=m[7]; tmp[6]=m[14]; tmp[7]=m[2];
            tmp[8]=m[6]; tmp[9]=m[0]; tmp[10]=m[3]; tmp[11]=m[9];
            tmp[12]=m[1]; tmp[13]=m[13]; tmp[14]=m[8]; tmp[15]=m[4];
            memcpy(m, tmp, sizeof(tmp));
        }
    }

    /* 输出: chaining XOR 前半和后半 */
    for (int i = 0; i < 8; ++i) {
        out[i] = v[i] ^ v[i + 8];
    }
    for (int i = 8; i < 16; ++i) {
        out[i] = v[i - 8] ^ v[i];
    }
}

/** @brief 从字节数组加载16个32位字(小端) */
void Blake3Hash::loadWords(const QByteArray& src, int offset, quint32 words[16])
{
    for (int i = 0; i < 16; ++i) {
        int pos = offset + 4 * i;
        if (pos + 3 < src.size()) {
            words[i] = static_cast<quint32>(static_cast<quint8>(src[pos]))
                     | (static_cast<quint32>(static_cast<quint8>(src[pos + 1])) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(src[pos + 2])) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(src[pos + 3])) << 24);
        } else {
            words[i] = 0;
            for (int b = 0; pos + b < src.size() && b < 4; ++b) {
                words[i] |= static_cast<quint32>(
                    static_cast<quint8>(src[pos + b])) << (8 * b);
            }
        }
    }
}

/** @brief 将8个32位字存储到字节串(小端) */
void Blake3Hash::storeChain(const quint32 words[8], QByteArray& dest)
{
    for (int i = 0; i < 8; ++i) {
        dest[4 * i]     = static_cast<char>(words[i] & 0xFF);
        dest[4 * i + 1] = static_cast<char>((words[i] >> 8) & 0xFF);
        dest[4 * i + 2] = static_cast<char>((words[i] >> 16) & 0xFF);
        dest[4 * i + 3] = static_cast<char>((words[i] >> 24) & 0xFF);
    }
}
