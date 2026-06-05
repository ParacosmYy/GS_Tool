/**
 * @file LzmaCompressor.cpp
 * @brief LZMA风格压缩器实现 — 范围编码 + LZ77匹配
 */

#include "utils/compress5/LzmaCompressor.h"

#include <QElapsedTimer>

#include <cstring>

/** @brief 构造函数 @param windowBits 窗口位数 @param parent 父对象 */
LzmaCompressor::LzmaCompressor(int windowBits, QObject* parent)
    : QObject(parent)
    , m_windowBits(std::max(10, std::min(windowBits, 24)))
    , m_windowSize(1 << m_windowBits)
{
    /* 初始化概率模型为均匀分布(2048 = 50%) */
    for (int i = 0; i < 256; ++i) m_literalProb[i] = 1024;
    for (int i = 0; i < 16; ++i) {
        m_lenProb[i] = 1024;
        m_distProb[i] = 1024;
    }
    m_matchProb = 1024;
}

/* ========== 范围编码器实现 ========== */

/** @brief 初始化编码器 */
void LzmaCompressor::RangeEncoder::init()
{
    low = 0;
    range = 0xFFFFFFFF;
    output.clear();
}

/** @brief 编码一位 @param bit 位值 @param prob 概率(0~2048) */
void LzmaCompressor::RangeEncoder::encodeBit(int bit, uint32_t& prob)
{
    const uint32_t kNumBitModelTotalBits = 11;
    const uint32_t kBitModelTotal = 1u << kNumBitModelTotalBits;
    const int kNumMoveBits = 5;

    uint32_t newBound = (range >> kNumBitModelTotalBits) * prob;
    if (bit) {
        /* 编码1: 范围收缩到上半部分 */
        low += newBound + 1;
        range -= newBound + 1;
        prob -= (prob - kBitModelTotal + 1) >> kNumMoveBits;
    } else {
        /* 编码0: 范围收缩到下半部分 */
        range = newBound;
        prob += (kBitModelTotal - prob) >> kNumMoveBits;
    }

    /* 归一化: 保持range在有效范围内 */
    while (range < (1u << 24)) {
        if (low < 0xFF000000u || (low + range) > 0xFF000000u) {
            output.append(static_cast<char>(low >> 24));
            low <<= 8;
        } else {
            low <<= 8;
        }
        range <<= 8;
    }
}

/** @brief 编码直接位 @param value 值 @param numBits 位数 */
void LzmaCompressor::RangeEncoder::encodeDirectBits(uint32_t value, int numBits)
{
    for (int i = numBits - 1; i >= 0; --i) {
        range >>= 1;
        if ((value >> i) & 1) {
            low += range + 1;
        }
        while (range < (1u << 24)) {
            if (low < 0xFF000000u || (low + range) > 0xFF000000u) {
                output.append(static_cast<char>(low >> 24));
                low <<= 8;
            } else {
                low <<= 8;
            }
            range <<= 8;
        }
    }
}

/** @brief 刷新编码器 */
void LzmaCompressor::RangeEncoder::flush()
{
    for (int i = 0; i < 5; ++i) {
        output.append(static_cast<char>(low >> 24));
        low <<= 8;
    }
}

/* ========== 范围解码器实现 ========== */

/** @brief 初始化解码器 @param src 输入数据 @param size 数据大小 */
void LzmaCompressor::RangeDecoder::init(const char* src, int size)
{
    data = src;
    dataSize = size;
    pos = 0;
    code = 0;
    range = 0xFFFFFFFF;
    for (int i = 0; i < 5; ++i) {
        code = (code << 8) | static_cast<uint8_t>(pos < size ? src[pos++] : 0);
    }
}

