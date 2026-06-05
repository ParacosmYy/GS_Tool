/**
 * @file LzssCompressor.cpp
 * @brief LZSS压缩/解压引擎实现 — 滑动窗口字典匹配
 */

#include "LzssCompressor.h"

#include <QElapsedTimer>
#include <cstring>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

LzssCompressor::LzssCompressor(QObject* parent)
    : QObject(parent)
{
}

LzssCompressor::~LzssCompressor() = default;

// ═══════════════════════════════════════════════════════════
// 压缩
// ═══════════════════════════════════════════════════════════

QByteArray LzssCompressor::compress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        return {};
    }

    const int inputLen = data.size();
    const char* src = data.constData();

    /*
     * 输出格式:
     *   每组以1字节flag开头, 8个标志位对应后续8个token:
     *     bit=0: 原文字节(1字节)
     *     bit=1: 匹配引用(2字节: offset_high|offset_low<<4 | length)
     */

    QByteArray output;
    output.reserve(inputLen + inputLen / 8);

    /* 滑动窗口(初始化为零/空) */
    char window[WINDOW_SIZE];
    std::memset(window, 0, WINDOW_SIZE);
    int windowPos = 0;

    int srcPos = 0;
    QByteArray tokenBuf;  /* 当前flag组的token缓冲 */
    int flagBit = 0;
    quint8 flagByte = 0;

    auto flushFlag = [&]() {
        if (flagBit > 0) {
            output.prepend(flagByte);  /* 不好: 改用列表 */
        }
    };

    /* 简化实现: 逐字节扫描匹配 */
    while (srcPos < inputLen) {
        /* 搜索最长匹配 */
        int bestLen = 0;
        int bestOff = 0;

        int searchStart = windowPos - WINDOW_SIZE;
        if (searchStart < 0) searchStart = 0;

        for (int si = searchStart; si < windowPos; ++si) {
            int wIdx = ((si % WINDOW_SIZE) + WINDOW_SIZE) % WINDOW_SIZE;
            int len = 0;
            while (len < MAX_MATCH && srcPos + len < inputLen &&
                   window[wIdx] == src[srcPos + len]) {
                ++len;
                wIdx = (wIdx + 1) % WINDOW_SIZE;
            }
            if (len > bestLen) {
                bestLen = len;
                bestOff = windowPos - si;
            }
        }

        if (bestLen >= MIN_MATCH) {
            /* 匹配引用 */
            flagByte |= (1 << flagBit);
            int offset = bestOff - 1;  /* 0-based */
            int length = bestLen - MIN_MATCH;  /* 0-based */

            /* 编码: 2字节 = offset(12bit) + length(4bit) */
            quint8 byte1 = static_cast<quint8>((offset >> 4) & 0xFF);
            quint8 byte2 = static_cast<quint8>(((offset & 0x0F) << 4) |
                                                (length & 0x0F));
            output.append(byte1);
            output.append(byte2);

            /* 更新窗口 */
            for (int i = 0; i < bestLen; ++i) {
                window[windowPos % WINDOW_SIZE] = src[srcPos + i];
                ++windowPos;
            }
            srcPos += bestLen;
        } else {
            /* 原文字节 */
            output.append(src[srcPos]);
            window[windowPos % WINDOW_SIZE] = src[srcPos];
            ++windowPos;
            ++srcPos;
        }

        ++flagBit;
        if (flagBit >= 8) {
            /* 在输出当前位置插入flag字节 */
            /* 用更简单的方案: 先输出flag再输出tokens */
            flagBit = 0;
            flagByte = 0;
        }
    }

    /*
     * 上面的实现有问题(无法回写flag), 改用简单可靠的方案:
     * 重做压缩: flag字节先行 + tokens
     */
    output.clear();
    windowPos = 0;
    std::memset(window, 0, WINDOW_SIZE);
    srcPos = 0;

    while (srcPos < inputLen) {
        quint8 flags = 0;
        QByteArray tokens;
        int bitPos = 0;

        while (bitPos < 8 && srcPos < inputLen) {
            /* 搜索最长匹配 */
            int bestLen2 = 0;
            int bestOff2 = 0;

            int searchEnd = windowPos;
            int searchBeg = windowPos - WINDOW_SIZE;
            if (searchBeg < 0) searchBeg = 0;

            for (int si = searchBeg; si < searchEnd; ++si) {
                int wIdx = ((si % WINDOW_SIZE) + WINDOW_SIZE) % WINDOW_SIZE;
                int len = 0;
                while (len < MAX_MATCH && srcPos + len < inputLen &&
                       window[wIdx] == src[srcPos + len]) {
                    ++len;
                    wIdx = (wIdx + 1) % WINDOW_SIZE;
                }
                if (len > bestLen2) {
                    bestLen2 = len;
                    bestOff2 = windowPos - si;
                }
            }

            if (bestLen2 >= MIN_MATCH) {
                flags |= (1 << bitPos);
                int offset = bestOff2 - 1;
                int length = bestLen2 - MIN_MATCH;
                tokens.append(static_cast<char>((offset >> 4) & 0xFF));
                tokens.append(static_cast<char>((((offset & 0x0F) << 4) |
                                                  (length & 0x0F))));
                for (int i = 0; i < bestLen2; ++i) {
                    window[windowPos % WINDOW_SIZE] = src[srcPos + i];
                    ++windowPos;
                }
                srcPos += bestLen2;
            } else {
                tokens.append(src[srcPos]);
                window[windowPos % WINDOW_SIZE] = src[srcPos];
                ++windowPos;
                ++srcPos;
            }
            ++bitPos;
        }

        output.append(static_cast<char>(flags));
        output.append(tokens);
    }

    /* 更新统计 */
    m_stats.totalCompressions++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalCompressions + m_stats.totalDecompressions;
    if (cnt == 1) {
        m_stats.avgProcessingTimeMs = static_cast<double>(elapsed);
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
            static_cast<double>(elapsed) / cnt;
    }

    emit compressionCompleted(inputLen, output.size());
    return output;
}

