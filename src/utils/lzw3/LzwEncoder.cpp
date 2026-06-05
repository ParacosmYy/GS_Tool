/**
 * @file LzwEncoder.cpp
 * @brief LZW编码/解码器实现
 */

#include "utils/lzw3/LzwEncoder.h"

#include <QElapsedTimer>
#include <QtMath>

LzwEncoder::LzwEncoder(QObject* parent)
    : QObject(parent)
{
}

void LzwEncoder::setMaxDictSize(int maxDictSize)
{
    m_maxDictSize = qMax(256, maxDictSize);
}

void LzwEncoder::setInitialBitWidth(int bitWidth)
{
    m_initialBitWidth = qBound(8, bitWidth, 16);
}

QByteArray LzwEncoder::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        return QByteArray();
    }

    /* 初始化字典: 单字节条目 0~255 */
    QMap<QByteArray, int> dictionary;
    for (int i = 0; i < 256; ++i) {
        dictionary[QByteArray(1, static_cast<char>(i))] = i;
    }

    int nextCode = 256;
    int bitWidth = m_initialBitWidth;
    QByteArray output;
    int bitBuffer = 0;
    int bitCount = 0;

    /* 头部: 写入原始数据长度(4字节小端) */
    int origSize = data.size();
    output.append(static_cast<char>(origSize & 0xFF));
    output.append(static_cast<char>((origSize >> 8) & 0xFF));
    output.append(static_cast<char>((origSize >> 16) & 0xFF));
    output.append(static_cast<char>((origSize >> 24) & 0xFF));

    QByteArray w;
    for (int i = 0; i < data.size(); ++i) {
        char c = data[i];
        QByteArray wc = w + QByteArray(1, c);

        if (dictionary.contains(wc)) {
            w = wc;
        } else {
            /* 输出w的编码 */
            writeBits(output, dictionary[w], bitWidth, bitBuffer, bitCount);

            /* 将wc加入字典 */
            if (nextCode < m_maxDictSize) {
                dictionary[wc] = nextCode++;
                /* 位宽增长: 当nextCode超过当前位宽能表示的范围时增1 */
                if (nextCode > (1 << bitWidth) && bitWidth < 16) {
                    ++bitWidth;
                }
            }

            w = QByteArray(1, c);
        }
    }

    /* 输出最后的w */
    if (!w.isEmpty()) {
        writeBits(output, dictionary[w], bitWidth, bitBuffer, bitCount);
    }

    /* 刷新剩余位 */
    if (bitCount > 0) {
        output.append(static_cast<char>(bitBuffer & 0xFF));
    }

    m_stats.totalEncodes++;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += output.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(data.size(), output.size());
    return output;
}

QByteArray LzwEncoder::decode(const QByteArray& compressed)
{
    QElapsedTimer timer;
    timer.start();

    if (compressed.size() < 4) {
        m_timeSum += timer.elapsed();
        return QByteArray();
    }

    /* 读取原始数据长度 */
    int origSize = (static_cast<unsigned char>(compressed[0])) |
                   (static_cast<unsigned char>(compressed[1]) << 8) |
                   (static_cast<unsigned char>(compressed[2]) << 16) |
                   (static_cast<unsigned char>(compressed[3]) << 24);

    /* 初始化字典 */
    QMap<int, QByteArray> dictionary;
    for (int i = 0; i < 256; ++i) {
        dictionary[i] = QByteArray(1, static_cast<char>(i));
    }

    int nextCode = 256;
    int bitWidth = m_initialBitWidth;
    int bytePos = 4;
    int bitBuffer = 0;
    int bitCount = 0;

    QByteArray output;
    output.reserve(origSize);

    /* 读取第一个码字 */
    int code = readBits(compressed, bitWidth, bytePos, bitBuffer, bitCount);
    if (code < 0 || !dictionary.contains(code)) {
        m_timeSum += timer.elapsed();
        return QByteArray();
    }

    QByteArray entry = dictionary[code];
    output.append(entry);

    QByteArray w = entry;

    while (output.size() < origSize) {
        code = readBits(compressed, bitWidth, bytePos, bitBuffer, bitCount);
        if (code < 0) break;

        QByteArray currentEntry;
        if (dictionary.contains(code)) {
            currentEntry = dictionary[code];
        } else if (code == nextCode) {
            /* 特殊情况: 码字等于nextCode */
            currentEntry = w + QByteArray(1, w[0]);
        } else {
            /* 错误: 无效码字 */
            break;
        }

        output.append(currentEntry);

        /* 添加新字典条目 */
        if (nextCode < m_maxDictSize) {
            dictionary[nextCode++] = w + QByteArray(1, currentEntry[0]);
            if (nextCode > (1 << bitWidth) && bitWidth < 16) {
                ++bitWidth;
            }
        }

        w = currentEntry;
    }

    /* 截取到原始长度 */
    output = output.left(origSize);

    m_stats.totalDecodes++;
    m_stats.totalBytesIn += compressed.size();
    m_stats.totalBytesOut += output.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(compressed.size(), output.size());
    return output;
}

double LzwEncoder::compressionRatio(int originalSize, int compressedSize) const
{
    if (originalSize <= 0) return 1.0;
    return static_cast<double>(compressedSize) / originalSize;
}

void LzwEncoder::writeBits(QByteArray& output, int code,
                            int bitWidth, int& bitBuffer,
                            int& bitCount) const
{
    bitBuffer |= (code << bitCount);
    bitCount += bitWidth;

    while (bitCount >= 8) {
        output.append(static_cast<char>(bitBuffer & 0xFF));
        bitBuffer >>= 8;
        bitCount -= 8;
    }
}

int LzwEncoder::readBits(const QByteArray& input, int bitWidth,
                          int& bytePos, int& bitBuffer,
                          int& bitCount) const
{
    while (bitCount < bitWidth && bytePos < input.size()) {
        bitBuffer |= (static_cast<unsigned char>(input[bytePos]) << bitCount);
        bytePos++;
        bitCount += 8;
    }

    if (bitCount < bitWidth) return -1;

    int code = bitBuffer & ((1 << bitWidth) - 1);
    bitBuffer >>= bitWidth;
    bitCount -= bitWidth;

    return code;
}

LzwEncoder::Stats LzwEncoder::stats() const
{
    return m_stats;
}

void LzwEncoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