/** @brief 解码一位 @param prob 概率 @return 位值 */
int LzmaCompressor::RangeDecoder::decodeBit(uint32_t& prob)
{
    const uint32_t kNumBitModelTotalBits = 11;
    const uint32_t kBitModelTotal = 1u << kNumBitModelTotalBits;
    const int kNumMoveBits = 5;

    uint32_t newBound = (range >> kNumBitModelTotalBits) * prob;
    int bit;

    if (code < newBound + 1) {
        /* 解码为0 */
        range = newBound;
        bit = 0;
        prob += (kBitModelTotal - prob) >> kNumMoveBits;
    } else {
        /* 解码为1 */
        range -= newBound + 1;
        code -= newBound + 1;
        bit = 1;
        prob -= (prob - kBitModelTotal + 1) >> kNumMoveBits;
    }

    while (range < (1u << 24)) {
        code = (code << 8) | static_cast<uint8_t>(pos < dataSize ? data[pos++] : 0);
        range <<= 8;
    }
    return bit;
}

/** @brief 解码直接位 @param numBits 位数 @return 值 */
uint32_t LzmaCompressor::RangeDecoder::decodeDirectBits(int numBits)
{
    uint32_t result = 0;
    for (int i = numBits - 1; i >= 0; --i) {
        range >>= 1;
        code -= range;
        uint32_t t = 0u - (code >> 31);
        code += range & t;
        if (code == range) {
            code = (code << 8) | static_cast<uint8_t>(pos < dataSize ? data[pos++] : 0);
            range <<= 8;
        }
        result = (result << 1) | (t + 1);
        while (range < (1u << 24)) {
            code = (code << 8) | static_cast<uint8_t>(pos < dataSize ? data[pos++] : 0);
            range <<= 8;
        }
    }
    return result;
}

/* ========== LZ匹配查找 ========== */

/** @brief 查找最长匹配 @param data 输入数据 @param pos 当前位置 @param matchLen 输出匹配长度 @return 匹配偏移 */
int LzmaCompressor::findMatch(const QByteArray& data, int pos, int& matchLen) const
{
    int bestLen = m_minMatchLen - 1;
    int bestOff = 0;
    int start = std::max(0, pos - m_windowSize);
    int dataLen = data.size();
    int maxLen = std::min(m_maxMatchLen, dataLen - pos);

    if (maxLen < m_minMatchLen) return 0;

    /* 在滑动窗口中搜索匹配 */
    int searchEnd = std::min(pos, start + 4096);
    for (int i = start; i < pos; ++i) {
        int len = 0;
        while (len < maxLen && data[i + len] == data[pos + len]) {
            ++len;
        }
        if (len > bestLen) {
            bestLen = len;
            bestOff = pos - i;
            if (len == maxLen) break;
        }
    }

    if (bestLen >= m_minMatchLen) {
        matchLen = bestLen;
        return bestOff;
    }
    return 0;
}

/** @brief 编码字面量 @param enc 编码器 @param byte 字节 */
void LzmaCompressor::encodeLiteral(RangeEncoder& enc, uint8_t byte)
{
    /* 编码为字面量(非匹配)标记 */
    enc.encodeBit(0, m_matchProb);
    /* 按位编码字面量字节 */
    for (int i = 7; i >= 0; --i) {
        enc.encodeBit((byte >> i) & 1, m_literalProb[i]);
    }
}

/** @brief 编码匹配 @param enc 编码器 @param offset 偏移 @param length 长度 */
void LzmaCompressor::encodeMatch(RangeEncoder& enc, int offset, int length)
{
    /* 编码为匹配标记 */
    enc.encodeBit(1, m_matchProb);

    /* 编码长度(变长编码) */
    int lenIdx = 0;
    int tmpLen = length - m_minMatchLen;
    while (tmpLen > 0 && lenIdx < 15) { tmpLen >>= 1; ++lenIdx; }
    enc.encodeBit(lenIdx & 1, m_lenProb[lenIdx]);
    if (lenIdx > 0) enc.encodeDirectBits(
        static_cast<uint32_t>(length - m_minMatchLen), lenIdx);

    /* 编码距离(变长编码) */
    int distBits = 0;
    int tmpDist = offset - 1;
    while (tmpDist > 0 && distBits < 15) { tmpDist >>= 1; ++distBits; }
    enc.encodeBit(distBits & 1, m_distProb[distBits]);
    if (distBits > 0) enc.encodeDirectBits(
        static_cast<uint32_t>(offset - 1), distBits);
}