// ═══════════════════════════════════════════════════════════
// 解压
// ═══════════════════════════════════════════════════════════

QByteArray LzssCompressor::decompress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        return {};
    }

    QByteArray output;
    output.reserve(data.size() * 4);

    char window[WINDOW_SIZE];
    std::memset(window, 0, WINDOW_SIZE);
    int windowPos = 0;

    int srcPos = 0;
    const int srcLen = data.size();
    const char* src = data.constData();

    while (srcPos < srcLen) {
        quint8 flags = static_cast<quint8>(src[srcPos++]);

        for (int bit = 0; bit < 8 && srcPos < srcLen; ++bit) {
            if (flags & (1 << bit)) {
                /* 匹配引用: 2字节 */
                if (srcPos + 1 >= srcLen) break;

                quint8 byte1 = static_cast<quint8>(src[srcPos++]);
                quint8 byte2 = static_cast<quint8>(src[srcPos++]);

                int offset = ((byte1 << 4) | (byte2 >> 4)) + 1;
                int length = (byte2 & 0x0F) + MIN_MATCH;

                for (int i = 0; i < length; ++i) {
                    int wIdx = ((windowPos - offset + i) % WINDOW_SIZE
                                + WINDOW_SIZE) % WINDOW_SIZE;
                    char ch = window[wIdx];
                    output.append(ch);
                    window[windowPos % WINDOW_SIZE] = ch;
                    ++windowPos;
                }
            } else {
                /* 原文字节 */
                char ch = src[srcPos++];
                output.append(ch);
                window[windowPos % WINDOW_SIZE] = ch;
                ++windowPos;
            }
        }
    }

    /* 更新统计 */
    m_stats.totalDecompressions++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalCompressions + m_stats.totalDecompressions;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit decompressionCompleted(data.size(), output.size());
    return output;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

LzssCompressor::Stats LzssCompressor::stats() const
{
    return m_stats;
}

void LzssCompressor::resetStatistics()
{
    m_stats = Stats{};
}
