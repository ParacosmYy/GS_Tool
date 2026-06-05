/**
 * @file LzwCompressor.cpp
 * @brief LZW压缩器实现
 */

#include "utils/lzw/LzwCompressor.h"
#include <QElapsedTimer>

LzwCompressor::LzwCompressor(QObject* parent)
    : QObject(parent), m_maxDictSize(4096), m_timeSum(0.0) {}

void LzwCompressor::setMaxDictSize(int maxSize) { m_maxDictSize = qMax(256, maxSize); }

QByteArray LzwCompressor::compress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (data.isEmpty()) return result;

    /* 初始化字典: 单字节条目 */
    QMap<QByteArray, int> dict;
    for (int i = 0; i < 256; ++i) dict[QByteArray(1, static_cast<char>(i))] = i;
    int nextCode = 256;

    QByteArray w;
    for (int i = 0; i < data.size(); ++i) {
        QByteArray wc = w + data[i];
        if (dict.contains(wc)) {
            w = wc;
        } else {
            /* 输出w的编码 */
            int code = dict[w];
            result.append(static_cast<char>((code >> 8) & 0xFF));
            result.append(static_cast<char>(code & 0xFF));

            /* 添加新条目 */
            if (nextCode < m_maxDictSize) dict[wc] = nextCode++;
            w = QByteArray(1, data[i]);
        }
    }

    /* 输出剩余 */
    if (!w.isEmpty()) {
        int code = dict[w];
        result.append(static_cast<char>((code >> 8) & 0xFF));
        result.append(static_cast<char>(code & 0xFF));
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalCompressions;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    m_stats.avgCompressionRatio = (m_stats.totalBytesIn > 0)
        ? static_cast<double>(m_stats.totalBytesOut) / m_stats.totalBytesIn : 0.0;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalCompressions + m_stats.totalDecompressions;
    m_stats.averageProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit compressComplete(data.size(), result.size(), m_stats.avgCompressionRatio);
    return result;
}

QByteArray LzwCompressor::decompress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (data.size() < 2) return result;

    /* 读取编码序列 */
    QVector<int> codes;
    for (int i = 0; i + 1 < data.size(); i += 2) {
        int code = (static_cast<quint8>(data[i]) << 8) | static_cast<quint8>(data[i + 1]);
        codes.append(code);
    }

    if (codes.isEmpty()) return result;

    /* 初始化字典 */
    QMap<int, QByteArray> dict;
    for (int i = 0; i < 256; ++i) dict[i] = QByteArray(1, static_cast<char>(i));
    int nextCode = 256;

    QByteArray prev = dict.value(codes[0], QByteArray());
    result.append(prev);

    for (int i = 1; i < codes.size(); ++i) {
        int code = codes[i];
        QByteArray entry;

        if (dict.contains(code)) {
            entry = dict[code];
        } else if (code == nextCode) {
            entry = prev + prev.left(1);
        } else {
            break;
        }

        result.append(entry);
        if (nextCode < m_maxDictSize) {
            dict[nextCode++] = prev + entry.left(1);
        }
        prev = entry;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalDecompressions;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalCompressions + m_stats.totalDecompressions;
    m_stats.averageProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decompressComplete(data.size(), result.size());
    return result;
}

void LzwCompressor::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