/** @brief 解码token @param dec 解码器 @return (isMatch, literal, offset, length) */
std::tuple<bool, uint8_t, int, int> LzmaCompressor::decodeToken(RangeDecoder& dec)
{
    int isMatch = dec.decodeBit(m_matchProb);

    if (!isMatch) {
        /* 字面量: 解码8位 */
        uint8_t literal = 0;
        for (int i = 7; i >= 0; --i) {
            literal |= (dec.decodeBit(m_literalProb[i]) << i);
        }
        return {false, literal, 0, 0};
    }

    /* 匹配: 解码长度 */
    int lenIdx = dec.decodeBit(m_lenProb[0]);
    /* 简化的长度解码 */
    int length = m_minMatchLen;
    int bits = lenIdx;
    if (bits > 0) {
        length += static_cast<int>(dec.decodeDirectBits(bits));
    }

    /* 解码距离 */
    int distIdx = dec.decodeBit(m_distProb[0]);
    int offset = 1;
    int dbits = distIdx;
    if (dbits > 0) {
        offset += static_cast<int>(dec.decodeDirectBits(dbits));
    }

    return {true, 0, offset, length};
}

/** @brief 压缩数据 @param input 输入 @return 压缩后数据 */
QByteArray LzmaCompressor::compress(const QByteArray& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return {};

    RangeEncoder enc;
    enc.init();

    /* 写入原始数据大小(用于解压) */
    int origSize = input.size();
    QByteArray header;
    for (int i = 0; i < 4; ++i) {
        header.append(static_cast<char>((origSize >> (i * 8)) & 0xFF));
    }

    /* LZ77编码 */
    int pos = 0;
    while (pos < input.size()) {
        int matchLen = 0;
        int offset = findMatch(input, pos, matchLen);

        if (offset > 0 && matchLen >= m_minMatchLen) {
            encodeMatch(enc, offset, matchLen);
            pos += matchLen;
        } else {
            encodeLiteral(enc, static_cast<uint8_t>(input[pos]));
            ++pos;
        }
    }

    enc.flush();

    QByteArray result = header + enc.output;
    m_lastRatio = (origSize > 0) ? static_cast<double>(result.size()) / origSize : 0.0;

    m_stats.totalCompressions++;
    m_stats.totalBytesIn += origSize;
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalCompressions + m_stats.totalDecompressions);

    emit compressionCompleted(origSize, result.size(), m_lastRatio);
    return result;
}

/** @brief 解压数据 @param compressed 压缩数据 @param originalSize 原始大小 @return 解压后数据 */
QByteArray LzmaCompressor::decompress(const QByteArray& compressed, int originalSize)
{
    QElapsedTimer timer;
    timer.start();

    if (compressed.size() < 4) return {};

    /* 读取头部存储的原始大小 */
    int storedSize = 0;
    for (int i = 0; i < 4; ++i) {
        storedSize |= (static_cast<uint8_t>(compressed[i]) << (i * 8));
    }

    int outSize = (originalSize > 0) ? originalSize : storedSize;
    if (outSize <= 0 || outSize > 64 * 1024 * 1024) return {};

    RangeDecoder dec;
    dec.init(compressed.constData() + 4, compressed.size() - 4);

    QByteArray output;
    output.reserve(outSize);

    while (output.size() < outSize) {
        auto [isMatch, literal, offset, length] = decodeToken(dec);

        if (!isMatch) {
            output.append(static_cast<char>(literal));
        } else {
            if (offset <= 0 || length <= 0) break;
            int startPos = output.size() - offset;
            if (startPos < 0) break;
            for (int i = 0; i < length && output.size() < outSize; ++i) {
                output.append(output[startPos + i]);
            }
        }
    }

    m_stats.totalDecompressions++;
    m_stats.totalBytesOut += output.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalCompressions + m_stats.totalDecompressions);

    emit decompressionCompleted(output.size());
    return output;
}

/** @brief 重置统计 */
void LzmaCompressor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
